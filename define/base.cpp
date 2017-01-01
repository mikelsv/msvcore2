#ifdef WIN32 // Win32 include
	int _TSYS = TSYS_WIN;
	#pragma comment(lib,"Ws2_32.lib") 

	int stdmkdir(const char *path, int mode){ return _mkdir(path); }
#else
	#ifdef USEMSV_ANDROID
		int _TSYS=TSYS_ANDROID;
	#else
		int _TSYS=TSYS_LIN;
	#endif

	#define _unlink unlink
	#define _chdir chdir
	#define _getcwd getcwd

	#ifndef USEMSV_FLASH
		DWORD GetCurrentThreadId(){ return pthread_self(); }
	#else
		DWORD GetCurrentThreadId(){ return 0; }
	#endif

	int stdmkdir(const char *path, int mode){
		return mkdir(path, mode);
	}

	void Sleep(unsigned int i){
		usleep((int64)i*1000);
		return ;
	}

#endif

// Time
unsigned int time(){
	timeb tb;
	ftime(&tb);
	return (int)tb.time;
}

timeb alltime(){
	timeb tb;
	ftime(&tb);
	return tb;
}

int timemi(timeb&o, timeb&t){
	return (int)(o.time-t.time)*1000+o.millitm-t.millitm;
}

int timemit(timeb&t){
	timeb o;
	ftime(&o);
	return (int)(o.time-t.time)*1000+o.millitm-t.millitm;
}

int64 sectime(){
	timeb ft;
	ftime(&ft);
	return int64(ft.time)*1000+ft.millitm;
}


#ifdef USEMSV_MEMORYCONTROL_X

// Memory Controll class
struct _msvcore_memcon_data{
	unsigned int id;
	const char *name;
	//unsigned int size;
	//unsigned int count, acount;
	int64 mcount, ucount, fcount;

	// all malloc, use, free count
	static int64 amcount, aucount, afcount;

	static int filetime;


	void init(int id, const char *name){
		if(!name){
			name = "<Unknown!!!>";
			printf("\r\nGLOBALERROR: Memory control type name is unknown!");
		}

		this->id = id;
		this->name = name;
		mcount = 0;
		ucount = 0;
		fcount = 0;
	}

	void m(){
		mcount ++;
		ucount ++;
		amcount ++;
		aucount ++;
	}

	void f(){
		fcount ++;
		ucount --;
		afcount ++;
		aucount --;
	}
};

int64 _msvcore_memcon_data::amcount = 0;
int64 _msvcore_memcon_data::aucount = 0;
int64 _msvcore_memcon_data::afcount = 0;

#ifdef USEMSV_MEMORYCONTROL_FILE_TIME
	int _msvcore_memcon_data::filetime = time(0) - USEMSV_MEMORYCONTROL_FILE_TIME + 300;
#else
	int _msvcore_memcon_data::filetime = 0;
#endif


// Global values
int msvcore_memcon_malloc_count = 0, msvcore_memcon_free_count = 0;
_msvcore_memcon_data *msvcore_memcon_data = 0;
int msvcore_memcon_data_sz = 0;
void **msvcore_memcon_data_void = 0;

int msvcore_memcon_print_ltime = 0;

// Print
void msvcore_memcon_print(char* tofile = 0);

// Lock
bool MemConLock();
bool MemConUnLock();

#ifdef WIN32
	void msvcore_memcon_stack_print(){
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

	unsigned int msvcore_memcon_stack_crc(){
		HANDLE process;
		void *stack[256];
		unsigned short frames;
		unsigned int ret = 0;

		process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);
		frames = CaptureStackBackTrace(0, 256, stack, NULL);

		for(int i = 0; i < frames; i++)
			ret ^= (unsigned int)stack[i];

		return ret;
	}

#else
	void msvcore_memcon_stack_print(){ return ; } 
	unsigned int msvcore_memcon_stack_crc(){ return 0; }
#endif

void * msvcore_memcon_malloc(int sz, _msvcore_memcon_data *item){
#ifdef USEMSV_MEMORYCONTROL_PRINT || USEMSV_MEMORYCONTROL_FILE_PRINT
	int tm = time(0);
#endif
	int prn = 0;

#ifdef USEMSV_MEMORYCONTROL_PRINT	
	if(msvcore_memcon_print_ltime + USEMSV_MEMORYCONTROL_PRINT < tm){
		msvcore_memcon_print_ltime = tm;
		prn = 1;
	}
#endif

#ifdef USEMSV_MEMORYCONTROL_FILE_PRINT
	if(_msvcore_memcon_data::filetime + USEMSV_MEMORYCONTROL_FILE_TIME < tm){
		_msvcore_memcon_data::filetime = tm;
		msvcore_memcon_print(USEMSV_MEMORYCONTROL_FILE_PRINT);		
	}
#endif

#ifdef USEMSV_MEMORYCONTROL_FINDLEAK
	int findleak = 0;
	if(item->id == USEMSV_MEMORYCONTROL_FINDLEAK){
		findleak = 1;
		sz += sizeof(msvcore_memon_findleak);
	}
#endif

	if(prn)
		msvcore_memcon_print();

	unsigned char *ret = (unsigned char*)malloc(sz);

#ifdef USEMSV_MEMORYCONTROL_FINDLEAK
	if(findleak){
		msvcore_memon_findleak *p = (msvcore_memon_findleak*)ret;

		MemConLock();

		if(!msvcore_memon_findleak_a){
			p->_n = 0;
		} else{
			p->_n = msvcore_memon_findleak_a;
			msvcore_memon_findleak_a->_p = p;
		}

		msvcore_memon_findleak_a = p;
		p->_p = 0;

		p->crc = msvcore_memcon_stack_crc();

#if USEMSV_MEMORYCONTROL_FINDLEAK_PRINT
		if(msvcore_memon_findleak_lprint + USEMSV_MEMORYCONTROL_FINDLEAK_PRINT < time(0)){
			msvcore_memon_findleak_lprint = time(0);
			msvcore_memcon_findleak_print();
		}
#endif

		MemConUnLock();

		ret += sizeof(msvcore_memon_findleak);
	}
#endif

	return ret;

}

void msvcore_memcon_free(void *v, _msvcore_memcon_data *item){
#ifdef USEMSV_MEMORYCONTROL_FINDLEAK
	if(item->id == USEMSV_MEMORYCONTROL_FINDLEAK){
		msvcore_memon_findleak *p = (msvcore_memon_findleak*)((unsigned char*)v - sizeof(msvcore_memon_findleak));
		
		MemConLock();
		if(p == msvcore_memon_findleak_a){
			msvcore_memon_findleak_a = p->_n;
			msvcore_memon_findleak_a->_p = 0;
		} else{
			p->_p->_n = p->_n;
			p->_n->_p = p->_p;
		}

		MemConUnLock();


		v = p;
	}
#endif

	return free(v);
}

_msvcore_memcon_data* msvcore_memcon_find(int id, const char *name){
	for(int i = 0; i < msvcore_memcon_data_sz; i++){
		if(msvcore_memcon_data[i].id == id)
			return msvcore_memcon_data + i;
	}

	if(msvcore_memcon_data_sz % 128 == 0){
		_msvcore_memcon_data *d = (_msvcore_memcon_data*)malloc(sizeof(_msvcore_memcon_data) * (msvcore_memcon_data_sz + 128));
		if(!d)
			return 0;

		memcpy(d, msvcore_memcon_data, sizeof(_msvcore_memcon_data) * msvcore_memcon_data_sz);
		free(msvcore_memcon_data);
		msvcore_memcon_data = d;
	}

	msvcore_memcon_data[msvcore_memcon_data_sz].init(id, name);
	return &msvcore_memcon_data[msvcore_memcon_data_sz++];
}

_msvcore_memcon_data* msvcore_memcon_malloc_c(int tid, const char *name){
	MemConLock();
	_msvcore_memcon_data *item = msvcore_memcon_find(tid, name);
	item->m();

//#if USEMSV_MEMORYCONTROL_GLOBAL && USEMSV_MEMORYCONTROL_TEST1
//	if(!msvcore_memcon_data_void){
//		msvcore_memcon_data_void = (void**)malloc(S16K * sizeof(void*));
//		memset(msvcore_memcon_data_void, 0, S16K * sizeof(void*));
//	}
//
//	for(int i = 0; i < S16K; i++)
//		if(!msvcore_memcon_data_void[i]){
//			msvcore_memcon_data_void[i] = mem;
//			break;
//		}	
//#endif

	msvcore_memcon_malloc_count ++;

	MemConUnLock();
	return item;
}

_msvcore_memcon_data* msvcore_memcon_free_c(int tid, void *mem){
	MemConLock();
	_msvcore_memcon_data *item = msvcore_memcon_find(tid, 0);
	item->f();

#if USEMSV_MEMORYCONTROL_GLOBAL && USEMSV_MEMORYCONTROL_TEST1
	int f = 0;

	for(int i = 0; i < S16K; i++)
		if(msvcore_memcon_data_void[i] == mem){
			msvcore_memcon_data_void[i] = 0;
			f = 1;
			break;
		}

	if(!f || !mem)
		int ff = 1; // print("Fail free().\r\n");

#endif

	msvcore_memcon_free_count ++;

	MemConUnLock();
	return item;
}	


void* msvcore_memcon_malloc2(int tid, void *t, const char *name, int size, int mallocsize){
	_msvcore_memcon_data *item = msvcore_memcon_malloc_c(tid, name);
	void *ret = msvcore_memcon_malloc(mallocsize, item);
	return ret;
}

void msvcore_memcon_free2(int tid, void *t, const char *name, int size, void *freev){
	if(!freev)
		return ;

	_msvcore_memcon_data *item = msvcore_memcon_free_c(tid, freev);
	return msvcore_memcon_free(freev, item);
}


	#ifdef USEMSV_MSVCORE
		#define malloc(v) msvcore_memcon_malloc2(typeid(*this).hash_code(), this, typeid(*this).name(), sizeof(*this), v)
		#define free(v) msvcore_memcon_free2(typeid(*this).hash_code(), this, typeid(*this).name(), sizeof(*this), v)

		void * operator new(size_t sz) throw(){
			return msvcore_memcon_malloc2(1, 0, "class Malloc", 0, sz);
			//return msvcore_memcon_malloc(sz);
		}

		void operator delete(void * p) throw(){
			return msvcore_memcon_free2(1, 0, "class Malloc", 0, p);
			//return msvcore_memcon_free(p);
		}

	#endif

#endif