#include <typeinfo>
//#include <atomic> 

#ifdef WIN32
	#include "DbgHelp.h"
	#include <WinBase.h>
	#pragma comment(lib, "Dbghelp.lib")
#endif


#ifdef USEMSV_MSVCORE
// Lock
bool MemConLock();
bool MemConUnLock();
bool MemConSymLock();
bool MemConSymUnLock();

// Print
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

// Alloc, free used in control
//int64 msvcore_memcon_malloc_amemory = 0, msvcore_memcon_malloc_fmemory = 0;


// Memory Controll Count class
class MsvCoreMemoryControlCount{
public:
	int64 acount, /*ucount, */fcount;
	int64 amemory, /*umemory, */fmemory;

	MsvCoreMemoryControlCount(){
		ClearCount();
	}

	void OnAlloc(int sz){
		acount ++;
		//ucount ++;

		amemory += sz;
		//umemory += sz;
	}

	void OnFree(int sz){
		//ucount --;
		fcount ++;

		//umemory -= sz;
		fmemory += sz;
	}

	void ClearCount(){
		acount = 0;
		//ucount = 0;
		fcount = 0;

		amemory = 0;
		//umemory = 0;
		fmemory = 0;
	}
};

MsvCoreMemoryControlCount msvcore_memcon_use;

// Memory Controll Array class
template <class Array>
class MsvCoreMemoryControlArray{
	Array *arr;
	unsigned int size;
	unsigned int asize;

public:
	MsvCoreMemoryControlArray(){
		arr = 0;
		size = 0;
		asize = 0;
	}

	unsigned int Size() const{
		return size;
	}

	Array& GetByPos(unsigned int pos){
		// No test. Please don't set bad pos.
		return arr[pos];
	}

	Array& operator[](unsigned int pos) const{
		// No test. Please don't set bad pos.
		return arr[pos];
	}

	template <typename TN>
	Array* Find(TN &tid){
		if(!arr)
			return 0;

		Array *f = arr, *to = arr + size, *p = 0;
		int sz = size, s;

		while(sz){
			p = f;
			s = sz / 2;
			p += s;

			if(p->GetTypeId() < tid){
				f = ++ p; 
				sz -= s + 1;
			}
			else
				sz = s;
		}

		if(f && f->GetTypeId() == tid)
			return f;

		return 0;
	}

	template <typename TN>
	Array* Add(TN &tid){
		if(size == asize)
			Resize();

		Array *f = arr, *to = arr + size, *p = 0;
		int sz = size, s;

		while(sz){
			p = f;
			s = sz / 2;
			p += s;

			if(p->GetTypeId() < tid){
				f = ++p; 
				sz -= s + 1;
			}
			else
				sz = s;
		}

		if(f != (arr + size))
			memcpy(f + 1, f, (size - (f - arr)) * sizeof(Array));
		size ++;

		f->SetTypeId(tid);

		return f;
	}

protected:
	void Resize(){
		if(!MemConIsLock())
			printf("\r\nGLOBALERROR: Memory control MsvCoreMemoryControlArray don't locked!");

		if(size == asize){
			// asize = asize ? asize << 1 : 128;
			asize += 128;

			Array *els = (Array*) msvcore_memcon_malloc_original(sizeof(Array) * (asize));
			if(!els){
				printf("\r\nGLOBALERROR: Memory control MsvCoreMemoryControlArray malloc() return null!");
				return ;
			}

MemConLock();
			msvcore_memcon_use.OnAlloc(sizeof(Array) * (asize));
			msvcore_memcon_use.OnFree(sizeof(Array) * (size));
			//msvcore_memcon_malloc_amemory += sizeof(Array) * (size + 128);
			//msvcore_memcon_malloc_fmemory += sizeof(Array) * (size);
MemConUnLock();

			memcpy(els, arr, sizeof(Array) * size);
			msvcore_memcon_free_original(arr);
			arr = els;			
		}

		return ;
	}


public:
	void Clean(){
		if(arr){
			msvcore_memcon_free_original(arr);

			MemConLock();
			//msvcore_memcon_malloc_fmemory += sizeof(Array) * (size);
			msvcore_memcon_use.OnFree(sizeof(Array) * (size));
			MemConUnLock();
		}

		arr = 0;
		size = 0;
	}

	~MsvCoreMemoryControlArray(){
		Clean();
	}

};

// Memory Controll Stack class
class MsvCoreMemoryControlStack : public MsvCoreMemoryControlCount{
public:
	unsigned int sid;
	char *stacktrace;

	unsigned int GetTypeId() const{
		return sid;
	}

	void SetTypeId(unsigned int tid){
		sid = tid;
	}

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

	unsigned int GetTypeId() const{
		return tid;
	}

	void SetTypeId(unsigned int tid){
		this->tid = tid;
	}


};

// Memory Controll Sym class
class MsvCoreMemoryControlSym{
public:
	void *svoid;
	char *name;

	void* GetTypeId() const{
		return svoid;
	}

	void SetTypeId(void *p){
		svoid = p;
	}
};


// Memory Controll Jump class
struct MsvCoreMemoryControlJump{
	void *parent;
	void *offset;
	void *real;
	unsigned long crc;

	MsvCoreMemoryControlArray<MsvCoreMemoryControlJump> jumps;

	// Get / Set
	MsvCoreMemoryControlJump& GetTypeId(){
		return *this;
	}

	MsvCoreMemoryControlJump& SetTypeId(MsvCoreMemoryControlJump &j){
		*this = j;
		memset(&jumps, 0, sizeof(jumps));

		return *this;
	}

	bool operator < (MsvCoreMemoryControlJump &j) const{
		if(parent < j.parent || parent == j.parent && offset < j.offset
			|| parent == j.parent && offset == j.offset && crc < j.crc
			)
			return 1;
		return 0;
	}

	bool operator == (MsvCoreMemoryControlJump &j) const{
		return parent == j.parent && offset == j.offset
			&& crc == j.crc
			;
	}
};

// Memory Controll SymFunctionTableAccess() class
struct MsvCoreMemoryControlSymTable{
	DWORD addr;
	unsigned int null;
	FPO_DATA result;

	// Get / Set
	DWORD GetTypeId(){
		return addr;
	}

	MsvCoreMemoryControlSymTable& SetTypeId(DWORD reqaddr){
		addr = reqaddr;
		return *this;
	}

	bool operator < (DWORD reqaddr) const{
		if(addr < reqaddr)
			return 1;
		return 0;
	}

	bool operator == (DWORD reqaddr) const{
		return addr == reqaddr;
	}
};

PVOID WINAPI SymFunctionTableAccessMc(_In_ HANDLE hProcess, _In_ DWORD AddrBase){
	static MsvCoreMemoryControlArray<MsvCoreMemoryControlSymTable> tables;

	static int ca = 0, cm = 0;

//	MemConSymLock();
	ca ++;

	MsvCoreMemoryControlSymTable *tel = tables.Find(AddrBase);
	void *result;

	if(tel){
		cm ++;
		//MemConSymUnLock();
		if(tel->null)
			return 0;
		return &tel->result;
	}

	result = SymFunctionTableAccess(hProcess, AddrBase);	
	if(!result)
		return 0;

	tel = tables.Add(AddrBase);
	if(!result)
		tel->null = 1;
	else
		tel->result = *(FPO_DATA*)result;

//	 MemConSymUnLock();
	 return &tel->result;
}

DWORD WINAPI SymGetModuleBaseMc(_In_ HANDLE hProcess, _In_ DWORD dwAddr){
	return SymGetModuleBase(hProcess, dwAddr);
}


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
	MsvCoreMemoryControlArray<MsvCoreMemoryControlType> types;
	//MsvCoreMemoryControlType *types;
	//unsigned int typesz;

	// Stack
	MsvCoreMemoryControlArray<MsvCoreMemoryControlStack> stacks;
	//MsvCoreMemoryControlStack *stacks;
	//unsigned int stacksz;	

	// Sym Optimize
	MsvCoreMemoryControlArray<MsvCoreMemoryControlSym> syms;
	char *syms_buf;
	unsigned int syms_use;
	unsigned int syms_size;

	// Jumps
	MsvCoreMemoryControlArray<MsvCoreMemoryControlJump> jumps, jumps2;

	// Crc table
	unsigned long crc_table[256];

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

		//types = 0;
		//typesz = 0;

		//stacks = 0;
		//stacksz = 0;

		syms_buf = 0;
		syms_use = 0;
		syms_size = 0;

		//isrealloc = 0;

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

		// Sym init
		SymSetOptions(SYMOPT_DEFERRED_LOADS);
		SymInitialize(GetCurrentProcess(), NULL, TRUE);

		// Init crc
		unsigned long crc;
		for(int i = 0; i < 256; i++){
			crc = i;
			for (int j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
			crc_table[i] = crc;
		}

		return ;
	}

	unsigned int GetTypeSz(){
		return types.Size();
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

		MsvCoreMemoryControlType *tel = types.Find(stack_type_id);
		if(!tel){
			//MemConSymLock();
			tel = types.Add(stack_type_id);
			tel->Init(stack_type_id, stack_type_info);
			//MemConSymUnLock();
		}

		return tel;

	/*	for(unsigned int i = 0; i < typesz; i ++){
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
		return &types[typesz ++];*/
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

#ifdef USEMSV_MEMORYCONTROL_INFO
		mallocsize += sizeof(MsvCoreMemoryControlInfo);
		//msvcore_memcon_use.OnAlloc(sizeof(MsvCoreMemoryControlInfo));
		msvcore_memcon_use.amemory += sizeof(MsvCoreMemoryControlInfo);
#endif

		// malloc
		unsigned char *mem = (unsigned char*) msvcore_memcon_malloc_original(mallocsize);  //malloc(mallocsize);

		// Type & Lock
		MsvCoreMemoryControlType *el = FindType(type_id);
		int oldmallocsize = mallocsize;

		OnAlloc(el, mallocsize);

#ifdef USEMSV_MEMORYCONTROL_INFO
		MsvCoreMemoryControlInfo *lel = (MsvCoreMemoryControlInfo*)mem;
		mem += sizeof(MsvCoreMemoryControlInfo);
		OMatrixTemplateAdd(el->_a, el->_e, lel);
		lel->mallocsize = oldmallocsize;
		lel->tid = type_id;

	#ifdef USEMSV_MEMORYCONTROL_STACK
		lel->sid = stack_id; //StackCrc();
		MsvCoreMemoryControlStack *sel = stacks.Find(lel->sid); //GetStack(lel->sid);
		if(!sel){
			char * stacktrace = GetStackTrace();
			//sel = AddStack(lel->sid, stacktrace);
			sel = stacks.Add(lel->sid);			
			sel->stacktrace = stacktrace;
			sel->ClearCount();
		}

		sel->OnAlloc(oldmallocsize);
	#endif

#endif

		if(printtime){
			time_t tm = time(0);
			if(lptime + printtime <= tm){
				lptime = tm;
				msvcore_memcon_print(0);
			}
		}

		if(filetime){
			time_t tm = time(0);
			if(lftime + filetime <= tm){
				lftime = tm;
				msvcore_memcon_print(filename);
			}
		}

		if(stacktime){
			time_t tm = time(0);
			if(lstime + stacktime <= tm){
				lstime = tm;
				msvcore_memcon_print_stack(stackname);
			}
		}

		MemConUnLock();
		return mem;
	}

	void* Realloc_DEL(int tid, void *t, const char *name, int size, void *freev, int mallocsize){
		//MemConLock();
		int freesize = 0;
		//isrealloc = 1;

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

		//isrealloc = 0;
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
		MsvCoreMemoryControlStack *sel = stacks.Find(lel->sid); //GetStack(lel->sid);
		if(!sel){
			printf("\r\nGLOBALERROR: Memory control stack not found!");
		} else
			sel->OnFree(lel->mallocsize);
	#endif

		mallocsize = lel->mallocsize;
#endif

		OnFree(el, mallocsize);

		//msvcore_memcon_malloc_fmemory += sizeof(MsvCoreMemoryControlInfo);
		//msvcore_memcon_use.OnFree(sizeof(MsvCoreMemoryControlInfo));
		msvcore_memcon_use.fmemory += sizeof(MsvCoreMemoryControlInfo);

		MemConUnLock();
		
		// free
		return msvcore_memcon_free_original(freev); //free(freev);
	}

public:
	// Stack
		unsigned int GetStackSz() const{
			return stacks.Size();
		}

		MsvCoreMemoryControlStack* GetStackPos(unsigned int pos) const{
			return &stacks[pos];
		}

protected:
	
	//MsvCoreMemoryControlStack* GetStack(unsigned int sid){
	//	if(!stacks)
	//		return 0;

	//	MsvCoreMemoryControlStack *f = stacks, *to = stacks + stacksz, *p = 0;
	//	int sz = stacksz, s;

	//	while(sz){
	//		p = f;
	//		s = sz / 2;
	//		p += s;

	//		if(p->sid < sid){
	//			f = ++ p; 
	//			sz -= s + 1; 
	//		}
	//		else
	//			sz = s;
	//	}

	//	if(f && f->sid == sid)
	//		return f;

	//	return 0;
	//}

	//MsvCoreMemoryControlStack* AddStack(unsigned int sid, char *stacktrace){
	//	if(stacksz % 128 == 0)
	//		ResizeStack();

	//	MsvCoreMemoryControlStack *f = stacks, *to = stacks + stacksz, *p = 0;
	//	int sz = stacksz, s;

	//	while(sz){
	//		p = f;
	//		s = sz / 2;
	//		p += s;

	//		if(p->sid < sid){
	//			f = ++p; 
	//			sz -= s + 1;
	//		}
	//		else
	//			sz = s;
	//	}

	//	if(f != (stacks + stacksz))
	//		memcpy(f + 1, f, (stacksz - (f - stacks)) * sizeof(MsvCoreMemoryControlStack));
	//	stacksz ++;

	//	f->sid = sid;
	//	f->stacktrace = stacktrace;
	//	f->ClearCount();

	//	return f;
	//}

	//void ResizeStack(){
	//	int nsz = stacksz + 128;
	//	MsvCoreMemoryControlStack *nstacks = (MsvCoreMemoryControlStack*) msvcore_memcon_malloc_original(nsz * sizeof(MsvCoreMemoryControlStack));
	//	
	//	memcpy(nstacks, stacks, stacksz * sizeof(MsvCoreMemoryControlStack));

	//	msvcore_memcon_free_original(stacks);
	//	stacks = nstacks;

	//	return ;
	//}

#ifdef WIN32

#ifdef NOUSE_IT
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

#endif


#ifdef NOOO

	char* GetStackTrace_OLD(){
		//HANDLE process;
		void *stack[256];
		unsigned short frames;
		//unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		//SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
		char sbuf[4 * 1024];

		//SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_ALLOW_ZERO_ADDRESS | SYMOPT_LOAD_ANYTHING);
		//SymSetOptions(SYMOPT_DEFERRED_LOADS);

		//process = GetCurrentProcess();
		//SymInitialize(process, NULL, TRUE);
		frames = CaptureStackBackTrace(0, 256, stack, NULL);
	//	symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		//symbol->MaxNameLen = 255;
		//symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

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
			char Name[256 * sizeof(char)];
			//SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			SymFromAddrMc(stack[i], Name);
			sprintf(sbuf, "%i: %s - 0x%0X\n", frames - i - 1, Name, stack[i]);
			size += strlen(sbuf);
		}
		size ++;

		char *ret = (char*)msvcore_memcon_malloc_original(size), *p = ret;

		MemConLock();
		msvcore_memcon_use.OnAlloc(size);
		MemConUnLock();

		for(int i = ifr; i < ito; i++){
			char Name[256 * sizeof(char)];
			//SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			SymFromAddrMc(stack[i], Name);
			//sprintf(sbuf, "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
			sprintf(sbuf, "%i: %s - 0x%0X\n", frames - i - 1, Name, stack[i]);
			int s = strlen(sbuf);
			memcpy(p, sbuf, s);
			p += s;
			//*p ++ = '\n';
		}
		*p ++ = 0;

		MemConSymUnLock();

		return ret;
	}

#endif

	void* GetStackAddrPrev(int count, void *&raddr){
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

		while(count > 0){
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

			count --;

			if(!result)
				return 0;
		}

		raddr = (void*)stk.AddrFrame.Offset;
		return (void*)stk.AddrPC.Offset;
	}

	struct GetStackNextStr{
		unsigned long rebp;
		unsigned long resp;
		unsigned long lebp;
		unsigned long addr;
		unsigned long crc;
		unsigned int frame;

		//
		unsigned int conframe;
		CONTEXT Context;
		STACKFRAME stk;

		GetStackNextStr(int f){
			frame = f;
			conframe = 0xffffffff;
			crc = 0xFFFFFFFFUL;
		};
	};


		void* GetStackAddrPrev(GetStackNextStr &frame, int count, void *&raddr){
		if(frame.conframe == 0xffffffff || count < frame.conframe){
			memset(&frame.Context, 0, sizeof(frame.Context));
			RtlCaptureContext(&frame.Context);

			memset(&frame.stk, 0, sizeof(frame.stk));

			frame.stk.AddrPC.Offset       = frame.Context.Eip;
			frame.stk.AddrPC.Mode         = AddrModeFlat;
			frame.stk.AddrStack.Offset    = frame.Context.Esp;
			frame.stk.AddrStack.Mode      = AddrModeFlat;
			frame.stk.AddrFrame.Offset    = frame.Context.Ebp;
			frame.stk.AddrFrame.Mode      = AddrModeFlat;

			frame.conframe = 0;
		}

		while(count != frame.conframe){
			BOOL result = StackWalk(
                            IMAGE_FILE_MACHINE_I386,   // __in      DWORD MachineType,
                            GetCurrentProcess(),        // __in      HANDLE hProcess,
                            GetCurrentThread(),         // __in      HANDLE hThread,
                            &frame.stk,                       // __inout   LP STACKFRAME64 StackFrame,
                            &frame.Context,                  // __inout   PVOID ContextRecord,
                            NULL,                     // __in_opt  PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                            SymFunctionTableAccessMc,                      // __in_opt  PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
                            SymGetModuleBaseMc,                     // __in_opt  PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                            NULL//,                       // __in_opt  PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
							//SYM_STKWALK_FORCE_FRAMEPTR
			);

			frame.conframe ++;

			if(!result)
				return 0;
		}

		raddr = (void*)frame.stk.AddrFrame.Offset;
		return (void*)frame.stk.AddrPC.Offset;
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

			if(!result || frames == 4)
				break;
		}

		char *p = stack_type_info;

		if(frames == 4){
			//HANDLE process = GetCurrentProcess();
			//unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
			//SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
			//symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			//symbol->MaxNameLen = 255;

			MemConSymLock();

			//SymSetOptions(SYMOPT_DEFERRED_LOADS);
			//SymInitialize(process, NULL, TRUE);
			//SymFromAddr(process, (DWORD64)(stk.AddrPC.Offset), 0, symbol);

			char Name[256 * sizeof(char)];
			SymFromAddrMc((void*)stk.AddrPC.Offset, Name);

			MemConSymUnLock();

			char *ln = Name, *lln = ln, *to = ln + strlen(Name);
			
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


	// Stack
	unsigned long GetStackFirst(int frame, unsigned long &crc){
		unsigned long rebp;
		unsigned long addr;
		__asm { mov rebp, ebp };

		while(frame){
			rebp = ((unsigned long *)rebp)[0];
			frame --;
		}

		addr = ((unsigned long *)rebp)[1];
		crc = 0xFFFFFFFFUL;
		
		unsigned char *pebp = (unsigned char*)&addr;
		for(int i = 0; i < sizeof(long); i ++)
			crc = crc_table[(crc ^ *pebp++) & 0xFF] ^ (crc >> 8);

		//MemConSymLock();
		//addr = ((unsigned long *)rebp)[1];
		//MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)addr);
		//printf("\r\n[FIRST STACK]\r\n%s\r\n", sel->name);
		//MemConSymUnLock();

		return rebp;
	}

	bool GetStackFirst(GetStackNextStr &frame){
		unsigned long rebp, resp;
		unsigned int count;
		//unsigned long addr;

		__asm { mov rebp, ebp };
		__asm { mov resp, esp };

		frame.rebp = rebp;
		frame.resp = resp;
		count = frame.frame;

		while(count){
			frame.lebp = frame.rebp;
			frame.rebp = ((unsigned long *)frame.rebp)[0];
			count --;
		}

		frame.addr = ((unsigned long *)frame.rebp)[1];
		//frame.crc = 0xFFFFFFFFUL;
		
		unsigned char *pebp = (unsigned char*)&frame.addr;
		for(int i = 0; i < sizeof(long); i ++)
			frame.crc = crc_table[(frame.crc ^ *pebp++) & 0xFF] ^ (frame.crc >> 8);

		//MemConSymLock();
		//addr = ((unsigned long *)rebp)[1];
		//MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)addr);
		//printf("\r\n[FIRST STACK]\r\n%s\r\n", sel->name);
		//MemConSymUnLock();

		return frame.rebp && frame.addr;
	}
	
	unsigned long GetStackNext(unsigned long rebp, int &frame, unsigned long &crc, bool repair = 0){
		unsigned long lebp;
		unsigned long addr;

		if(!rebp)
			return 0;

		lebp = rebp;
		addr = ((unsigned long *)rebp)[1];
		rebp = ((unsigned long *)rebp)[0];

		if(!addr)
			return 0;

		MsvCoreMemoryControlJump fjmp = {(void*)lebp, (void*)rebp, 0, crc}, *jmp = jumps.Find(fjmp);
		if(jmp && !repair){
			rebp = (unsigned long)jmp->real;
		} else {
			void *real;

			GetStackAddrPrev(frame + 2, real);

			if(!repair || !jmp){
				MsvCoreMemoryControlJump fjmp = {(void*)lebp, (void*)rebp, real, crc}, *ajmp = jumps.Add(fjmp);
			} else{
				jmp->real = real;
			}

			rebp = (unsigned long)real;
		}

		unsigned char *pebp = (unsigned char*)&addr;
		for(int i = 0; i < sizeof(long); i ++)
			crc = crc_table[(crc ^ *pebp++) & 0xFF] ^ (crc >> 8);

		frame ++;

		//MemConSymLock();
		//addr = ((unsigned long *)rebp)[1];
		//if(addr){
		//	MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)addr);
		//	printf("%s\r\n", sel->name);
		//}
		//MemConSymUnLock();

		return rebp;
	}

	unsigned long GetStackNext2(unsigned long rebp, int &frame, unsigned long &crc, MsvCoreMemoryControlArray<MsvCoreMemoryControlJump> *&jumps, bool repair = 0){
		unsigned long lebp;
		unsigned long addr;

		if(!rebp)
			return 0;

		lebp = rebp;
		addr = ((unsigned long *)rebp)[1];
		rebp = ((unsigned long *)rebp)[0];

		if(!addr)
			return 0;

		MsvCoreMemoryControlJump fjmp = {(void*)lebp, (void*)rebp, 0, crc}, *jmp = jumps->Find(fjmp);
		if(jmp && !repair){
			rebp = (unsigned long)jmp->real;
			jumps = &jmp->jumps;
		} else {
			void *real;

			GetStackAddrPrev(frame + 2, real);

			if(!repair || !jmp){
				MsvCoreMemoryControlJump fjmp = {(void*)lebp, (void*)rebp, real, crc}, *ajmp = jumps->Add(fjmp);
				jumps = &ajmp->jumps;
			} else{
				jmp->real = real;
			}
			
			rebp = (unsigned long)real;
		}

		unsigned char *pebp = (unsigned char*)&addr;
		for(int i = 0; i < sizeof(long); i ++)
			crc = crc_table[(crc ^ *pebp++) & 0xFF] ^ (crc >> 8);

		frame ++;

		//MemConSymLock();
		//addr = ((unsigned long *)rebp)[1];
		//if(addr){
		//	MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)addr);
		//	printf("%s\r\n", sel->name);
		//}
		//MemConSymUnLock();

		return rebp;
	}

	bool GetStackNext3(GetStackNextStr &frame){
		unsigned long laddr;
		bool repair = 0;

		if(!frame.rebp)
			return 0;

		frame.lebp = frame.rebp;
		laddr = frame.addr;
		frame.addr = ((unsigned long *)frame.rebp)[1];
		frame.rebp = ((unsigned long *)frame.rebp)[0];

		if(!frame.addr)
			return 0;

		MsvCoreMemoryControlJump fjmp = {(void*)frame.lebp, (void*)frame.rebp, 0, laddr}, *jmp = jumps.Find(fjmp);
		if(jmp && !repair){
			if(jmp->crc != laddr)
				int ertye = 456;

			frame.rebp = (unsigned long)jmp->real;
		} else {
			void *real;

			GetStackAddrPrev(frame, frame.frame + 2, real);

			if(!repair || !jmp){
				MsvCoreMemoryControlJump fjmp = {(void*)frame.lebp, (void*)frame.rebp, real, laddr}, *ajmp = jumps.Add(fjmp);
			} else{
				jmp->real = real;
			}

			frame.rebp = (unsigned long)real;
		}

		unsigned char *pebp = (unsigned char*)&frame.addr;
		for(int i = 0; i < sizeof(long); i ++)
			frame.crc = crc_table[(frame.crc ^ *pebp++) & 0xFF] ^ (frame.crc >> 8);

		frame.frame ++;

		//MemConSymLock();
		//addr = ((unsigned long *)rebp)[1];
		//if(addr){
		//	MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)addr);
		//	printf("%s\r\n", sel->name);
		//}
		//MemConSymUnLock();

		return frame.rebp && frame.addr;
	}

	bool GetStackNext4(GetStackNextStr &frame){
		unsigned long laddr;
		bool repair = 0;

		if(!frame.rebp)
			return 0;

		frame.lebp = frame.rebp;
		laddr = frame.addr;
		frame.addr = ((unsigned long *)frame.rebp)[1];
		frame.rebp = ((unsigned long *)frame.rebp)[0];

		if(!frame.addr)
			return 0;

		//MemConSymLock();
		////addr = ((unsigned long *)rebp)[1];
		//if(frame.addr){
		//	MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)frame.addr);
		//	printf("%s\r\n", sel->name);
		//}
		//MemConSymUnLock();

		if(frame.rebp < frame.resp){
			int er = 567;
		}

		void *real;
		GetStackAddrPrev(frame, frame.frame + 2, real);
		frame.rebp = (unsigned long)real;


/*		MsvCoreMemoryControlJump fjmp = {(void*)frame.lebp, (void*)frame.rebp, 0, laddr}, *jmp = jumps.Find(fjmp);
		if(jmp && !repair){
			if(jmp->crc != laddr)
				int ertye = 456;

			frame.rebp = (unsigned long)jmp->real;
		} else {
			void *real;

			GetStackAddrPrev(frame, frame.frame + 2, real);

			if(!repair || !jmp){
				MsvCoreMemoryControlJump fjmp = {(void*)frame.lebp, (void*)frame.rebp, real, laddr}, *ajmp = jumps.Add(fjmp);
			} else{
				jmp->real = real;
			}

			frame.rebp = (unsigned long)real;
		}*/

		unsigned char *pebp = (unsigned char*)&frame.addr;
		for(int i = 0; i < sizeof(long); i ++)
			frame.crc = crc_table[(frame.crc ^ *pebp++) & 0xFF] ^ (frame.crc >> 8);

		frame.frame ++;

		//MemConSymLock();
		//addr = ((unsigned long *)rebp)[1];
		//if(addr){
		//	MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)addr);
		//	printf("%s\r\n", sel->name);
		//}
		//MemConSymUnLock();

		return frame.rebp && frame.addr;
	}

	char* GetStackName(unsigned long rebp){
		unsigned long addr;

		if(!rebp)
			return 0;

		addr = ((unsigned long *)rebp)[1];

		if(!addr)
			return 0;

		MsvCoreMemoryControlSym *sel = SymFromAddrMcV((void*)addr);

		if(sel)
			return sel->name;

		return 0;
	}

	unsigned long crc32(unsigned long crc, unsigned long val){
		unsigned char *pcrc = (unsigned char*)&val;

		for(int i = 0; i < sizeof(long); i ++)
			crc = crc_table[(crc ^ *pcrc++) & 0xFF] ^ (crc >> 8);

		return crc;
	}

	unsigned int StackCrc(bool repair = 0){
		unsigned long crc = 0xFFFFFFFFUL;
		unsigned long rebp;
		//unsigned int crc = 0;
		int frame = 3 + repair;

		{
		GetStackNextStr frame(3 + repair);
		GetStackFirst(frame);


		//frame.crc = crc32(frame.crc, frame.resp);
		//frame.crc = crc32(frame.crc, frame.addr);
		//frame.crc = crc32(frame.crc, frame.lebp);
		//frame.crc = crc32(frame.crc, frame.rebp);

		return frame.crc ^ 0xFFFFFFFFUL;


		MemConSymLock();

		while(1){
			if(!GetStackNext4(frame))
				break;

			if(frame.frame > 256){
				MemConSymUnLock();
				return StackCrc(1);
			}
		}

		MemConSymUnLock();


		return frame.crc ^ 0xFFFFFFFFUL;
		}

		MsvCoreMemoryControlArray<MsvCoreMemoryControlJump> *jumps = &this->jumps2;

		MemConSymLock();

		while(rebp){
			rebp = GetStackNext2(rebp, frame, crc, jumps, repair);

			if(frame > 256){
				MemConSymUnLock();
				return StackCrc(1);
			}
		}

		MemConSymUnLock();

		return crc ^ 0xFFFFFFFFUL;
	

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

		//MemConSymLock();

		for(int i = 0; i < frames; i++){
			//SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			//ret ^= symbol->Address;
			ret ^= (unsigned int)stack[i];
		}

		//MemConSymUnLock();

		return ret;
	}

	char* GetStackTrace(bool repair = 0){
		void* addrs[256];
		char* names[256];
		char buf[S1K];
		int count = 0;
		int size = 0;



		GetStackNextStr frame(3 + repair);
		GetStackFirst(frame);

		MemConSymLock();

		while(frame.addr){
			addrs[count] = (void*)frame.addr;
			names[count] = GetStackName(frame.rebp);
			
			if(!names[count])
				break;

			// Repair
			if(!repair && names[count][0] == '?'){
				MemConSymUnLock();
				return GetStackTrace(1);
			}

			sprintf(buf, "%i: %s - 0x%0X\n", count + 1, names[count], addrs[count]);
			size += strlen(buf);


			if(!GetStackNext4(frame))
				break;
			
			count ++;

			if(count > 256){
				printf("Fatal error, frames > 256.\r\n");
				MemConSymUnLock();
				return 0;
			}
		}
		size ++;

		char *ret = (char*)msvcore_memcon_malloc_original(size), *p = ret;

		MemConLock();
		msvcore_memcon_use.OnAlloc(size);
		MemConUnLock();

		for(int i = 0; i < count; i ++){
			sprintf(buf, "%i: %s - 0x%0X\n", count - i, names[i], addrs[i]);
			int s = strlen(buf);
			memcpy(p, buf, s);
			p += s;
		}

		*p ++ = 0;

		MemConSymUnLock();

		return ret;
	}

	char* GetStackTrace_OLD(bool repair = 0){
		unsigned long rebp, addr;
		char* names[256];
		void* addrs[256];
		char buf[S1K];
		int frame = 3 + repair;
		int count = 0;
		int size = 0;

		static char *buffer = (char*)msvcore_memcon_malloc_original(100);
		memcpy(buffer, "OLOLO", 6);
		return buffer;

		MsvCoreMemoryControlArray<MsvCoreMemoryControlJump> *jumps = &this->jumps2;

		// Allways repair
		//repair = 1;

		unsigned long crc = 0xFFFFFFFFUL;
		rebp = GetStackFirst(frame, crc);

		MemConSymLock();
		
		while(rebp){
			addr = ((unsigned long *)rebp)[1];
			names[count] = GetStackName(rebp);
			addrs[count] = (void*)addr;

			if(!names[count])
				break;

			// Repair
			if(!repair && names[count][0] == '?'){
				MemConSymUnLock();
				return GetStackTrace(1);
			}

#ifdef USEMSV_MEMORYCONTROL_TESTS
			void *a, *r;
			a = GetStackAddrPrev(frame, r);

			if(r != (void*)rebp)
				int ert = 47;
#endif

			//printf("%i:  0x%0X\n", count + 1, addr);

			if(!addr)
				break;

			sprintf(buf, "%i: %s - 0x%0X\n", count + 1, names[count], addrs[count]);
			size += strlen(buf);

			//printf(buf);

			//rebp = GetStackNext(rebp, frame, crc, repair);
			rebp = GetStackNext2(rebp, frame, crc, jumps, repair);
			


			count ++;

			if(count > 256){
				printf("Fatal error, frames > 256.\r\n");
				MemConSymUnLock();
				return 0;
			}
		}
		size ++;

		char *ret = (char*)msvcore_memcon_malloc_original(size), *p = ret;

		MemConLock();
		msvcore_memcon_use.OnAlloc(size);
		MemConUnLock();

		for(int i = 0; i < count; i ++){
			sprintf(buf, "%i: %s - 0x%0X\n", count - i, names[i], addrs[i]);
			int s = strlen(buf);
			memcpy(p, buf, s);
			p += s;
		}

		*p ++ = 0;

		MemConSymUnLock();

		return ret;
	}

	void GetStackTypeInfo(unsigned int &stack_type_id, char *stack_type_info){
		char *p = stack_type_info;
		
		unsigned long rebp;
		int frame = 4;
		
		unsigned long crc = 0xFFFFFFFFUL;
		rebp = GetStackFirst(frame, crc);

		MemConSymLock();
		//rebp = GetStackNext(rebp, frame);
		char* Name = GetStackName(rebp);
		MemConSymUnLock();

		char *ln = Name, *lln = ln, *to = ln + strlen(Name);
			
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


		//HANDLE process = GetCurrentProcess();
		//unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		//SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
		//symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		//symbol->MaxNameLen = 255;



//		SymSetOptions(SYMOPT_DEFERRED_LOADS);
//		SymInitialize(process, NULL, TRUE);

		void *stack[256];
		int frames = CaptureStackBackTrace(0, 256, stack, NULL);

		MemConSymLock();

		
		/*
		unsigned long prev, lprev; //llprev;
		unsigned long addr = 1;
		int fcount = 0;
		bool nook = 0;

		__asm { mov prev, ebp };

		//printf("\r\n\r\n[Stack]\r\n");

		while(addr != 0 && prev != 0){
			lprev = prev;
			addr = ((unsigned long *)prev)[1]; 
			prev = ((unsigned long *)prev)[0];

			StackJump fjmp = {(void*)lprev, (void*)prev, 0}, *jmp = jump.Find(fjmp);
			if(jmp){
				prev = (unsigned long)jmp->real;
			} else {
				void *real;

				GetStackAddrPrev(fcount + 2, real);

				StackJump fjmp = {(void*)lprev, (void*)prev, real}, *ajmp = jump.Add(fjmp);
				prev = (unsigned long)real;
			}

			fcount ++;

			if(addr){		
				char Name[256 * sizeof(char)];
				//bool ok = SymFromAddrMc((void*)addr, Name);
				//printf("%s\r\n", Name);
			}
		}*/



/*
			stack[eframes] = (void*)addr;

			if(!addr)
				break;

			//if(frames - 1 > eframes && stack[eframes + 1] != (void*)addr)
			//	int eerr = 647;

			static int count = 0;
			count ++;

			char Name[256 * sizeof(char)];
			bool ok = SymFromAddrMc((void*)addr, Name);

			//if(!ok){
			//	addr = ((unsigned long *)lprev)[0];
			//	prev = ((unsigned long *)lprev)[1];
			//	ok = SymFromAddrMc((void*)addr, Name);
			//}

			if(nook)
				ok = 0;

			static MsvCoreMemoryControlArray<StackJump> jump;

			StackJump fjmp = {(void*)lprev, (void*)prev, 0}, *jmp = jump.Find(fjmp);
			if(!ok && jmp){
			//	prev = (unsigned long)jmp->real;
			//	//addr = ((unsigned long *)prev)[1]; 
			//	//prev = ((unsigned long *)prev)[0];
			//	//stack[eframes] = (void*)addr;
			//	//ok = SymFromAddrMc((void*)addr, Name);
			//	nook = 1;
			//	//ok = 1;
				printf("[J] ");
			}

			void *p, *r = (void*)prev;
			if(!ok){
				addr = (unsigned long)GetStackAddrPrev(eframes + 2, r);
				
				if(!jmp){
					StackJump fjmp = {(void*)lprev, (void*)prev, r}, *ajmp = jump.Add(fjmp);
				}

				prev = (unsigned long)r;
				ok = SymFromAddrMc((void*)addr, Name);

				printf("[P] ");

				//p = GetStackAddrPrev(eframes + 2, r);
				//ok = SymFromAddrMc((void*)p, Name);
				nook = 1;
			}
			
			if(addr)
				printf("%s\r\n", Name);

			if(prev == 2){
				int rt =  567;
			}

			eframes ++;
		}
*/

#ifdef NOOOOO

		if(frames <= 4 + isrealloc){
			isrealloc ++;
			GetStackTypeInfoWalk(stack_type_id, stack_type_info);
			isrealloc --;

			MemConSymUnLock();
			return ;
		}

		//SymFromAddr(process, (DWORD64)(stack[4 + isrealloc]), 0, symbol);

//		char Name[256 * sizeof(char)];
		SymFromAddrMc(stack[4 + isrealloc], Name);

		MemConSymUnLock();

		char *ln = Name, *lln = ln, *to = ln + strlen(Name);
			
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

#endif

		return ;
	}

	MsvCoreMemoryControlSym* SymFromAddrMcV(void *addr){
		MsvCoreMemoryControlSym *sel = syms.Find(addr);
		bool ret;
	
		if(sel){
			//memcpy(result, sel->name, strlen(sel->name) + 1);
			//return sel->name[0] != 0;
			return sel;
		}else {
			sel = syms.Add(addr);

			unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
			SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = 255;

			ret = ::SymFromAddr(GetCurrentProcess(), (DWORD64)addr, 0, symbol);

			if(!ret)
				memcpy(symbol->Name, "??????????", 11);

			int rsize = strlen(symbol->Name);
			rsize ++;

			if(syms_use + rsize > syms_size){
				// New buf. Not free.
				syms_buf = (char*)msvcore_memcon_malloc_original(S16K);
				syms_use = 0;
				syms_size = S16K;

				MemConLock();
				//msvcore_memcon_malloc_amemory += S16K;
				msvcore_memcon_use.OnAlloc(S16K);
				MemConUnLock();
			}

			sel->name = syms_buf + syms_use;
			memcpy(syms_buf + syms_use, symbol->Name, rsize);
			syms_use += rsize;
		}

		return sel;
	}

	bool SymFromAddrMc(void *p, char result[255]){
		bool ret;

#ifdef USEMSV_MEMORYCONTROL_SYMOPT
	MsvCoreMemoryControlSym *sel = syms.Find(p);
	if(sel){
		memcpy(result, sel->name, strlen(sel->name) + 1);
		return sel->name[0] != 0;
	}else {
		sel = syms.Add(p);
#endif

		unsigned char buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		SYMBOL_INFO *symbol = (SYMBOL_INFO*)buf;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = 255;

		ret = ::SymFromAddr(GetCurrentProcess(), (DWORD64)p, 0, symbol);

		int rsize = ret ? strlen(symbol->Name) : 0;
		memcpy(result, symbol->Name, rsize);
		result[rsize] = 0;

#ifdef USEMSV_MEMORYCONTROL_SYMOPT
		rsize ++;

		if(syms_use + rsize > syms_size){
			// New buf. Not free.
			syms_buf = (char*)msvcore_memcon_malloc_original(S16K);
			syms_use = 0;
			syms_size = S16K;

			MemConLock();
			//msvcore_memcon_malloc_amemory += S16K;
			msvcore_memcon_use.OnAlloc(S16K);
			MemConUnLock();
		}

		sel->name = syms_buf + syms_use;

		memcpy(syms_buf + syms_use, result, rsize);
		syms_use += rsize;		
	}
#endif

		return ret;
	}

	unsigned int crc32(unsigned char *buf, unsigned long len){
		unsigned long crc = 0xFFFFFFFFUL;

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

void * __cdecl msvcore_memcon_realloc_intercept(void *freev, size_t size){
		//MemConLock();
		int freesize = 0;
		//isrealloc = 1;

		if(freev){			
			//MsvCoreMemoryControlEl *el = FindElement(tid, name);
			unsigned int stack_type_id;
			MsvCoreMemoryControlType *el = MsvCoreMemoryControl.FindType(stack_type_id, freev);

#ifdef USEMSV_MEMORYCONTROL_INFO
			MsvCoreMemoryControlInfo *lel = (MsvCoreMemoryControlInfo*)freev;
			lel --;

			freesize = lel->mallocsize;
#endif
		}

		void *nalloc = MsvCoreMemoryControl.Malloc(0, 0, 0, 0, size);
		memcpy(nalloc, freev, minel(size, freesize));
		MsvCoreMemoryControl.Free(0, 0, 0, 0, freev);

		//isrealloc = 0;
		MemConUnLock();

		return nalloc;




//	return MsvCoreMemoryControl.Realloc(2, 0, "class Malloc intercept", 0, p, size);
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