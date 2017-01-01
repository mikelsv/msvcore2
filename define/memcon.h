#include <typeinfo>

#ifdef WIN32
	#include "DbgHelp.h"
	#include <WinBase.h>
	#pragma comment(lib, "Dbghelp.lib")
#endif


#ifdef USEMSV_MSVCORE
bool MemConLock();
bool MemConUnLock();
bool MemConSymLock();
bool MemConSymUnLock();

void msvcore_memcon_print(const char *filename);
void msvcore_memcon_print_stack(const char *filename);

// Malloc intercept
typedef void* (WINAPIV * MSVCORE_MEMCON_MALLOC)(size_t sz); 
typedef void* (WINAPIV * MSVCORE_MEMCON_REALLOC)(void *v, size_t sz); 
typedef void (WINAPIV* MSVCORE_MEMCON_FREE)(void *v); 

MSVCORE_MEMCON_MALLOC msvcore_memcon_malloc_original;
MSVCORE_MEMCON_REALLOC msvcore_memcon_realloc_original;
MSVCORE_MEMCON_FREE msvcore_memcon_free_original;

#ifdef USEMSV_INTERCEPT 
	void* InterceptFunction(void *func, void *mfunc);
#endif

void * __cdecl msvcore_memcon_malloc_intercept(size_t size);
void * __cdecl msvcore_memcon_realloc_intercept(void *p, size_t size);
void __cdecl msvcore_memcon_free_intercept(void *p);


// Memory Controll Count class
class MsvCoreMemoryControlCount{
public:
	int64 acount, ucount, fcount;
	int64 amemory, umemory, fmemory;

	MsvCoreMemoryControlCount(){
		ClearCount();
	}

	void OnAlloc(int sz){
		acount ++;
		ucount ++;

		amemory += sz;
		umemory += sz;
	}

	void OnFree(int sz){
		ucount --;
		fcount ++;

		umemory -= sz;
		fmemory += sz;
	}

	void ClearCount(){
		acount = 0;
		ucount = 0;
		fcount = 0;

		amemory = 0;
		umemory = 0;
		fmemory = 0;
	}

};

// Memory Controll Stack class
class MsvCoreMemoryControlStack : public MsvCoreMemoryControlCount{
public:
	unsigned int sid;
	char *stacktrace;
};

// Memory Controll Info class
class MsvCoreMemoryControlInfo{
public:
	MsvCoreMemoryControlInfo *_p, *_n;
	int tid;
	int mallocsize;
#ifdef USEMSV_MEMORYCONTROL_STACK
	unsigned int sid;
#endif
};

// Memory Controll Element class
class MsvCoreMemoryControlType : public MsvCoreMemoryControlCount{
public:
	// Type identify
	int tid;

	// Type name
	char name[256];

	// Alloc, use, free counts.
	//int64 acount, ucount, fcount;

	MsvCoreMemoryControlInfo *_a, *_e;

	void Init(int tid, const char *name){
		if(!name){
			name = "<Unknown!!!>";
			printf("\r\nGLOBALERROR: Memory control type name is unknown!");
		}

		this->tid = tid;
		int sz = strlen(name);

		memcpy(this->name, name, sz);
		this->name[sz] = 0;

		// strcpy(this->name, name);

		ClearCount();

		_a = 0;
		_e = 0;
	}

	//void OnAlloc(){
	//	acount ++;
	//	ucount ++;
	//}

	//void OnFree(){
	//	ucount --;
	//	fcount ++;
	//}
};

// Memory Controll class
class _MsvCoreMemoryControl : public MsvCoreMemoryControlCount{
	// Alloc, use, free counts.
	//int64 acount, ucount, fcount;

	// Print, file time, file name.
	int printtime, filetime, stacktime;
	char *filename, *stackname;

	// Last print, file, stack name.
	int lptime, lftime, lstime;

	// Types
	MsvCoreMemoryControlType *types;
	unsigned int typesz;

	// Stack
	MsvCoreMemoryControlStack *stacks;
	unsigned int stacksz;

	// 
	int isrealloc;

public:
	// Constructor
	_MsvCoreMemoryControl(){
		printtime = 0;
		filetime = 0;
		filename = 0;
		stacktime = 0;
		stackname = 0;
		lptime = 0;
		lftime = 0;
		lstime = 0;

		types = 0;
		typesz = 0;

		stacks = 0;
		stacksz = 0;

		isrealloc = 0;

		ClearCount();

#ifdef USEMSV_INTERCEPT
		msvcore_memcon_malloc_original = (MSVCORE_MEMCON_MALLOC) InterceptFunction(malloc, msvcore_memcon_malloc_intercept);
		msvcore_memcon_realloc_original = (MSVCORE_MEMCON_REALLOC) InterceptFunction(realloc, msvcore_memcon_realloc_intercept);
		msvcore_memcon_free_original = (MSVCORE_MEMCON_FREE) InterceptFunction(free, msvcore_memcon_free_intercept);
#else
		msvcore_memcon_malloc_original = malloc;
		msvcore_memcon_realloc_original = realloc;
		msvcore_memcon_free_original = free;
#endif
	}

	// Get
	//int64 ACount(){
	//	return acount;
	//}

	//int64 UCount(){
	//	return ucount;
	//}

	//int64 FCount(){
	//	return fcount;
	//}

	unsigned int GetTypeSz(){
		return typesz;
	}

	MsvCoreMemoryControlType& GetType(unsigned int pos){
		return types[pos];
	}

	MsvCoreMemoryControlType* FindType(unsigned int &stack_type_id, void *freev = 0){
		char stack_type_info[S1K];
		//unsigned int stack_type_id;

#ifdef USEMSV_MEMORYCONTROL_INFO
		if(freev){
			MsvCoreMemoryControlInfo *lel = (MsvCoreMemoryControlInfo*)freev;
			lel --;

			stack_type_id = lel->tid;
		} else
#endif
			GetStackTypeInfo(stack_type_id, stack_type_info);

		MemConLock();

		for(unsigned int i = 0; i < typesz; i ++){
			if(types[i].tid == stack_type_id)
				return &types[i];
		}

		if(typesz % 128 == 0){
			MsvCoreMemoryControlType *els = (MsvCoreMemoryControlType*)msvcore_memcon_malloc_original(sizeof(MsvCoreMemoryControlType) * (typesz + 128));
			if(!els){
				printf("\r\nGLOBALERROR: Memory control malloc() return null!");
				return 0;
			}

			memcpy(els, types, sizeof(MsvCoreMemoryControlType) * typesz);
			msvcore_memcon_free_original(types);
			types = els;
		}

		types[typesz].Init(stack_type_id, stack_type_info);
		return &types[typesz ++];
	}

	// Set
	void SetPrint(int timeout = 60){
		printtime = timeout;
	}

	void SetPrintFile(const char *name = "msvmemcon.txt", int timeout = 1800){
		filetime = timeout;
		filename = (char*)name;
	}

	void SetPrintStack(const char *name = "msvmemcon_s.txt", int timeout = 1800){
		stacktime = timeout;
		stackname = (char*)name;
	}

	// On
	void OnAlloc(MsvCoreMemoryControlType *el, int size){
		MsvCoreMemoryControlCount::OnAlloc(size);
		el->OnAlloc(size);
	}

	void OnFree(MsvCoreMemoryControlType *el, int size){
		MsvCoreMemoryControlCount::OnFree(size);
		el->OnFree(size);
	}

	// Malloc / Free
	void* Malloc(int tid, void *t, const char *name, int size, int mallocsize){
		unsigned int type_id;
		unsigned int stack_id;

#ifdef USEMSV_MEMORYCONTROL_INFO
	#ifdef USEMSV_MEMORYCONTROL_STACK
		stack_id = StackCrc();
	#endif
#endif

		MsvCoreMemoryControlType *el = FindType(type_id);
		int oldmallocsize = mallocsize;

		OnAlloc(el, mallocsize);

#ifdef USEMSV_MEMORYCONTROL_INFO
		mallocsize += sizeof(MsvCoreMemoryControlInfo);
#endif

		// malloc
		unsigned char *mem = (unsigned char*) msvcore_memcon_malloc_original(mallocsize);  //malloc(mallocsize);

#ifdef USEMSV_MEMORYCONTROL_INFO
		MsvCoreMemoryControlInfo*lel = (MsvCoreMemoryControlInfo*)mem;
		mem += sizeof(MsvCoreMemoryControlInfo);
		OMatrixTemplateAdd(el->_a, el->_e, lel);
		lel->mallocsize = oldmallocsize;
		lel->tid = type_id;

	#ifdef USEMSV_MEMORYCONTROL_STACK
		lel->sid = stack_id; //StackCrc();
		MsvCoreMemoryControlStack *sel = GetStack(lel->sid);
		if(!sel){
			char * stacktrace = GetStackTrace();
			sel = AddStack(lel->sid, stacktrace);
		}

		sel->OnAlloc(oldmallocsize);
	#endif

#endif

		if(printtime){
			int tm = time(0);
			if(lptime + printtime <= tm){
				lptime = tm;
				msvcore_memcon_print(0);
			}
		}

		if(filetime){
			int tm = time(0);
			if(lftime + filetime <= tm){
				lftime = tm;
				msvcore_memcon_print(filename);
			}
		}

		if(stacktime){
			int tm = time(0);
			if(lstime + stacktime <= tm){
				lstime = tm;
				msvcore_memcon_print_stack(stackname);
			}
		}

		MemConUnLock();
		return mem;
	}

	void* Realloc(int tid, void *t, const char *name, int size, void *freev, int mallocsize){
		//MemConLock();
		int freesize = 0;
		isrealloc = 1;

		if(freev){			
			//MsvCoreMemoryControlEl *el = FindElement(tid, name);
			unsigned int stack_type_id;
			MsvCoreMemoryControlType *el = FindType(stack_type_id, freev);

#ifdef USEMSV_MEMORYCONTROL_INFO
			MsvCoreMemoryControlInfo *lel = (MsvCoreMemoryControlInfo*)freev;
			lel --;

			freesize = lel->mallocsize;
#endif
		}

		void *nalloc = Malloc(tid, t, name, size, mallocsize);
		memcpy(nalloc, freev, minel(mallocsize, freesize));
		Free(tid, t, name, size, freev);

		isrealloc = 0;
		MemConUnLock();

		return nalloc;
	}

	void Free(int tid, void *t, const char *name, int size, void *freev){
		if(!freev)
			return ;

		//MemConLock();
		unsigned int stack_type_id;
		MsvCoreMemoryControlType *el = FindType(stack_type_id, freev);
		//MsvCoreMemoryControlEl *el = FindElement(tid, name);
		int mallocsize = 0;

#ifdef USEMSV_MEMORYCONTROL_INFO
		MsvCoreMemoryControlInfo *lel = (MsvCoreMemoryControlInfo*)freev;
		lel --;

		OMatrixTemplateDel(el->_a, el->_e, lel);
		freev = lel;

	#ifdef USEMSV_MEMORYCONTROL_STACK
		MsvCoreMemoryControlStack *sel = GetStack(lel->sid);
		if(!sel){
			printf("\r\nGLOBALERROR: Memory control stack not found!");
		} else
			sel->OnFree(lel->mallocsize);
	#endif

		mallocsize = lel->mallocsize;
#endif

		OnFree(el, mallocsize);
		MemConUnLock();
		
		// free
		return msvcore_memcon_free_original(freev); //free(freev);
	}

public:
	// Stack
		unsigned int GetStackSz() const{
			return stacksz;
		}

		MsvCoreMemoryControlStack* GetStackPos(unsigned int pos) const{
			return &stacks[pos];
		}

protected:
	
	MsvCoreMemoryControlStack* GetStack(unsigned int sid){
		if(!stacks)
			return 0;

		MsvCoreMemoryControlStack *f = stacks, *to = stacks + stacksz, *p = 0;
		int sz = stacksz, s;

		while(sz){
			p = f;
			s = sz / 2;
			p += s;

			if(p->sid < sid){
				f = ++ p; 
				sz -= s + 1; 
			}
			else
				sz = s;
		}

		if(f && f->sid == sid)
			return f;

		return 0;
	}

	MsvCoreMemoryControlStack* AddStack(unsigned int sid, char *stacktrace){
		if(stacksz % 128 == 0)
			ResizeStack();

		MsvCoreMemoryControlStack *f = stacks, *to = stacks + stacksz, *p = 0;
		int sz = stacksz, s;

		while(sz){
			p = f;
			s = sz / 2;
			p += s;

			if(p->sid < sid){
				f = ++p; 
				sz -= s + 1;
			}
			else
				sz = s;
		}

		if(f != (stacks + stacksz))
			memcpy(f + 1, f, (stacksz - (f - stacks)) * sizeof(MsvCoreMemoryControlStack));
		stacksz ++;

		f->sid = sid;
		f->stacktrace = stacktrace;
		f->ClearCount();

		return f;
	}

	void ResizeStack(){
		int nsz = stacksz + 128;
		MsvCoreMemoryControlStack *nstacks = (MsvCoreMemoryControlStack*) msvcore_memcon_malloc_original(nsz * sizeof(MsvCoreMemoryControlStack));
		
		memcpy(nstacks, stacks, stacksz * sizeof(MsvCoreMemoryControlStack));

		msvcore_memcon_free_original(stacks);
		stacks = nstacks;

		return ;
	}

#ifdef WIN32
	void StackPrint(){
		HANDLE process;
		void *stack[256];
		unsigned short frames;
		unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;	

		process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);
		frames = CaptureStackBackTrace(0, 256, stack, NULL);
	//	symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		for(int i = 0; i < frames; i++){
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			printf("%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
		}

		//free(symbol);
		return ;
	}

	unsigned int StackCrc(){
		HANDLE process;
		void *stack[256];
		unsigned short frames;
		unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
		unsigned int ret = 0;

		process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);
		frames = CaptureStackBackTrace(0, 256, stack, NULL);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		MemConSymLock();

		for(int i = 0; i < frames; i++){
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			ret ^= symbol->Address;
			//ret ^= (unsigned int)stack[i];
		}

		MemConSymUnLock();

		return ret;
	}

	char* GetStackTrace(){
		HANDLE process;
		void *stack[256];
		unsigned short frames;
		unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
		char sbuf[4 * 1024];

		//SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_ALLOW_ZERO_ADDRESS | SYMOPT_LOAD_ANYTHING);
		SymSetOptions(SYMOPT_DEFERRED_LOADS);

		process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);
		frames = CaptureStackBackTrace(0, 256, stack, NULL);
	//	symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		int ifr = 3, ito = frames - 3;
		int size = 0;

		CONTEXT Context;
		memset(&Context, 0, sizeof(Context));
		RtlCaptureContext(&Context);

		STACKFRAME stk;
		memset(&stk, 0, sizeof(stk));

		stk.AddrPC.Offset       = Context.Eip;
		stk.AddrPC.Mode         = AddrModeFlat;
		stk.AddrStack.Offset    = Context.Esp;
		stk.AddrStack.Mode      = AddrModeFlat;
		stk.AddrFrame.Offset    = Context.Ebp;
		stk.AddrFrame.Mode      = AddrModeFlat;

		int eframes = 0;
		
		while(1){
			BOOL result = StackWalk(
                            IMAGE_FILE_MACHINE_I386,   // __in      DWORD MachineType,
                            GetCurrentProcess(),        // __in      HANDLE hProcess,
                            GetCurrentThread(),         // __in      HANDLE hThread,
                            &stk,                       // __inout   LP STACKFRAME64 StackFrame,
                            &Context,                  // __inout   PVOID ContextRecord,
                            NULL,                     // __in_opt  PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                            SymFunctionTableAccess,                      // __in_opt  PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
                            SymGetModuleBase,                     // __in_opt  PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                            NULL                       // __in_opt  PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
			);

			stack[eframes] = (void*)stk.AddrPC.Offset;
			eframes ++;

			if(!result)
				break;
		}

		frames = eframes;
		ifr = 2 + isrealloc;
		ito = frames;

		//unsigned int *eNextBP  = (unsigned int *)Context.Ebp;
		//int frame = 0;
		

#ifdef USEMSV_MEMORYCONTROL_STACKEBP
		unsigned long prev;
		unsigned long addr = 1;
		int eframes = 0;

		__asm { mov prev, ebp };

		while(addr != 0){ 
			addr = ((unsigned long *)prev)[1]; 
			prev = ((unsigned long *)prev)[0]; 
			stack[eframes] = (void*)addr;
			eframes ++;
		}

		frames = eframes;
		ifr = 0;
		ito = frames;
#endif

		MemConSymLock();

		for(int i = ifr; i < ito; i++){
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			sprintf(sbuf, "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
			size += strlen(sbuf);
		}
		size ++;

		char *ret = (char*)msvcore_memcon_malloc_original(size), *p = ret;

		for(int i = ifr; i < ito; i++){
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			sprintf(sbuf, "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
			int s = strlen(sbuf);
			memcpy(p, sbuf, s);
			p += s;
			//*p ++ = '\n';
		}
		*p ++ = 0;

		MemConSymUnLock();

		return ret;
	}

	void GetStackTypeInfoWalk(unsigned int &stack_type_id, char *stack_type_info){
		CONTEXT Context;
		memset(&Context, 0, sizeof(Context));
		RtlCaptureContext(&Context);

		STACKFRAME stk;
		memset(&stk, 0, sizeof(stk));

		stk.AddrPC.Offset       = Context.Eip;
		stk.AddrPC.Mode         = AddrModeFlat;
		stk.AddrStack.Offset    = Context.Esp;
		stk.AddrStack.Mode      = AddrModeFlat;
		stk.AddrFrame.Offset    = Context.Ebp;
		stk.AddrFrame.Mode      = AddrModeFlat;

		int frames = 0;

		while(1){
			BOOL result = StackWalk(
                            IMAGE_FILE_MACHINE_I386,   // __in      DWORD MachineType,
                            GetCurrentProcess(),        // __in      HANDLE hProcess,
                            GetCurrentThread(),         // __in      HANDLE hThread,
                            &stk,                       // __inout   LP STACKFRAME64 StackFrame,
                            &Context,                  // __inout   PVOID ContextRecord,
                            NULL,                     // __in_opt  PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                            SymFunctionTableAccess,                      // __in_opt  PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
                            SymGetModuleBase,                     // __in_opt  PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                            NULL//,                       // __in_opt  PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
							//SYM_STKWALK_FORCE_FRAMEPTR
			);

			frames ++;

			if(!result || frames == 4 + isrealloc)
				break;
		}

		char *p = stack_type_info;

		if(frames == 4 + isrealloc){
			HANDLE process = GetCurrentProcess();
			unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
			SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = 255;

			MemConSymLock();

			SymSetOptions(SYMOPT_DEFERRED_LOADS);
			SymInitialize(process, NULL, TRUE);
			SymFromAddr(process, (DWORD64)(stk.AddrPC.Offset), 0, symbol);

			MemConSymUnLock();

			char *ln = symbol->Name, *lln = ln, *to = ln + strlen(symbol->Name);
			
			while(ln < to){
				if(*ln == ':')
					break;
				ln ++;
			}

			if(ln != to){
				memcpy(p, "class ", 6);
				p += 6;

				memcpy(p, lln, ln - lln);
				p += ln - lln;
			} else {
				memcpy(p, lln, to - lln);
				p += to - lln;
			}

		} else {
			memcpy(p, "UNKNOWN", 7);
		}

		*p = 0;

		stack_type_id = crc32((unsigned char*)stack_type_info, p - stack_type_info);

		return ;
	}

	void GetStackTypeInfo(unsigned int &stack_type_id, char *stack_type_info){
		char *p = stack_type_info;

		HANDLE process = GetCurrentProcess();
		unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = 255;

		MemConSymLock();

		SymSetOptions(SYMOPT_DEFERRED_LOADS);
		SymInitialize(process, NULL, TRUE);

		void *stack[256];
		int frames = CaptureStackBackTrace(0, 256, stack, NULL);

		if(frames <= 4 + isrealloc){
			isrealloc ++;
			GetStackTypeInfoWalk(stack_type_id, stack_type_info);
			isrealloc --;

			MemConSymUnLock();
			return ;
		}

		SymFromAddr(process, (DWORD64)(stack[4 + isrealloc]), 0, symbol);
		MemConSymUnLock();

		char *ln = symbol->Name, *lln = ln, *to = ln + strlen(symbol->Name);
			
		while(ln < to){
			if(*ln == ':')
				break;
			ln ++;
		}

		if(ln != to){
			memcpy(p, "class ", 6);
			p += 6;

			memcpy(p, lln, ln - lln);
			p += ln - lln;
		} else {
			memcpy(p, lln, to - lln);
			p += to - lln;
		}

		*p = 0;

		stack_type_id = crc32((unsigned char*)stack_type_info, p - stack_type_info);

		return ;
	}

	unsigned int crc32(unsigned char *buf, unsigned long len){
		unsigned long crc_table[256];
		unsigned long crc;
		for (int i = 0; i < 256; i++){
			crc = i;
			for (int j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
			crc_table[i] = crc;
		};
		crc = 0xFFFFFFFFUL;
		while (len--)
			crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
		return crc ^ 0xFFFFFFFFUL;
	}

#else
	void StackPrint(){ return ; } 
	unsigned int StackCrc(){ return 0; }
#endif

} MsvCoreMemoryControl;

// Malloc, free, new, delete
void* msvcore_memcon_malloc(int tid, void *t, const char *name, int size, int mallocsize){
	return MsvCoreMemoryControl.Malloc(tid, t, name, size, mallocsize);
}

void msvcore_memcon_free(int tid, void *t, const char *name, int size, void *freev){
	return MsvCoreMemoryControl.Free(tid, t, name, size, freev);
}

void * __cdecl msvcore_memcon_malloc_intercept(size_t size){
	return MsvCoreMemoryControl.Malloc(2, 0, "class Malloc intercept", 0, size);
	//return msvcore_memcon_malloc_original(size);
}

void * __cdecl msvcore_memcon_realloc_intercept(void *p, size_t size){
	return MsvCoreMemoryControl.Realloc(2, 0, "class Malloc intercept", 0, p, size);
}

void __cdecl msvcore_memcon_free_intercept(void *p){
	return MsvCoreMemoryControl.Free(2, 0, "class Malloc intercept", 0, p);
	//return msvcore_memcon_free_original(p);
}

void * operator new(size_t sz) throw(){
	return msvcore_memcon_malloc(1, 0, "class Malloc", 0, sz);
	//return msvcore_memcon_malloc(sz);
}

void operator delete(void * p) throw(){
	return msvcore_memcon_free(1, 0, "class Malloc", 0, p);
	//return msvcore_memcon_free(p);
}

#else
	void* msvcore_memcon_malloc(int tid, void *t, const char *name, int size, int mallocsize);
	void msvcore_memcon_free(int tid, void *t, const char *name, int size, void *freev);

	void* operator new(size_t sz);
	void operator delete(void * p);
#endif

#ifndef USEMSV_INTERCEPT
	// Redefine
	#define malloc(v) msvcore_memcon_malloc(typeid(*this).hash_code(), this, typeid(*this).name(), sizeof(*this), v)
	#define free(v) msvcore_memcon_free(typeid(*this).hash_code(), this, typeid(*this).name(), sizeof(*this), v)
#endif