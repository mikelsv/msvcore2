#if defined(WIN32) && !defined(__GNUC__)

void pthread_mutex_init(pthread_mutex_t *shared_mutex, void*){
#ifndef WINCE
	if(shared_mutex->LockCount >= 0 && !shared_mutex->OwningThread && !shared_mutex->LockSemaphore){ InitializeCriticalSection(shared_mutex);  }
#else
	if(shared_mutex->LockCount == 0 && !shared_mutex->hCrit && !shared_mutex->LockSemaphore){ InitializeCriticalSection(shared_mutex); }
#endif
}

void pthread_mutex_lock(pthread_mutex_t *shared_mutex){
	EnterCriticalSection(shared_mutex);
}

void pthread_mutex_unlock(pthread_mutex_t *shared_mutex){
	LeaveCriticalSection(shared_mutex);
}

void pthread_mutex_destroy(pthread_mutex_t *shared_mutex){
	DeleteCriticalSection(shared_mutex);
}

void WINAPI pthread_cond_init_winxp(pthread_cond_t *cond){
	cond->Ptr = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void WINAPI pthread_cond_wait_winxp(pthread_cond_t *cond, pthread_mutex_t *mutex, DWORD tm){
	pthread_mutex_unlock(mutex);
	WaitForMultipleObjects(1, (HANDLE*)&cond->Ptr, FALSE, tm);
	pthread_mutex_lock(mutex);
}

void WINAPI pthread_cond_signal_winxp(pthread_cond_t *cond){
  // Try to release one waiting thread.
  //PulseEvent(cv->events_[PRTHREAD_COND_SIGNAL]);
	SetEvent(cond->Ptr);
}

typedef void (WINAPI *t_InitializeConditionVariable)(pthread_cond_t *external_cond);
typedef void (WINAPI *t_SleepConditionVariable)(pthread_cond_t *external_cond, pthread_mutex_t *external_mutex, DWORD tm);
typedef void (WINAPI *t_WakeConditionVariable)(pthread_cond_t *external_cond);

t_InitializeConditionVariable f_InitializeConditionVariable = 0;
t_SleepConditionVariable f_SleepConditionVariable = 0;
t_WakeConditionVariable f_WakeConditionVariable = 0;


class __UniqClassConditionVariable{

public:
	__UniqClassConditionVariable(){
		HMODULE module = GetModuleHandle(MODUNICODE("kernel32"));

		if(module){
			f_InitializeConditionVariable = (t_InitializeConditionVariable)GetProcAddress(module, "InitializeConditionVariable");
			f_SleepConditionVariable = (t_SleepConditionVariable)GetProcAddress(module, "SleepConditionVariableCS");
			f_WakeConditionVariable = (t_WakeConditionVariable)GetProcAddress(module, "WakeConditionVariable");
		}

		if(!f_InitializeConditionVariable || !f_SleepConditionVariable || !f_WakeConditionVariable){
			f_InitializeConditionVariable = pthread_cond_init_winxp;
			f_SleepConditionVariable = pthread_cond_wait_winxp;
			f_WakeConditionVariable = pthread_cond_signal_winxp;
		}

		return ;
	}

} __UniqClassConditionVariable;

void pthread_cond_init(pthread_cond_t *external_cond, const void*){
	f_InitializeConditionVariable(external_cond);
}

void pthread_cond_wait(pthread_cond_t *external_cond, pthread_mutex_t *external_mutex){
	f_SleepConditionVariable(external_cond, external_mutex, INFINITE);
}

void pthread_cond_signal(pthread_cond_t *external_cond){
	f_WakeConditionVariable(external_cond);
}

#endif


//class GLock{
	

GLock::GLock(){
	memset(this, 0, sizeof(GLock));
	pthread_mutex_init(&shared_mutex, NULL);
	return ;
}

GLock::~GLock(){
	if(lock)
		UnLock();
	pthread_mutex_destroy(&shared_mutex);
}

bool GLock::Lock(){
#ifdef WIN32_CRITICAL_SECTIONS
#ifndef WINCE
	if(!flock && shared_mutex.LockCount>=0 && !shared_mutex.OwningThread && !shared_mutex.LockSemaphore){
		InitializeCriticalSection(&shared_mutex); flock=1; }
#else
	if(!flock && shared_mutex.LockCount==0 && !shared_mutex.hCrit && !shared_mutex.LockSemaphore){ InitializeCriticalSection(&shared_mutex); flock=1; }
#endif
#endif

#ifdef WIN32_CRITICAL_SECTIONS
	int llc=shared_mutex.LockCount;
	HANDLE t=shared_mutex.OwningThread;
	EnterCriticalSection(&shared_mutex);
	if(lock || !shared_mutex.OwningThread){ // || t
		globalerror("GLOCK Lock!\r\n");
	}
#else
	pthread_mutex_lock(&shared_mutex);
	if(lock){
		globalerror("GLOCK Lock!\r\n");
	}
#endif

	lock=1; return 1;
}

bool GLock::UnLock(){
#ifdef WIN32_CRITICAL_SECTIONS
	if(!lock || !shared_mutex.OwningThread){
		globalerror("GLOCK UnLock!\r\n");
	}
#else
	if(!lock){
		globalerror("GLOCK UnLock!\r\n");
	}
#endif
	lock=0;
	pthread_mutex_unlock(&shared_mutex);
	
	return 1;
}

//};

GLock tlocklock;


//class TLock{
TLock::TLock(){
	Init();	
}

TLock::~TLock(){
	if(lock)
		UnLock();
	pthread_mutex_destroy(&shared_mutex);
}

void TLock::Init(){
	memset(this, 0, sizeof(*this));
	pthread_mutex_init(&shared_mutex, NULL);
	flock = 1;
}


bool TLock::IsLock(){
	DWORD thid = GetCurrentThreadId();
	if(lock && thid != threadid)
		return 1;

	return 0;
}

int TLock::Count(){
	return lock;
}

bool TLock::Lock(){
#ifdef WIN32
	if(!flock){
		tlocklock.Lock();
		Init();
		tlocklock.UnLock();
	}
#endif

	DWORD thid = GetCurrentThreadId();
	if(lock && thid == threadid){
		lock ++;
		return 1;
	}

#ifdef WIN32_CRITICAL_SECTIONS
	int llc = shared_mutex.LockCount;
	HANDLE t = shared_mutex.OwningThread;

	try{
		pthread_mutex_lock(&shared_mutex);
	}catch(...){
		return 0;
	}
	if(lock || !shared_mutex.OwningThread){ // || t
		globalerror("GLOCK Lock!\r\n");
	}
#else
	pthread_mutex_lock(&shared_mutex);
	if(lock){
		globalerror("GLOCK Lock!\r\n");
	}
#endif

	lock=1; threadid=thid; return 1;
}

bool TLock::UnLock(){
	DWORD thid=GetCurrentThreadId();

	if(lock>1){
		if(thid != threadid){
			globalerror("TLOCK UnLock thread id!\r\n"); return 0;
		}
		lock--; return 1;
	}

#ifdef WIN32_CRITICAL_SECTIONS
	if(!lock || !shared_mutex.OwningThread){
		globalerror("GLOCK UnLock!\r\n");
	}

	lock = 0;
	threadid = 0;
	LeaveCriticalSection(&shared_mutex);
#else
	if(!lock){
		globalerror("GLOCK UnLock!\r\n");
	}
	lock = 0;
	threadid = 0;
	pthread_mutex_unlock(&shared_mutex);
#endif
	return 1;
}

//};



// Conditional Lock
//class CLock{
CLock::CLock(){
	Init();
}

void CLock::Init(){
	memset(this, 0, sizeof(*this));
	pthread_mutex_init(&shared_mutex, NULL);
	pthread_cond_init(&shared_cond, 0);
	flock = 1;
}

bool CLock::Lock(){
#ifdef WIN32
	if(!flock){
		tlocklock.Lock();
		Init();
		tlocklock.UnLock();
	}
#endif

	pthread_mutex_lock(&shared_mutex);
	lock = 1;
	return 1;
}

bool CLock::UnLock(){
	lock = 0;
	pthread_mutex_unlock(&shared_mutex);
	return 1;
}

void CLock::Wait(int lockit){
	if(lockit)
		pthread_mutex_lock(&shared_mutex);

	pthread_cond_wait(&shared_cond, &shared_mutex);

	if(lockit)
		pthread_mutex_unlock(&shared_mutex);
}

void CLock::Signal(){
	//if(lockit)
	//	pthread_mutex_lock(&shared_mutex);

	pthread_cond_signal(&shared_cond);

	//if(lockit)
	//	pthread_mutex_unlock(&shared_mutex);
}
	
CLock::~CLock(){
#ifdef WIN32
	//CloseHandle(shared_cond.events[PRTHREAD_COND_SIGNAL]);
	//CloseHandle(shared_cond.events[PRTHREAD_COND_BROADCAST]);		
#else
	pthread_cond_destroy(&shared_cond);
#endif
}
//};


// Auto Lock & UnLock
//class ALock{
ALock::ALock(){ gl = 0; type = 0; }
ALock::ALock(GLock &g) : gl(&g){ g.Lock(); type=1; }
ALock::ALock(TLock &t) : tl(&t){ t.Lock(); type=2; }
ALock::ALock(GLock *g) : gl(g){ /*if(g){*/ g->Lock(); type = 1; /*} else tp=0;*/ }
ALock::ALock(TLock *t) : tl(t){ /*if(t){*/ t->Lock(); type = 2; /*} else tp=0;*/ }

void ALock::Lock(GLock *g){ gl = g; gl->Lock(); type = 1; return ; }
void ALock::Lock(TLock *t){ tl = t; tl->Lock(); type = 2; return ; }
void ALock::operator=(GLock *g){ UnLock(); gl = g; gl->Lock(); type = 1; return ; }
void ALock::operator=(TLock *t){ UnLock(); tl = t; tl->Lock(); type = 2; return ; }

void ALock::UnLock(){ if(type == 1) gl->UnLock(); else if(type == 2) tl->UnLock(); type = 0; }
ALock::~ALock(){ if(type == 1) gl->UnLock(); else if(type == 2) tl->UnLock(); type = 0; }
//};


#ifdef USEMSV_MEMORYCONTROL
	TLock memconlock, memconsymlock;
	bool MemConLock(){ return memconlock.Lock(); }
	bool MemConUnLock(){ return memconlock.UnLock(); }
	bool MemConSymLock(){ return memconsymlock.Lock(); }
	bool MemConSymUnLock(){ return memconsymlock.UnLock(); }
#endif