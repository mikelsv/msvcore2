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
