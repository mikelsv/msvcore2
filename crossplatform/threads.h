#ifndef WIN32
//#include "pthread.h"
#define LPSECURITY_ATTRIBUTES pthread_attr_t*
#endif

typedef DWORD (*LPTHREAD_METHOD)(LPVOID pParam);

//#ifdef USEMSV_TUPLE
//#ifndef USEMSV_ANDROID
//typedef DWORD (*LPTHREAD_METHOD_T)(MTuple &_tuple);
//#endif
//#endif

// Статическая функция, которая запустит метод.
//static  DWORD StartFunc (LPSTARTUP_PARAM pStartup);

// Структура параметров для статической функции.
typedef struct STARTUP_PARAM {
    LPTHREAD_METHOD		pMethod;
    LPVOID				pParam;
	HANDLE				hdl;
} *LPSTARTUP_PARAM;

DWORD StartFunc(LPSTARTUP_PARAM pStartup);
HANDLE StartThread(LPTHREAD_METHOD pMethod, LPVOID pParam = 0, LPDWORD pdwThreadID = 0, const LPSECURITY_ATTRIBUTES pSecurity = 0, DWORD dwStackSize = 0, DWORD dwFlags = 0);


// COT: Class One Thread
class MSVCOT;

DWORD MSVCOT_RunCot(LPVOID lp);

struct MSVCOTD{
	LPVOID lp;
	MSVCOT *cot;
};

class MSVCOT{
public:
	DWORD cottid, cotuse;

protected:
	MSVCOT();

public:
	// run in class for memory control, dont use this functions
	MSVCOT(unsigned short u);

public:
	virtual DWORD COT(LPVOID lp);
	int COTS(LPVOID lp = 0);

	//void KillCot(int wl=20){
	//	DWORD ct=cottid;
	//	if(cottid && cottid!=0xdeaddead) cottid=0xdeaddead;
	//	if(cottid) Sleep(wl);
	//	if(cottid) KillThread(ct);
	//}

	DWORD MSVCOT_RunCot(LPVOID lp);

	void StopCot();
};



// Multi COT
class MSVMCOT;

DWORD MSVMCOT_RunCot(LPVOID lp);

struct MSVMCOTD{
	LPVOID lp;
	MSVMCOT*cot;
};

class MSVMCOT{
public:
	DWORD cottid;

protected:
	MSVMCOT();

public:
	// run in class for memory control, dont use this functions
	MSVMCOT(unsigned short u);

	virtual DWORD MCOT(LPVOID lp);

	//void KillCot(int wl=20){
	//	DWORD ct=cottid;
	//	if(cottid && cottid!=0xdeaddead) cottid=0xdeaddead;
	//	if(cottid) Sleep(wl);
	//	if(cottid) KillThread(ct);
	//}

	int MCOTS(LPVOID lp = 0);

	DWORD MSVMCOT_RunCot(LPVOID lp);

	void StopCot();

};