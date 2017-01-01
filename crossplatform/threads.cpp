DWORD StartFunc(LPSTARTUP_PARAM pStartup){
	// Распаковываем данные в новом потоке.
	LPTHREAD_METHOD pMethod	= pStartup->pMethod;
	LPVOID pParam			= pStartup->pParam;

#if(MSV_PROC_STAT)
_msvps_thread_cnt++;
_msvps_thread_is++;
#endif
    // Запускаем функцию в новом потоке.
    DWORD dwResult = (*pMethod)(pParam);
#if(MSV_PROC_STAT)
_msvps_thread_is--;
#endif

    // Удаляем временные данные и возвращаем код возврата из нового потока.
#ifdef WIN32
	CloseHandle(pStartup->hdl);
#endif
	delete pStartup;
    return dwResult;
}


HANDLE StartThread(LPTHREAD_METHOD pMethod, LPVOID pParam, LPDWORD pdwThreadID, const LPSECURITY_ATTRIBUTES pSecurity, DWORD dwStackSize, DWORD dwFlags){
    // Создаем структуру и упаковываем данные для статической функции.
    LPSTARTUP_PARAM pStartup = new STARTUP_PARAM;
    pStartup->pMethod	= pMethod;
    pStartup->pParam 	= pParam;

    // Создаем новый поток.
#ifdef WIN32
	 pStartup->hdl=CreateThread(pSecurity, dwStackSize, (LPTHREAD_START_ROUTINE)StartFunc, pStartup, dwFlags, pdwThreadID);
	 if(!ishandle(pStartup->hdl)){ delete pStartup; return 0; }
	 return pStartup->hdl;
#else
	pthread_t restrict; if(pdwThreadID) *pdwThreadID=0;
	if(pthread_create(&restrict, pSecurity, (void* (*)(void*))StartFunc, pStartup)){ return 1; }
	if(pdwThreadID) *pdwThreadID=1;
	pthread_detach(restrict);
	return 0;
#endif
}

#ifdef USEMSV_TUPLE
#ifndef USEMSV_ANDROID
/////////////////////////////////////////////// TUPLE
typedef struct STARTUP_PARAM_T {
    LPTHREAD_METHOD_T		pMethod;
    MTuple				pParam;
	HANDLE				hdl;
} *LPSTARTUP_PARAM_T;

DWORD StartFuncT(LPSTARTUP_PARAM_T pStartup){
	// Распаковываем данные в новом потоке.
	LPTHREAD_METHOD_T pMethod	= pStartup->pMethod;
	MTuple &pParam			= pStartup->pParam;

#if(MSV_PROC_STAT)
_msvps_thread_cnt++;
_msvps_thread_is++;
#endif
    // Запускаем функцию в новом потоке.
    DWORD dwResult = (*pMethod)(pParam);
#if(MSV_PROC_STAT)
_msvps_thread_is--;
#endif

    // Удаляем временные данные и возвращаем код возврата из нового потока.
#ifdef WIN32
	CloseHandle(pStartup->hdl);
#endif
	delete pStartup;
    return dwResult;
}

HANDLE StartThreadT(LPTHREAD_METHOD_T pMethod, MTuple &pParam, LPDWORD pdwThreadID=0, const LPSECURITY_ATTRIBUTES pSecurity=0, DWORD dwStackSize=0, DWORD dwFlags=0){
    // Создаем структуру и упаковываем данные для статической функции.
    LPSTARTUP_PARAM_T pStartup = new STARTUP_PARAM_T;
    pStartup->pMethod	= pMethod;
    pStartup->pParam	= pParam;

    // Создаем новый поток.
#ifdef WIN32
	 pStartup->hdl=CreateThread(pSecurity, dwStackSize, (LPTHREAD_START_ROUTINE)StartFuncT, pStartup, dwFlags, pdwThreadID);
	 if(!ishandle(pStartup->hdl)){ delete pStartup; return 0; }
	 return pStartup->hdl;
#else
	pthread_t restrict; if(pdwThreadID) *pdwThreadID=0;
	if(pthread_create(&restrict, pSecurity, (void* (*)(void*))StartFuncT, pStartup)){ return 1; }
	if(pdwThreadID) *pdwThreadID=1;
	pthread_detach(restrict);
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
#endif
#endif



// COT: Class One Thread
// MSVCOT class //
MSVCOT::MSVCOT(){
	cottid=0;
	cotuse = 0;
}

// run in class for memory control, dont use this functions
MSVCOT::MSVCOT(unsigned short u){ cottid=0; }

DWORD MSVCOT::COT(LPVOID lp){
	//while(cotuse) Sleep(1);
	return 0;
}

//void KillCot(int wl=20){
//	DWORD ct=cottid;
//	if(cottid && cottid!=0xdeaddead) cottid=0xdeaddead;
//	if(cottid) Sleep(wl);
//	if(cottid) KillThread(ct);
//}

int MSVCOT::COTS(LPVOID lp){
	MSVCOTD *cot; // new MSVCOTD;
	msvcorenew(cot, MSVCOTD);
	cot->cot = this; cot->lp = lp;
	StopCot(); cot->cot->cotuse = 1;
	StartThread(::MSVCOT_RunCot, cot, &cottid, 0, 0, 0);
return 1;
}

DWORD MSVCOT::MSVCOT_RunCot(LPVOID lp){
	MSVCOTD *cot = (MSVCOTD*)lp;
	cot->cot->COT(cot->lp);
	cot->cot->cottid = 0;
	//delete cot;
	msvcoredelete(cot, MSVCOTD);
	return 0;
}

void MSVCOT::StopCot(){
	if(cottid)
		cotuse = 0;
		
	while(cottid)
		Sleep(2);
}
// MSVCOT class ~ //

unsigned short MSVCOTFORMEMORYCONTROL_US;
MSVCOT MSVCOTFORMEMORYCONTROL(MSVCOTFORMEMORYCONTROL_US);


DWORD MSVCOT_RunCot(LPVOID lp){
	return MSVCOTFORMEMORYCONTROL.MSVCOT_RunCot(lp);
}


//DWORD MSVCOT_RunCot(LPVOID lp){
//	MSVCOTD*cot=(MSVCOTD*)lp;
//	cot->cot->COT(cot->lp); cot->cot->cottid=0;
//	delete cot; return 0;
//}


// Multi COT
class MSVMCOT;

DWORD MSVMCOT_RunCot(LPVOID lp);

// MSVMCOT class //
MSVMCOT::MSVMCOT(){
	cottid = 0;
}

DWORD MSVMCOT::MCOT(LPVOID lp){
	//while(cotuse) Sleep(1);
	return 0;
}

//void KillCot(int wl=20){
//	DWORD ct=cottid;
//	if(cottid && cottid!=0xdeaddead) cottid=0xdeaddead;
//	if(cottid) Sleep(wl);
//	if(cottid) KillThread(ct);
//}

int MSVMCOT::MCOTS(LPVOID lp){
	MSVMCOTD *cot; // = new MSVMCOTD;
	msvcorenew(cot, MSVMCOTD);
	cot->cot = this; cot->lp = lp;
	StartThread(::MSVMCOT_RunCot, cot, &cottid, 0, 0, 0);
	return 1;
}

// run in class for memory control, dont use this functions
MSVMCOT::MSVMCOT(unsigned short u){ cottid=0; }

DWORD MSVMCOT::MSVMCOT_RunCot(LPVOID lp){
	MSVMCOTD *cot = (MSVMCOTD*)lp;
	cot->cot->MCOT(cot->lp);
	cot->cot->cottid = 0;
	//delete cot;
	msvcoredelete(cot, MSVMCOTD);
	return 0;
}
	
void MSVMCOT::StopCot(){}
// MSVMCOT class ~ //

unsigned short MSVMCOTFORMEMORYCONTROL_US;
MSVMCOT MSVMCOTFORMEMORYCONTROL(MSVMCOTFORMEMORYCONTROL_US);


DWORD MSVMCOT_RunCot(LPVOID lp){
	return MSVMCOTFORMEMORYCONTROL.MSVMCOT_RunCot(lp);
}