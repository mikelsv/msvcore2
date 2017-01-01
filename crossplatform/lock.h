#if defined(WIN32) && !defined(__GNUC__)
	#define WIN32_CRITICAL_SECTIONS
	#define PRTHREAD_COND_SIGNAL 0

	typedef CRITICAL_SECTION pthread_mutex_t;
	typedef CONDITION_VARIABLE pthread_cond_t;
#endif

// UgLock
#define UGLOCKT() ALock __l(this);
#define UGLOCK(l) ALock __l(l);
#define UGLOCK2(l) ALock __l2(l);
#define UGLOCKD(l, r) ALock __ld(l); ALock __rd(r);


#define ALOCKT() ALock __l(this);
#define ALOCK(l) ALock __l(l);
#define ALOCK2(l) ALock __l2(l);
#define ALOCKD(l, r) ALock __ld(l); ALock __rd(r);

// Global lock: Critical sections. Use TLock.
class GLock{
	// Section
	pthread_mutex_t shared_mutex;

	// Locked & first lock = !flock;
	bool lock;
	bool flock;

public:
	GLock();
	~GLock();

	bool Lock();
	bool UnLock();
};


// Thread Lock : Critical sections
class TLock{
protected:
	// Section & thread id
	pthread_mutex_t shared_mutex;
	DWORD threadid;
	
	// Locked count & first lock = !flock;
	int lock;
	bool flock;

public:

	TLock();
	~TLock();

	void Init();
	bool IsLock();
	int Count();
	
	bool Lock();
	bool UnLock();

};

// Conditional Lock
class CLock{
	// Section & condition
	pthread_mutex_t shared_mutex;
	pthread_cond_t shared_cond;

	// Locked & first lock = !flock;
	int lock;
	bool flock;

public:
	CLock();
	~CLock();

private:
	void Init();

public:
	bool Lock();
	bool UnLock();

	void Wait(int lockit = 1);
	void Signal();
};


// Auto Lock & UnLock
class ALock{
	// Lock pointer
	union {
		GLock *gl;
		TLock *tl;
	};

	// Type
	int type;

public:
	ALock();
	ALock(GLock &g);
	ALock(TLock &t);
	ALock(GLock *g);
	ALock(TLock *t);

	void Lock(GLock *g);
	void Lock(TLock *t);
	void operator=(GLock *g);
	void operator=(TLock *t);

	void set(GLock *g);

	void UnLock();
	~ALock();
};