//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// Pipeline class /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
#define PPLNULL	0
#define PPLPIPE	1
#define PPLRBUF	S8K

struct pipe_timeline{ unsigned int pos, tm, ml; };

#ifdef WIN32
#define pipe _pipe
#endif

class PipeEnv{
	MString d;
public:

	void New(int sz){
		d.Reserve(sz*sizeof(void*));
		return ;
	}

	void Set(int i, VString val){
		//if(d.size()/sizeof(void*)<i)

	}



};

// int maxpipelock=512, thispipelock=0;

class PipeLine{
	int upipe[2];
	int stat;

	VString env, inp;
	LString *tml;

	VString opt_dir;

	// For OutBuffer
	int isoutbuffer, isprocess;

	unsigned char buf[PPLRBUF];           //буфер ввода/вывода

#ifdef WIN32
	STARTUPINFO si;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;        //структура security для пайпов
	PROCESS_INFORMATION pi;

	HANDLE newstdin, newstdout, read_stdout, write_stdin;  //дескрипторы пайпов

	FILETIME StartTime, EndTime;
	FILETIME KernelTime, UserTime;

	unsigned long exitcode;  //код завершения процесса
	unsigned long bread;   //кол-во прочитанных байт
	unsigned long avail;   //кол-во доступных байт
#else
	int p_stdin[2], p_stdout[2], p_stderr[2], p_stdret[2], pid;// int *prc=&rc; int *rrc=new int;


#endif


public:
	int64 usertime, kerneltime, cputime;

	GLock pipelock;

	PipeLine(){
		stat=PPLNULL; isoutbuffer=0; isprocess=0;
		tml=0;
	}

	~PipeLine(){
#ifdef WIN32
		if(isprocess) EndProcess();
#else
		int rc; LString hls, ehls;
		if(isprocess) EndProcess(rc, hls, ehls, 0);
#endif
	}

	int Create(){
	//#ifdef __GNUC__

	//	if(pipe( upipe ) == -1 ) return 0;

	stat=PPLPIPE;
	return 1;
	}

	void OutBuffer(int i){ isoutbuffer=i; }
	unsigned char* GetData(){ return buf; }


#define bzero(a) memset(a, 0, sizeof(a)) //для сокращения писанины


#ifdef WIN32
#define pipe _pipe

	bool IsWinNT(){  //проверка запуска под NT
		OSVERSIONINFO osv;
		osv.dwOSVersionInfoSize = sizeof(osv);
		GetVersionEx(&osv);
		return (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	void ErrorMessage(char *str){  //вывод подробной информации об ошибке
	LPVOID msg;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // язык по умолчанию
		(LPTSTR) &msg,
		0,
		NULL
	);

	printf("%s: %s\n",str,msg);
	LocalFree(msg);
	}

	int fork(){ return 0; }
	int execle(char *){ return 0; }
#endif

	void SetEnv(VString line){ env=line; }
	void SetInput(VString line){ inp=line; }
	void SetTimeLine(LString &tm){ tml=&tm; }
	void SetTimeLine(){ tml=0; }

	void SetDir(VString v){
		opt_dir = v;
	}

#ifndef WIN32

int Run(MString cmd, int &rc, LString &hls, LString &ehls, int tocon=1){
cputime=0; kerneltime=0; usertime=0;

if(isprocess) EndProcess(rc, hls, ehls, tocon);

pipelock.Lock();
//if(thispipelock+4>maxpipelock) return 0;
if(pipe(p_stdin)<0){ Sleep(100);// print("IP. ");
	pid=0; while(pipe(p_stdin)<0 && pid++<5) Sleep(100);
	if(pid>=5){
		//close(p_stdin[0]); close(p_stdin[1]);
		//close(p_stdout[0]); close(p_stdout[1]);
		//close(p_stderr[0]); close(p_stderr[1]);
		print("Died on pipe(p_stdin, ", itos(errno), ")!\n"); pipelock.UnLock(); return 0;
	}
}
if(pipe(p_stdout)<0){ Sleep(10);// print("OP. ");
	pid=0; while(pipe(p_stdout)<0 && pid++<5) Sleep(100);
	if(pid>=5){
		close(p_stdin[0]); close(p_stdin[1]);
		//close(p_stdout[0]); close(p_stdout[1]);
		//close(p_stderr[0]); close(p_stderr[1]);
		print("Died on pipe(p_stdout, ", itos(errno), ")!\n"); pipelock.UnLock(); return 0;
	}
}
if(pipe(p_stderr)<0){ Sleep(10);// print("EP. ");
	pid=0; while(pipe(p_stderr)<0 && pid++<5) Sleep(100);
	if(pid>=5){
		close(p_stdin[0]); close(p_stdin[1]);
		close(p_stdout[0]); close(p_stdout[1]);
		//close(p_stderr[0]); close(p_stderr[1]);
		print("Died on pipe(p_stderr, ", itos(errno), ")!\n"); pipelock.UnLock(); return 0;
	}
}
if(pipe(p_stdret)<0){ Sleep(10);// print("RP. ");
	pid=0; while(pipe(p_stdret)<0 && pid++<5) Sleep(100);
	if(pid>=5){
		close(p_stdin[0]); close(p_stdin[1]);
		close(p_stdout[0]); close(p_stdout[1]);
		close(p_stderr[0]); close(p_stderr[1]);
		print("Died on pipe(p_stdret, ", itos(errno), ")!\n"); pipelock.UnLock(); return 0;
	}
}
//thispipelock+=4;
pipelock.UnLock();

if ((pid = fork())<0) { printf("Died on fork()!\n"); return 0; }

if(pid==0){
    close(p_stdin[1]); close(p_stdout[0]); close(p_stderr[0]); close(p_stdret[0]);

    dup2(p_stdin[0], fileno(stdin));
    dup2(p_stdout[1], fileno(stdout));
    dup2(p_stderr[1], fileno(stderr));
	

//	write( fileno(stdout), cmd, cmd);
	//*prc=3;

/*	ILink il; il.Ilink(cmd); char *p[S1K], *e[S1K];
	HLString arg; arg+il.GetPath()+il.file+_rc;
	VString a; il.FirstArg(); while(a=il.GetArg()){ arg+a+_rc; } arg+_rc; arg.oneline();

	// p,e.
	int i=0; 
	while(ln<to){
		p[i]=ln; i++; if(i>=S1K-1) break;
		while(ln<to && *ln!=0) ln++;
		while(ln<to && *ln==0) ln++;	
	} p[i]=0;*/

	char*ln=env, *to=ln+env.size();
	//ln=env; to=ln+env.size(); i=0;
	while(ln<to){
		if(*ln) putenv(ln);// printf(ln); printf("\r\n");
		//e[i]=ln; i++; if(i>=S1K-1) break;
		while(ln<to && *ln!=0) ln++;
		while(ln<to && *ln==0) ln++;	
	} //e[i]=0;

 //putenv(MString("DOCUMENT_ROOT=/home/server/vm/projects/MSV-SNT/Termo/http-serv/"));
 //putenv(MString("SCRIPT_FILENAME=/home/server/vm/projects/MSV-SNT/Termo/http-serv/index.php"));

	//rc=execve(il.GetPath()+il.file, p, e);

	rc=system(cmd);// if(rc<0) rc=0;
//	HLString ls; ls+"<"+itos(rc)+">"; write(fileno(stdout), ls.nomov(), ls.size());
	write(p_stdret[1], &rc, 4);

struct rusage ru;
rc=getrusage(RUSAGE_SELF, &ru);
if(rc){ memset(&ru.ru_utime, 0, sizeof(struct timeval)*2); }
write(p_stdret[1], &ru.ru_utime, sizeof(struct timeval)*2);

	close(p_stdin[0]); close(p_stdout[1]); close(p_stderr[1]); close(p_stdret[1]);
    quick_exit(0); //exit(0);
}

	if(pid>0){ close(p_stdin[0]); close(p_stdout[1]); close(p_stderr[1]); close(p_stdret[1]); }

	if(inp) write(p_stdin[1], inp, inp);

	isprocess=1;
	if(!isoutbuffer) Process(rc, hls, ehls, tocon);

	return 1;
}

int Process(int &rc, LString &hls, LString &ehls, int tocon=1){
	if(!isprocess) return 0;

	//char buf[S4K]; int rbufi=1, ebufi=1;
	fd_set rfds, efds; timeval tm; int sel, maxs=0;
	int rbufi=1, ebufi=1;

	while(1){
		FD_ZERO(&rfds); FD_ZERO(&efds); maxs=0; tm.tv_sec=1; tm.tv_usec=0;
		FD_SET(p_stdout[0], &rfds); FD_SET(p_stderr[0], &rfds); maxs=p_stdout[0]>p_stderr[0] ? p_stdout[0] : p_stderr[0];
		FD_SET(p_stdout[0], &efds); FD_SET(p_stderr[0], &efds); maxs++;

		sel=select(maxs, &rfds, 0, &efds, &tm);// print("S ");
		
		if(FD_ISSET(p_stdout[0], &rfds)){// print("R ");
			rbufi=read(p_stdout[0], buf, sizeof(buf));// if(rbufi<=0) break; // PPLRBUF

			if(rbufi < 0)
				rbufi = 0;
			
			if(tocon)
				print(VString(buf, rbufi));
			
			if(tml){
				pipe_timeline pt; pt.pos=hls.size();
				timeb tm=alltime(); pt.tm=tm.time; pt.ml=tm.millitm; *tml+VString((char*)&pt, sizeof(pt));
			}
			
			if(isoutbuffer)
				return rbufi;
			else
				hls.add((char*)buf, rbufi);
			
			if(rbufi>0)
				continue;
		}

		if(FD_ISSET(p_stderr[0], &rfds)){// print("RE ");
			ebufi=read(p_stderr[0], &buf, sizeof(buf));// if(bufi<0) break;

			if(ebufi < 0)
				ebufi = 0;
			
			if(tocon) print(VString(buf, ebufi));
			
			if(isoutbuffer) return ebufi;
			else ehls.add((char*)buf, ebufi);
			
			if(ebufi>0) continue;
			//if(ebufi>0) break;
		}

		if(FD_ISSET(p_stdout[0], &efds) || FD_ISSET(p_stderr[0], &efds) || ( rbufi<=0 && ebufi<=0)){ break; }

		if(sel<0) break;
	}

	return EndProcess(rc, hls, ehls, tocon);
}

int EndProcess(int &rc, LString &hls, LString &ehls, int tocon=1){
	if(!isprocess) return 0;
	fd_set rfds, efds; timeval tm; int sel, maxs=0;
	int ebufi;



//	print("s:", itos(sel), " ");

//}
/*	  
	while((bufi=read(p_stdout[0], &buf, S4K))>0){
		print(VString(buf, bufi));
		hls.add(buf, bufi);
	}

	while((bufi=read(p_stderr[0], &buf, S4K))>0){
		print(VString(buf, bufi));
		ehls.add(buf, bufi);
	}
*/

	//HLString ls; ls+"("+itos(*rc)+")"; write(fileno(stdout), ls.nomov(), ls.size());
	read(p_stdret[0], &rc, 4);
	struct timeval ctm[2];
	int r=read(p_stdret[0], &ctm, sizeof(struct timeval)*2);
	if(r==sizeof(struct timeval)*2){  }
	usertime=int64(ctm[0].tv_sec)*1000000+ctm[0].tv_usec*1000;
	kerneltime=int64(ctm[1].tv_sec)*1000000+ctm[1].tv_usec*1000;
	cputime=kerneltime+usertime;

	r=waitpid(pid, 0, 0);// print(itos(r), ". ");
	//if(rbufi+ebufi<=0) return 0;
	//if(hls){ printf("HLS!!! "); } // printf(hls.oneline()); }
	//if(ehls) printf("EHLS!!! ");

	//printf("END!!! "); printf(itos(rc)); printf("\r\n");
	close(p_stdin[1]); close(p_stdout[0]); close(p_stderr[0]); close(p_stdret[0]);
	if(tml){ tml->oneline(); tml=0; }
	//pipelock.Lock(); thispipelock-=4; pipelock.UnLock();
	return 1;
}

#else // WIN 32
//#ifdef WIN32_NOUSEIT_456

int Run(MString cmd, int &rc, LString &hls, LString &ehls, int tocon=1){
	cputime=0; kerneltime=0; usertime=0; rc = -1;

	if(isprocess)
		EndProcess();

	if(IsWinNT()){	//инициализация security для Windows NT
		InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, true, NULL, false);
		sa.lpSecurityDescriptor = &sd;
	}
	else sa.lpSecurityDescriptor = NULL;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = true;	//разрешаем наследование дескрипторов

	if(!CreatePipe(&newstdin,&write_stdin,&sa,0)){	//создаем пайп для stdin
		ErrorMessage("CreatePipe");
		//getch();
		return 0;
	}

	if(!CreatePipe(&read_stdout,&newstdout,&sa,0)){ //создаем пайп для stdout
		ErrorMessage("CreatePipe");
		//getch();
		CloseHandle(newstdin);
		CloseHandle(write_stdin);
		return 0;
	}

	GetStartupInfo(&si);	//создаем startupinfo для дочернего процесса

  /*
  Параметр dwFlags сообщает функции CreateProcess
  как именно надо создать процесс.

  STARTF_USESTDHANDLES управляет полями hStd*.
  STARTF_USESHOWWINDOW управляет полем wShowWindow.
  */

	  si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
	  si.wShowWindow = SW_HIDE;
	  si.hStdOutput = newstdout;
	  si.hStdError = newstdout;   //подменяем дескрипторы для
	  si.hStdInput = newstdin;    // дочернего процесса

	//создаем дочерний процесс
	// ILink il; il.Ilink(cmd);

	if(!CreateProcess(NULL, MODUNICODE(cmd), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, env, MODUNICODE(opt_dir), &si, &pi)){
		//il.GetPath()+il.file, il.iquest
		//ErrorMessage("CreateProcess");
		//getch();
		CloseHandle(newstdin);
		CloseHandle(newstdout);
		CloseHandle(read_stdout);
		CloseHandle(write_stdin);
		return 0;
	}

	if(inp) WriteFile(write_stdin, inp, inp.size(), &bread, NULL);
	//CloseHandle(newstdin);
	//CloseHandle(write_stdin);


	bzero(buf);

	memset(&StartTime, 0, sizeof(StartTime)*4);
	exitcode=0;

	isprocess=1;
	if(!isoutbuffer) Process(rc, hls, ehls, tocon);

	return 1;
	}

	int Process(int &rc, LString &hls, LString &ehls, int tocon=1){
		if(!isprocess) return 0;

		//_INPUT_RECORD ir[4];
		//DWORD rd; //	SHIFT_PRESSED
		//ReadConsoleInputW(read_stdout, ir, 4, &rd);

		//CHAR_INFO ci[60]; COORD cd={10,10}, p={0, 0}; SMALL_RECT sr; memset(ci, 0, sizeof(ci));
		//int r=ReadConsoleOutput(read_stdout, ci, cd, p, &sr);

	for(;;){	//основной цикл программы
		GetExitCodeProcess(pi.hProcess, &exitcode); //пока дочерний процесс не закрыт

		PeekNamedPipe(read_stdout, buf, PPLRBUF-1, &bread, &avail, NULL); if(!bread) Sleep(1);

		//Проверяем, есть ли данные для чтения в stdout

    if(bread!=0){
		bzero(buf);
		while(avail){
			ReadFile(read_stdout,buf,PPLRBUF-1,&bread,NULL);  //читаем из пайпа stdout

			if(tml){
				pipe_timeline pt; pt.pos=hls.size();
				timeb tm=alltime(); pt.tm=(unsigned int)tm.time; pt.ml=tm.millitm; *tml+VString((char*)&pt, sizeof(pt));
			}

			if(tocon) printf("%s",buf);

			if(isoutbuffer) return bread;
			else{ hls+=VString((char*)&buf, bread); }
			
			avail-=bread; 

			bzero(buf); break;
		}
	}
	else Sleep(1);

		if(exitcode!=STILL_ACTIVE)
			break;
	
	}

	if(tml){ tml->oneline(); tml=0; }
	EndProcess();

	rc = exitcode;

	return 0;
	}

	int EndProcess(){
		if(!isprocess) return 0;
		isprocess=0;

		GetProcessTimes(pi.hProcess, &StartTime, &EndTime, &KernelTime, &UserTime);
		kerneltime=*(int64*)&KernelTime/10; usertime=*(int64*)&UserTime/10; cputime=kerneltime+usertime;

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		CloseHandle(newstdin);            //небольшая уборка за собой
		CloseHandle(newstdout);
		CloseHandle(read_stdout);
		CloseHandle(write_stdin);

		pi.hThread=0; pi.hProcess=0; newstdin=0; newstdout=0; read_stdout=0; write_stdin=0;

		return 1;
	}

#endif

#ifdef NOUSEIT_571
{
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwDummy;
	CONST COORD origin = { 0, 0 };
SECURITY_ATTRIBUTES sa; sa.nLength = sizeof(sa); sa.lpSecurityDescriptor = NULL; sa.bInheritHandle = TRUE;

HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,0,&sa,CONSOLE_TEXTMODE_BUFFER,NULL);
//FillConsoleOutputCharacter(hConsole, '\0', MAXLONG, origin, &dwDummy); // fill screen buffer with zeroes
//SetStdHandle(STD_OUTPUT_HANDLE, hConsole); // to be inherited by child process

PROCESS_INFORMATION pi; STARTUPINFO si; 
SECURITY_ATTRIBUTES sd;
HANDLE newstdin,newstdout,read_stdout,write_stdin;

InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
SetSecurityDescriptorDacl(&sd, true, NULL, false);
sa.lpSecurityDescriptor = &sd;

sa.nLength = sizeof(SECURITY_ATTRIBUTES);
sa.bInheritHandle = true;       //разрешаем наследование дескрипторов

if(!CreatePipe(&newstdin,&write_stdin,&sa,0)){
	print("#!PIPE ERROR!#\r\n"); return 0;
}

if (!CreatePipe(&read_stdout,&newstdout,&sa,0)) {
	print("#!PIPE ERROR!#\r\n"); return 0;
}
    HANDLE inWrite_, outRead_, process_;
	   HANDLE inRead, inWrite, outRead, outWrite, errWrite;


	 // Create non-inheritable copies of handle, our child process should not inherit
        HANDLE curProc = GetCurrentProcess();
     //   DuplicateHandle(curProc, write_stdin, curProc, &inWrite_, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
     //   DuplicateHandle(curProc, read_stdout, curProc, &outRead_, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

        // Create a copy of out handle for error output
    //    DuplicateHandle(curProc, newstdout, curProc, &errWrite, 0, TRUE, DUPLICATE_SAME_ACCESS);


GetStartupInfo(&si);      //создаем startupinfo для дочернего процесса

/*
  Параметр dwFlags сообщает функции CreateProcess как именно надо создать процесс.
  STARTF_USESTDHANDLES управляет полями hStd*.
  STARTF_USESHOWWINDOW управляет полем wShowWindow.
*/

si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
si.wShowWindow = SW_HIDE;
si.hStdOutput = _std_output_handle; //newstdout;
si.hStdError = newstdout;   //подменяем дескрипторы для
si.hStdInput = _std_input_handle;//  newstdin;    // дочернего процесса

//char app_spawn[] = "c:\\windows\\system32\\cmd.exe";
char app_spawn[] = "c:\\nc\\far\\far.exe";


//создаем дочерний процесс CREATE_NEW_CONSOLE DETACHED_PROCESS
if (!CreateProcess(app_spawn,NULL,NULL,NULL,0,CREATE_NEW_PROCESS_GROUP,NULL,NULL,&si,&pi)){
//	ErrorMessage("CreateProcess");
//	getch();
	CloseHandle(newstdin);
	CloseHandle(newstdout);
	CloseHandle(read_stdout);
	CloseHandle(write_stdin);
	print("#!command not found!#\r\n");
	return 0;
}

unsigned long exit=0;  //код завершения процесса
unsigned long bread;   //кол-во прочитанных байт
unsigned long avail;   //кол-во доступных байт
MString buf; buf.Reserv(S1K);


  for(;;)      //основной цикл программы
  { Sleep(10);
    GetExitCodeProcess(pi.hProcess,&exit); //пока дочерний процесс // не закрыт
    if (exit != STILL_ACTIVE) break;

    PeekNamedPipe(write_stdin, buf, 1023, &bread, &avail, NULL); //read_stdout
    //Проверяем, есть ли данные для чтения в stdout
	if (bread!=0) { buf.Clean(0);
		ReadFile(write_stdin, buf, 1023, &bread, NULL);
		//printf("%s",buf);
		print(buf.str(0, bread));

	}
//*
      if (avail > 1023*2)
      {
        while (bread >= 1023)
        {
          ReadFile(read_stdout,buf,1023,&bread,NULL);  //читаем из
                                                       // пайпа stdout
          printf("%s",buf);
         // bzero(buf);

        }
	  }

      else {

       // printf("%s",buf);
      }
	}//*/

/*	if(kbhit()){ //continue;
		ct=console.read(); ct.GetWinf(ir);
		if(ct.gett()==CNTKEY)
			int etety=56;
		unsigned short keys=ct.getkeys(), us=ct.getcode();
		unsigned char s=ct.getcode(); *buf=s;
		//WriteConsoleInput(write_stdin, &ir, 1, &rd);
		//WriteFile(write_stdin, &c, 1, &rd, 0);
	//	WriteFile(write_stdin,&s,1,&bread,NULL);
		//WriteConsoleOutput(write_stdin, &ir, 1, &rd);
    //if (kbhit())      //проверяем, введено ли что-нибудь с клавиатуры

      //bzero(buf);
	//	buf.Clean(0);
     // *buf = (char)getche();
* /


	if(ct.gett()==CNTKEY && keys&CN_DOWN){
      //printf("%c",*buf); //console.print(*buf.rchar());
		WriteFile(write_stdin,&s,1,&bread,NULL); //отправляем это в stdin
		//WriteConsoleOutput(write_stdin, &us, 0, 0, 0);
		//WriteConsoleInput(write_stdin, &ir, 1, &rd);
		if(!(ct.getkeys()&CN_KEY)) console.print(us);
		if (s == '\r') { s = '\n';
			WriteFile(write_stdin,&s,1,&bread,NULL); console.print(s); //формирум конец строки, если нужно
	}}
        //printf("%c",*buf);
		//console.print(*buf.rchar())
	}*/


  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  CloseHandle(newstdin);            //небольшая уборка за собой
  CloseHandle(newstdout);
  CloseHandle(read_stdout);
  CloseHandle(write_stdin);


  return 1;
}}
#endif

};

#define PIPELINE_STATE_NONE	0
#define PIPELINE_STATE_RDWR 1
#define PIPELINE_STATE_READ 2


class PipeLine2{
	// State
	int state;

	// Options
	int iscont; // Continue
	int isinput; // Input

	// Result
	SendDataRing input, output, errput;
	int64 usertime, kerneltime, cputime;
	int exitcode;

	// OS 
#ifdef WIN32
	STARTUPINFO			si;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;        //структура security для пайпов
	PROCESS_INFORMATION pi;

	HANDLE newstdin, newstdout, newstderr, write_stdin, read_stdout, read_stderr;  //дескрипторы пайпов

//	unsigned long exitcode;  //код завершения процесса
//	unsigned long bread;   //кол-во прочитанных байт
//	unsigned long avail;   //кол-во доступных байт
#else
	GLock pipelock;
	int p_stdin[2], p_stdout[2], p_stderr[2], p_stdret[2], pid;// int *prc=&rc; int *rrc=new int;
#endif

public:

	PipeLine2(){
		state = PIPELINE_STATE_NONE;
		iscont = 1;
		isinput = 1;

#ifdef WIN32
		newstdin = 0; newstdout = 0; newstderr = 0; write_stdin = 0; read_stdout = 0; read_stderr = 0;
#endif
	}

	SendDataRing& StdIn(){
		return input;
	}

	SendDataRing& StdOut(){
		return output;
	}

	SendDataRing& StdErr(){
		return errput;
	}

	void StdInClose(){
		isinput = 0;
	}

#ifdef WIN32

	int Run(VString cmd, VString dir = VString(), VString env = VString(), int isinput = 0){
		cputime = 0;
		kerneltime = 0;
		usertime = 0;
		exitcode = 0;

		if(state == PIPELINE_STATE_RDWR || state == PIPELINE_STATE_READ)
			EndProcess();

		if(IsWinNT()){
			//инициализация security для Windows NT
			InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
			SetSecurityDescriptorDacl(&sd, true, NULL, false);
			sa.lpSecurityDescriptor = &sd;
		}
		else
			sa.lpSecurityDescriptor = NULL;

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = true;	//разрешаем наследование дескрипторов

		if(!CreatePipe(&newstdin, &write_stdin, &sa, 0)){	//создаем пайп для stdin
			ErrorMessage("CreatePipe");
			return 0;
		}

		if(!CreatePipe(&read_stdout, &newstdout, &sa, 0)){ //создаем пайп для stdout
			ErrorMessage("CreatePipe");
			CloseHandle(newstdin);
			CloseHandle(write_stdin);

			newstdin = 0;
			write_stdin = 0;
			return 0;
		}

		if(!CreatePipe(&read_stderr, &newstderr, &sa, 0)){ //создаем пайп для stdout
			ErrorMessage("CreatePipe");
			CloseHandle(newstdin);
			CloseHandle(write_stdin);
			CloseHandle(read_stdout);
			CloseHandle(newstdout);

			newstdin = 0;
			write_stdin = 0;
			read_stdout = 0;
			newstdout = 0;
			return 0;
		}

		//создаем startupinfo для дочернего процесса
		GetStartupInfo(&si);

		// Параметр dwFlags сообщает функции CreateProcess как именно надо создать процесс.
		// STARTF_USESTDHANDLES управляет полями hStd*.
		// STARTF_USESHOWWINDOW управляет полем wShowWindow.

		si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.hStdInput = newstdin;	 //подменяем дескрипторы для
		si.hStdOutput = newstdout;	 // дочернего процесса
		si.hStdError = newstderr;

		SString scmd(cmd), sdir(dir);

		if(!CreateProcess(NULL, MODUNICODE(scmd), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, env, MODUNICODE(sdir), &si, &pi)){
			//il.GetPath()+il.file, il.iquest
			ErrorMessage("CreateProcess");
			//getch();
			EndProcess();
			return 0;
		}

		if(isinput)
			state = PIPELINE_STATE_RDWR;
		else
			state = PIPELINE_STATE_READ;

		if(iscont)
			Process();

		return 1;
	}

	int Process(){
		if(!state)
			return 0;

		unsigned char buf[S8K];
		DWORD bread, bwrite, avail, rd;
		DWORD ec;

		//основной цикл программы
		while(1){
			// пока дочерний процесс не закрыт
			GetExitCodeProcess(pi.hProcess, &ec);

			rd = 0;

			if(IsHandle(write_stdin)){
				if(input.IsRead()){
					bread = input.Read(VString(buf, sizeof(buf)));
					WriteFile(write_stdin, buf, bread, &bwrite, NULL);
					input.Readed(bwrite);
				}
				else if(!isinput){
					CloseHandle(write_stdin);
					CloseHandle(newstdin);
					write_stdin = 0;
					newstdin = 0;

					//signal(SIGINT, pi.dwProcessId);
				}
			}

			//Проверяем, есть ли данные для чтения в stdout
			PeekNamedPipe(read_stdout, buf, sizeof(buf) - 1, &bread, &avail, NULL);
			
			if(bread || avail){
				rd = 1;
				if(output.IsFree()){
					int s = minel(output.IsFree(), sizeof(buf) - 1);
					ReadFile(read_stdout, buf, s, &bread, NULL);
					output.Write(VString(buf, bread));
				}
			}

			//Проверяем, есть ли данные для чтения в stderr
			PeekNamedPipe(read_stderr, buf, sizeof(buf) - 1, &bread, &avail, NULL);
			
			if(bread || avail){
				rd = 1;
				if(errput.IsFree()){
					int s = minel(errput.IsFree(), sizeof(buf) - 1);
					ReadFile(read_stderr, buf, s, &bread, NULL);
					errput.Write(VString(buf, bread));
				}
			}

			if(!rd && ec != STILL_ACTIVE)
				break;

			if(!iscont)
				return 1;

			if(!rd)
				Sleep(1);
		}

		EndProcess();
		exitcode = ec;

		return 0;
	}

	int EndProcess(){
		if(state == PIPELINE_STATE_NONE)
			return 0;

		FILETIME StartTime, EndTime;
		FILETIME KernelTime, UserTime;
		
		state = PIPELINE_STATE_NONE;

		GetProcessTimes(pi.hProcess, &StartTime, &EndTime, &KernelTime, &UserTime);
		kerneltime = *(int64*)&KernelTime / 10;
		usertime = *(int64*)&UserTime / 10;
		cputime = kerneltime + usertime;

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		CloseHandle(newstdin);            //небольшая уборка за собой
		CloseHandle(newstdout);
		CloseHandle(newstderr);
		CloseHandle(write_stdin);
		CloseHandle(read_stdout);
		CloseHandle(read_stderr);		

		pi.hThread = 0;
		pi.hProcess = 0;
		newstdin = 0;
		newstdout = 0;
		newstderr = 0;
		write_stdin = 0;
		read_stdout = 0;
		read_stderr = 0;

		return 1;
	}

#else

	int RunNewPipe(int *apipe){
		if(pipe(apipe) < 0){
			Sleep(100);
			pid = 0;
		
			while(pipe(apipe) < 0 && pid ++ < 5)
				Sleep(100);
		
			if(pid >= 5)
				return 0;			
		}

		return 1;
	}

	int Run(VString cmd, VString dir = VString(), VString env = VString(), int isinput = 0){
		cputime = 0;
		kerneltime = 0;
		usertime = 0;
		exitcode = 0;

		if(state == PIPELINE_STATE_RDWR || state == PIPELINE_STATE_READ)
			EndProcess();

		pipelock.Lock();

		if(!RunNewPipe(p_stdin)){
			print("Died on pipe(p_stdin, ", itos(errno), ")!\n");
			pipelock.UnLock();
			return 0;
		}

		if(!RunNewPipe(p_stdout)){
			close(p_stdin[0]);
			close(p_stdin[1]);
			print("Died on pipe(p_stdout, ", itos(errno), ")!\n");
			pipelock.UnLock();
			return 0;
		}

		if(!RunNewPipe(p_stderr)){
			close(p_stdin[0]);
			close(p_stdin[1]);
			close(p_stdout[0]);
			close(p_stdout[1]);
			print("Died on pipe(p_stderr, ", itos(errno), ")!\n");
			pipelock.UnLock();
			return 0;
		}

		if(!RunNewPipe(p_stdret)){
			close(p_stdin[0]);
			close(p_stdin[1]);
			close(p_stdout[0]);
			close(p_stdout[1]);
			close(p_stderr[0]);
			close(p_stderr[1]);
			print("Died on pipe(p_stdret, ", itos(errno), ")!\n");
			pipelock.UnLock();
			return 0;
		}

		pipelock.UnLock();

		if((pid = fork()) < 0){
			close(p_stdin[0]);
			close(p_stdin[1]);
			close(p_stdout[0]);
			close(p_stdout[1]);
			close(p_stderr[0]);
			close(p_stderr[1]);
			close(p_stdret[0]);
			close(p_stdret[1]);
			printf("Died on fork()!\n");
			return 0;
		}

		if(pid == 0){
			// Child
			close(p_stdin[1]);
			close(p_stdout[0]);
			close(p_stderr[0]);
			close(p_stdret[0]);

			dup2(p_stdin[0], fileno(stdin));
			dup2(p_stdout[1], fileno(stdout));
			dup2(p_stderr[1], fileno(stderr));

			// Set env
			char *ln = env, *to = ln + env.size();
			int rc = -1;
	
			while(ln < to){
				if(*ln)
					putenv(ln);
				
				while(ln < to && *ln != 0)
						ln++;
				while(ln < to && *ln == 0)
					ln++;	
			}

			rc = system(TString(cmd));

			write(p_stdret[1], &rc, 4);

			struct rusage ru;
			rc = getrusage(RUSAGE_SELF, &ru);
			if(rc){
				memset(&ru.ru_utime, 0, sizeof(struct timeval) * 2);
			}

			write(p_stdret[1], &ru.ru_utime, sizeof(struct timeval) * 2);

			close(p_stdin[0]);
			close(p_stdout[1]);
			close(p_stderr[1]);
			close(p_stdret[1]);

			quick_exit(0); //exit(0);
			return 0;
		}
		
		// Parent
		close(p_stdin[0]);
		close(p_stdout[1]);
		close(p_stderr[1]);
		close(p_stdret[1]);

		if(isinput)
			state = PIPELINE_STATE_RDWR;
		else
			state = PIPELINE_STATE_READ;

		if(iscont)
			Process();

		return 1;
	}

	int Process(){
		if(!state)
			return 0;

		unsigned char buf[S8K];
		fd_set wfds, rfds, efds;
		int aread, bwrite, bread = 1, eread = 1, maxs;
		timeval tm;
		

		while(1){
			FD_ZERO(&wfds);
			FD_ZERO(&rfds);
			FD_ZERO(&efds);			
			FD_SET(p_stdin[1], &wfds);
			FD_SET(p_stdout[0], &rfds);
			FD_SET(p_stderr[0], &rfds);
			FD_SET(p_stdout[0], &efds);
			FD_SET(p_stderr[0], &efds);

			maxs = maxel(p_stdin[1], p_stdout[0]);
			maxs = maxel(maxs, p_stderr[0]);
			maxs ++;

			tm.tv_sec = 0;
			tm.tv_usec = 0;
			aread = 0;

			int sel = select(maxs, &rfds, &wfds, &efds, &tm);

			// Write
			if(IsHandle(p_stdin[1]) && FD_ISSET(p_stdin[1], &wfds)){
				if(input.IsRead()){
					bwrite = input.Read(VString(buf, sizeof(buf)));
					//WriteFile(write_stdin, buf, bread, &bread, NULL);
					write(p_stdin[1], buf, bwrite);
					input.Readed(bwrite);
				}
				else if(!isinput){
					close(p_stdin[1]);
					p_stdin[1] = 0;
				}
			}

			if(FD_ISSET(p_stdout[0], &rfds)){
				aread = 1;
				if(output.IsFree()){
					int s = minel(output.IsFree(), sizeof(buf) - 1);
					//ReadFile(read_stdout, buf, s, &bread, NULL);
					bread = read(p_stdout[0], buf, s);
					if(bread > 0)
						output.Write(VString(buf, bread));
				}
			}

			if(FD_ISSET(p_stderr[0], &rfds)){
				aread = 1;
				if(errput.IsFree()){
					int s = minel(errput.IsFree(), sizeof(buf) - 1);
					//ReadFile(read_stdout, buf, s, &bread, NULL);
					eread = read(p_stderr[0], buf, s);
					if(eread > 0)
						errput.Write(VString(buf, eread));
				}
			}

		if(FD_ISSET(p_stdout[0], &efds) || FD_ISSET(p_stderr[0], &efds) || ( bread <= 0 && eread <= 0))
			break;

		if(sel < 0)
			break;

		if(!iscont)
			return 1;

		if(!aread)
			Sleep(1);
		}

		EndProcess();
		return 0;
	}

	int EndProcess(){
		if(state == PIPELINE_STATE_NONE)
			return 0;

		state = PIPELINE_STATE_NONE;

		struct timeval ctm[2];

		int r = read(p_stdret[0], &exitcode, 4);
		r = read(p_stdret[0], &ctm, sizeof(struct timeval) * 2);

		//if(r == sizeof(struct timeval) * 2){  }
		usertime = int64(ctm[0].tv_sec) * 1000000 + ctm[0].tv_usec * 1000;
		kerneltime = int64(ctm[1].tv_sec) * 1000000 + ctm[1].tv_usec * 1000;
		cputime = kerneltime + usertime;

		r = waitpid(pid, 0, 0);
		
		if(p_stdin[1])
			close(p_stdin[1]);
		close(p_stdout[0]);
		close(p_stderr[0]);
		close(p_stdret[0]);

		return 1;
	}

#endif


#ifdef WIN32
	bool IsWinNT(){  //проверка запуска под NT
		OSVERSIONINFO osv;
		osv.dwOSVersionInfoSize = sizeof(osv);
		GetVersionEx(&osv);
		return (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	void ErrorMessage(char *str){  //вывод подробной информации об ошибке
		LPVOID msg;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // язык по умолчанию
			(LPTSTR) &msg,
			0,
			NULL
		);

		printf("%s: %s\n", str, msg);
		LocalFree(msg);
	}

#else


#endif

	void Clean(){
		EndProcess();
	}

	~PipeLine2(){
		EndProcess();
	}



};