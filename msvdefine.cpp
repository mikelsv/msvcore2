// Include Base
#include "define/base.cpp"

#ifdef UNICODE
	#include "crossplatform/wince-twoms.cpp"
#endif

// Include Lock
#include "crossplatform/lock.cpp"

// Include Threads
#include "crossplatform/threads.cpp"

// Include Network
#include "crossplatform/net.cpp"

// Include Send & Recv
#include "crossplatform/send.cpp"

// Include Str Int
#include "crossplatform/strint.cpp"

// Include I/O
#include "crossplatform/io.cpp"

// Include Config Line Options
#include "crossplatform/conflineops.cpp"

// Msv Core State
//class MsvCoreState{
MsvCoreState::MsvCoreState(){
	procupped = (unsigned int)time(0);
}

void MsvCoreState::Main(int args, char* argv[], char* arge[]){
	char bpath[S8K];
	LString cmd;

	this->args = args;
	this->argv = argv;
	this->arge = arge;
		
	if(args)
		cmd + VString(argv[0]) + "?";

	for(int i = 1; i < args; i ++)
		cmd + VString(argv[i]) + (i + 1 < args ? (" ") : (""));	

	link.Link((VString)cmd);

	thispath = NormalPath(_getcwd(bpath, S8K));
	
	if(link.GetOnlyPath()){
		_chdir(SString(link.GetOnlyPath()));

		procpath = NormalPath(_getcwd(bpath, S8K));

		_chdir(thispath);
	}

#ifdef WIN32
	GetWindowsDirectory();
#endif

#ifdef USEMSV_ANDROID
	SString tmp;
	sstat64 stt = GetFileInfo(tmp.Add("/sdcard/Android/data/", PROJECTNAME));
		
	if((stt.st_mode & S_IFMT) == S_IFDIR){
		thispath = tmp;
		chdir(thispath);
	}
	else{
		stt = GetFileInfo(tmp.Add("/data/data/", PROJECTNAME));
		if((stt.st_mode & S_IFMT) == S_IFDIR){
			thispath = tmp;
			chdir(thispath);
		}
	}
	signal(SIGABRT, SIG_IGN);
#endif

#ifndef WIN32
	// Запуск как демон под Линукс
	if(link.Find("--background"))
		daemon(1,0);
#endif

	link.FirstArg();

	return ;
}

VString MsvCoreState::NormalPath(char *path){
	int psz = path ? strlen(path) : 0;

	if(psz && *(path - 1) != '\\' && *(path - 1) != '/'){
		path[psz] = '/';
		psz ++;
	}

	return VString(path, psz);
}

void MsvCoreState::GetWindowsDirectory(){
	SStringX<S4K> tmp;
	tmp.Reserve(S4K - 1); 

#ifdef WIN32
#ifdef UNICODE
	int s = ::GetWindowsDirectory((LPWSTR)tmp.rchar(), tmp.size()/2);
	winpath = stoc((unsigned short*)tmp.rchar(), s); 
#else
	winpath = tmp.str(0, ::GetWindowsDirectory(tmp, tmp.size()));
#endif
#endif

	return ;
}

//};
MsvCoreState msvcorestate;

//
int msvcoremain(int args, char* arg[], char* env[]){
	msvcorestate.Main(args, arg, env);
return 1;
}