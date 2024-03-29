#ifdef WIN32
#ifndef WINCE
HANDLE _std_input_handle = GetStdHandle(STD_INPUT_HANDLE);
HANDLE _std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

void MsvCoreAllocConsole(){
	AllocConsole();
	_std_input_handle = GetStdHandle(STD_INPUT_HANDLE);
	_std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
}
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////// Print
#ifdef USEMSV_TCPPRINT
	int ConnectPort(VString lip, int port);
	MString msvuse_tcpprint; SOCKET msvuse_tcpprint_sock=0;
#endif

#ifdef USEMSV_FILEPRINT
	MString msvuse_fileprint="msvuse_fileprint.log"; HFILE msvuse_fileprint_sock=0;
#endif

#ifdef USEMSV_LOGPRINT
	SendDataL msvuse_logprint;
	int msvuse_logprint_getsz(){ return msvuse_logprint.size(); }
	int msvuse_logprint_get(unsigned char * data, unsigned int sz){ int rd=msvuse_logprint.Get(data, sz); msvuse_logprint.Del(rd); return rd; }
#endif

int print(const VString &line){ if(!line) return 0;

#ifdef USEMSV_TCPPRINT
	if(msvuse_tcpprint && msvuse_tcpprint_sock==0){
		msvuse_tcpprint_sock=ConnectPort(msvuse_tcpprint, 0);
		if(!msvuse_tcpprint_sock) msvuse_tcpprint_sock=-1;
	}
	if(msvuse_tcpprint_sock>0) send(msvuse_tcpprint_sock, line, line, 0);
#endif

#ifdef USEMSV_FILEPRINT
		if(msvuse_fileprint && msvuse_fileprint_sock==0){
		msvuse_fileprint_sock=CreateFile(msvuse_fileprint, O_CREAT|O_RDWR, S_IREAD|S_IWRITE);
		if(ishandle(msvuse_fileprint_sock)) SetEndOfFile(msvuse_fileprint_sock);
		if(!msvuse_fileprint_sock) msvuse_fileprint_sock=-1;
	}
	if(msvuse_fileprint_sock>0) WriteFile(msvuse_fileprint_sock, line, line);
#endif

#ifdef USEMSV_LOGPRINT
	msvuse_logprint.Set(line); if(msvuse_logprint.size()>S1M) msvuse_logprint.Del(msvuse_logprint.size()%S1M);
#endif

#ifdef USEMSV_ANDROID_LOGPRINT
	__android_log_print(ANDROID_LOG_INFO, USEMSV_ANDROID_LOGPRINT, MString(line));
#endif


#ifdef WIN32
    DWORD wr;
    return WriteFile(_std_output_handle, line, line, &wr, 0); //printf(line);
#else
    return write(1, line, line);
#endif
}

// Multi Print
int print(const VString &l1, const VString &l2){
    int r=print(l1); r=print(l2) && r; return r;
}
int print(const VString &l1, const VString &l2, const VString &l3){
    int r=print(l1); r=print(l2) && r; r=print(l3) && r; return r;
}
int print(const VString &l1, const VString &l2, const VString &l3, const VString &l4){
	int r=print(l1); r=print(l2) && r; r=print(l3) && r; r=print(l4) && r; return r;
}
int print(const VString &l1, const VString &l2, const VString &l3, const VString &l4, const VString &l5
, const VString &l6, const VString &l7, const VString &l8, const VString &l9, const VString &l10){
	int r=print(l1); r=print(l2) && r; r=print(l3) && r; r=print(l4) && r; r=print(l5) && r;
	r=print(l6) && r; r=print(l7) && r; r=print(l8) && r; r=print(l9) && r; r=print(l10) && r; return r;
}

// Global Error
int globalerror(){
	print("\r\nGLOBALERROR\r\n");
	return 0;
}

int globalerror(const char *line){
	print("\r\nGLOBALERROR: ");
	print(line);
	print("\r\n");
	return 0;
}


// Normal path
int normalpath(char*file, int asz, char *in, int ins, bool r, int opt){ // opt =1 then save "/../"
	//print(itos(asz), " ", itos(ins), " '", VString(in, ins), "'", "\r\n");
	if(!asz) return 0;

	char*ln=file, *to=in+ins, *tof=file+asz; char o='\\', t='/'; if(r){ o='/'; t='\\'; }
	for(in; in<to; in++){// print("'", itos(*in), "'");
		if(*in==0){
			//print("normalpath!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			continue;
		}
		if(*in==o){
			*ln=t; ln++;
		}
		else if(*in=='.' && ln-1>=file && *(ln-1)=='.' && ((!opt) && ln-1==file || ln-2>=file && *(ln-2)==t)
			&& (!opt || !(ln-4>=file && *(ln-3)=='.' && *(ln-4)=='.' && (ln-4==file || *(ln-5)==t)))){ // << ../../
			//char *lln=ln;
			ln-=2; while(ln>file){ if(*ln!='/' && *ln!='\\'){ break; } ln--; }
			if(ln>file) {ln--; while(ln>file){ if(*ln==t) break; ln--; }}
			
			//if(opt&1 && lln){ ln==lln; if(ln>=file) *ln=*in; ln++; }
		}
		else {
			if(ln>=file) *ln=*in; ln++;
		} // if(ln==file || *(ln-1)!='/' )

		if(*(ln-1)==t && ln-1>file && *(ln-2)==t){ ln--; }
		if(ln>=tof){ *file=0; return 0; }
	}
	if(ln<tof) *ln=0;
	return (int)(ln-file);
}

// NormalPath
VString NormalHttpPath(VString path){
unsigned char*line=(unsigned char*)path.uchar(), *ln=line, *to=line+path.size(), *ret=ln; unsigned int r;
int c=0;//, m=0;
while(line<to){
	switch(*line){
		case '/': case '\\': // 1
			if(line>ln){ memcpy(ret, ln, line-ln); ret+=line-ln; ln=line; }
			if(ret>path.uchar() && ( *(ret-1)=='\\' || *(ret-1)=='/')){ ln=line+1; }
			else{ *ret++=*line; ln=line+1; c|=1; }

			//if(c&1) {ln=line; m=1;}
			//else {
			//if(line+1<to && line>ln && *line=='/' && *(line+1)=='/' && *(line-1)==':')  {line++;}
			//else {
			//if(m && line+1>ln) memcpy(ret, ln, line-ln); ret+=line-ln; ln=line; c|=1;}
			//}
		break;
		case '.': // 2
			if(ret==path.uchar() || ( *(ret-1)=='\\' || *(ret-1)=='/')){ r=0; c=0;
				if(line>ln){ memcpy(ret, ln, line-ln); ret+=line-ln; ln=line; }
				while(line < to && *line=='.'){ line++; r++; }
					if(*line=='\\' || *line=='/' || line>=to){
						if(r==1) ln=line+1;
						else{
							if(ret-1>path.uchar()) ret--; //while(ret>path.uchar() && (*(ret-1)!='\\' || *ret!='/')) ret--; ret--;
							while(ret>path.uchar() && *(ret-1)!='\\' && *(ret-1)!='/') ret--;
							ln=line+1;
						}
				}
			}
			//if(c>=3) {ln=line; m=1;}
			//else if(c)
			//else if(c==2) {c=3;}
			 //{if(m && line>ln) memcpy(ret, ln, line-ln); ret+=line-ln; ln=line; c|=2;}
		break;
		case '?':
			if(line > ln)
				memcpy(ret, ln, line - ln);
			ret += line - ln;
			ln = line;

			while(ret && *(ret - 1) == 32 && ret > path.uchar())
				ret --;

			if(line > ln + 1){
				memcpy(ret, ln, line - ln);
				ret += line - ln;
			}
			
			line = to - 1;
		break;
		case '%':
			 if(line>ln) memcpy(ret, ln, line-ln); ret+=line-ln; 
			line++; if(*line>='0' && *line<='9') *ret=(*line-'0')*16; else if(*line>='A' && *line<='F') *ret=(*line-'A'+10)*16; else *ln=0;
			line++; if(*line>='0' && *line<='9') *ret+=(*line-'0'); else if(*line>='A' && *line<='F') *ret+=(*line-'A'+10);
		if(*ret=='.') { if(c>=3) ret--; c|=2;} else if(*ret=='\\' || *ret=='/') { if(c&1) ret--; c|=1;} else c=0;
	// is utf 
		if(*ret>127 && ret>path.uchar()) if(((*(ret)&192)==128) && (*(ret-1)&224)==192) 
		{r=((*(ret-1)&31)<<6)+(*(ret)&63); *(ret-1)=stoc(r); ret--;}
		if(*ret<32) *ret=' '; ln=line+1; ret++;// m=1;
		break;
		default:
			if(*line<32)
				*line=' ';
			c=0;
		break;
} line++; }

	if(line > ln)
		memcpy(ret, ln, line - ln);
	ret += line - ln;

	while(ret && *(ret - 1) == 32 && ret > path.uchar())
		ret --;

	if(path.size() == ret - path.uchar())
		return path; //if(!m) return path;

	return path.str(0, ret - path.uchar());
}



// File I/O
#ifdef WIN32
	inline int fileeof(HFILE hfile){ return chsize64(hfile, tell64(hfile)); }
	int _open(char* file, int op, int pm){
		int fl = 0;
#ifndef __GNUC__
		if(!_sopen_s(&fl, file, op, 0x40, pm)) // _SH_DENYNO
			return fl;
#else
	return open(file, op, pm);
#endif
		//return fl = _open(
		return -1;
	}
#else
	inline int fileeof(HFILE hfile){ return ftruncate(hfile, lseek64(hfile, 0, SEEK_CUR)); }
#endif 


// File functions
HFILE CreateFile(VString file, int op, int pm){
	if(!file)
		return -1;

	MSVCORE_NORMALPATH(buf, file);

	return _open(file, op | O_BINARY, pm);
}

int ReadFile(HFILE fl, void* buf, unsigned int sz){
	return _read(fl, buf, sz);
}

int WriteFile(HFILE fl, const void* buf, unsigned int sz){
	return _write(fl, buf, sz);
}

int64 GetFilePointer(HFILE fl){
	return lseek64(fl, 0, FILE_CURRENT);
}

int64 SetFilePointer(HFILE fl, int64 pos, int origin){
	return lseek64(fl, pos, origin);
}

int SetEndOfFile(HFILE hfile){
	return fileeof(hfile);
}

sstat64 GetFileInfo(VString file){
	sstat64 stt;

	while(file.endo() == '/')
		file.sz --;

	MSVCORE_NORMALPATH(buf, file);

	if(!file || _stati64(file, &stt))
		memset(&stt, 0, sizeof(stt));

	return stt;
}

sstat64 GetFileInfo(HFILE hfile){
	sstat64 stt;

	if(_fstati64(hfile, &stt))
		memset(&stt, 0, sizeof(stt));

	return stt;
}

int CloseHandle(HFILE fl){
	if(!ishandle(fl))
		return 0;
	return _close(fl);
}

//
int64 GetFileSize(HFILE hfile){
	int64 pos = GetFilePointer(hfile);
	int64 sz = SetFilePointer(hfile, 0, FILE_END);
	SetFilePointer(hfile, pos, FILE_BEGIN);
	return sz;
}

int64 GetFileSize(VString file){
	sstat64 stt = GetFileInfo(file);
	return stt.st_size;
}

// Dir functions
bool IsDir(VString path){
	MSVCORE_NORMALPATH(buf, path);

	sstat64 stt = GetFileInfo(path);

	return (stt.st_mode & S_IFDIR) != 0;
}

int MkDir(VString path, unsigned int mode){
	MSVCORE_NORMALPATH(buf, path);

	return !stdmkdir(path, mode);
}

// File operations
bool IsFile(VString file){
	HFILE fl = CreateFile(file, O_RDONLY, S_IREAD | S_IWRITE);
	if(!ishandle(fl))
		return 0;

	CloseHandle(fl);
	return 1;
}

MString LoadFile(VString file){
	MString ret;
	unsigned int rd;

	if(!file)
		return MString();

	HFILE fl = CreateFile(file, O_RDONLY, S_IREAD | S_IWRITE);

	if(fl == -1)
		return MString();

	ret.Reserve((unsigned int)GetFileSize(fl));

	rd = ReadFile(fl, ret, ret);

	CloseHandle(fl);

	if(rd != ret.size()) 
		return MString();

	return ret;
}

unsigned int SaveFile(VString file, VString data){
	if(!file)
		return 0;

	HFILE fl = CreateFile(file, O_RDWR | O_CREAT, S_IREAD | S_IWRITE); //  | S_IRGRP | S_IROTH
	if(!ishandle(fl))
		return 0;

	unsigned int wr = WriteFile(fl, data, data);
	SetEndOfFile(fl);
	CloseHandle(fl);

	return wr;
}

unsigned int SaveFileAppend(VString file, VString data){
	HFILE fl = CreateFile(file, O_RDWR | O_CREAT, S_IREAD | S_IWRITE); //  | S_IRGRP | S_IROTH
	if(!ishandle(fl))
		return 0;

	SetFilePointer(fl, 0, FILE_END);

	unsigned int wr = WriteFile(fl, data, data);

	CloseHandle(fl);

	return wr;
}

int CopyFile(VString from, VString to){
	MSVCORE_NORMALPATH(fbuf, from);
	MSVCORE_NORMALPATH(tbuf, to);

	sstat64 stt = GetFileInfo(fbuf);
	if((stt.st_mode & S_IFDIR) != 0)
		return 0;

	sstat64 stt2 = GetFileInfo(tbuf);
	if(stt.st_size == stt2.st_size && stt.st_ctime == stt2.st_ctime && stt.st_mtime == stt2.st_mtime)
		return 1;

	HFILE fl = CreateFile(fbuf, O_RDONLY, S_IREAD | S_IWRITE);
	if(!ishandle(fl))
		return 0;

	HFILE fl2 = CreateFile(tbuf, O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
	if(!ishandle(fl2)){
		CloseHandle(fl);
		return 0;
	}

	// Set pointer
	SetFilePointer(fl2, 0);

	// 
	unsigned char buf[S4K];
	int ret = 1;

	int64 sz = stt.st_size;

	while(sz > 0){
		int rd = ReadFile(fl, buf, sz > S4K ? S4K : sz & 0xffff); // minel(sz, S4K)
		int wr = WriteFile(fl2, buf, rd);

		if(sz && rd != wr){
			ret = 0;
			break;
		}

		sz -= rd;
	}

	if(ret)
		SetFileTime(fl2, stt.st_ctime, stt.st_atime, stt.st_mtime);

	// Set EOF
	SetEndOfFile(fl2);

	// Close
	CloseHandle(fl);
	CloseHandle(fl2);

	return ret;
}

int MoveFile(VString from, VString to){
	MSVCORE_NORMALPATH(fbuf, from);
	MSVCORE_NORMALPATH(tbuf, to);

	return !_rename(from, to);
}

int DeleteFile(VString file){
	MSVCORE_NORMALPATH(buf, file);

	return _unlink(file);
}


#ifdef WIN32
FILETIME UnixTimeToWin(time_t time){
	SYSTEMTIME SystemTime;
	FILETIME LocalFileTime;
	FILETIME ResultFileTime;

	if(!time){
		ResultFileTime.dwHighDateTime = 0;
		ResultFileTime.dwLowDateTime = 0;
		return ResultFileTime;
	}

	struct tm tmb;
#ifndef __GNUC__
	_localtime64_s(&tmb, &time);
#else
	struct tm *tmp = localtime(&time);
	if(tmp)
		tmb = *tmp;
#endif

	SystemTime.wYear   = (WORD)(tmb.tm_year + 1900);
	SystemTime.wMonth  = (WORD)(tmb.tm_mon + 1);
	SystemTime.wDay    = (WORD)(tmb.tm_mday);
	SystemTime.wHour   = (WORD)(tmb.tm_hour);
	SystemTime.wMinute = (WORD)(tmb.tm_min);
	SystemTime.wSecond = (WORD)(tmb.tm_sec);
	SystemTime.wMilliseconds = 0;

	SystemTimeToFileTime(&SystemTime, &LocalFileTime);
	LocalFileTimeToFileTime(&LocalFileTime, &ResultFileTime);

	return ResultFileTime;
}

bool SetFileTime(HFILE fl, time_t ctime, time_t atime, time_t mtime){
	FILETIME LocalFileTime = UnixTimeToWin(ctime);
    FILETIME LastWriteTime = UnixTimeToWin(mtime);
    FILETIME LastAccessTime = UnixTimeToWin(atime);

	return SetFileTime((HANDLE)_get_osfhandle(fl), ctime ? &LocalFileTime : 0, atime ? &LastAccessTime : 0, mtime ? &LastWriteTime : 0) != 0;
}

#else // Linux
	bool SetFileTime(HFILE fl, time_t ctime, time_t atime, time_t mtime){
		//struct utimbuf tm;
		//tm.actime = atime;
		//tm.modtime = mtime;

		struct timeval tv[2];
		tv[0].tv_sec = atime;
		tv[1].tv_sec = mtime;

		tv[0].tv_usec = 0;
		tv[1].tv_usec = 0;

		return futimes(fl, tv);
	}
#endif


class AHFILE{
	HFILE hfile;

public:

	AHFILE(HFILE h){
		hfile=h;
	}

	operator HFILE(){
		return hfile;
	}

	~AHFILE(){
		if(ishandle(hfile))
			CloseHandle(hfile);
	}

};


bool MakeDir(VString path, VString fold){
#ifdef  __GNUC__
#define MAKEDIRSLASH    "/"
#else
#define MAKEDIRSLASH    "\\"
#endif
	MString rfold=Replace(fold, "\\", "/"); fold=rfold;
	VString o, t=fold, lo; MString tmp;
	while(t){
		o=PartLine(t, t, "/");
		o.sz=o.endu()-fold.data; o.data=fold;// if(lo==o) break;
		tmp.Add(path, path ? "/" : "",  o);
		MkDir(tmp);
		lo=o;
	}
	return 1;
}


#define READDIRSZ	64

#include "sort.cpp"
#include "ef.cpp"

/*
class Readdir{
	LString ls;
	IList<VSi> list;
	//VSiLine line;

#ifdef WIN32
	HANDLE handle;
#else
	DIR *handle;
#endif


public:
	*/

	Readdir::Readdir(){
		handle = 0;
	}

	int Readdir::Add(VString file, sstat64 &stt){
		//if(line.size==line.asize()) line.Add(READDIRSZ);
		//VString fn=file;
		list.A();
		list.n().key = ls.addnf(file);// .set(&ls.el(ls.addnf(file, file)), file);
		list.n().stt = stt;
	
		list.Added();
		//line.size++;
		return list.Size();
	}

	int Readdir::OpenDir(VString &path){
		VString file;
		char buf[S4K];

		if(path.size() > S4K - 4)
			 return 0;

		// Clean
		Clean();

		// Normal path
		path.set(buf, normalpath(buf, S4K, path, path.size()));

		// Win32
		if(_TSYS == TSYS_WIN){
			path.data[path.sz++] = '*';
			path.data[path.sz++] = '.';
			path.data[path.sz++] = '*';
		}
		path.data[path.sz]=0;

		// Find
	#ifdef WIN32
		WIN32_FIND_DATA ff;
		handle = FindFirstFile(MODUNICODE(path), &ff);
		if(!ishandle(handle)){
			handle = 0;
			return 0;
		}
	#else
		handle = opendir(path ? path : "./");
		if(!handle)
			return 0;
	#endif


#ifdef WIN32
	#ifdef UNICODE // #unicode
		MString fnuni = MODLPWSTR_R(ff.cFileName);
		file = fnuni;
	#else
		file = MODLPWSTR(ff.cFileName); 
	#endif // #e unicode

		sstat64 stt;
		memset(&stt, 0, sizeof(stt));
		Add(file, stt);
#endif

		return 1;
	}

	VString Readdir::ReadOne(SString &ss){
		if(!handle)
			return "";

		if(list.Size()){
			ss.set(list[0].key);
			list.Del(0);
			return ss;
		}

		VString file;

		while(1){
#ifdef WIN32
			WIN32_FIND_DATA ff;
			if(!FindNextFile(handle, &ff)) 
				return "";
	#ifdef UNICODE // #unicode
			MString fnuni = MODLPWSTR_R(ff.cFileName);
			file = fnuni;
	#else
			file = MODLPWSTR(ff.cFileName); 
	#endif // #e unicode
			return ss.set(file);
#else
			struct dirent *dr = readdir(handle);
			if(!dr)
				return "";
			file = dr->d_name;
			return ss.set(file);
#endif
		}

		return "";
	}

int Readdir::ReadDir(MString dir){
	ls.Clean(); list.Clear(); sstat64 stt; /*lsstat64 st;*/ VString path, fn; int fs=0; unsigned int pos;
char bf[S2K]; if(dir.size()>S2K-3) return 0; //memcpy(bf, dir, dir);
MSVEF ef; ef.fcmp=1; // ILink il; il.Ilink(dir); if(!il.file){ dir+"*"; il.file="*";}
//if(_TSYS==TSYS_WIN){ wcslash(dir); dir+="*.*"; }
// normal path for find
path.set(bf, normalpath(bf, S2K, dir, dir.size()));
while(path.sz>0){ if(*(path.end()-1)=='/' || *(path.end()-1)=='\\') break; path.sz--; fs++; }
ef.Read(VString(path.pos(path.sz), fs));
if(_TSYS==TSYS_WIN){ path.data[path.sz++]='*'; path.data[path.sz++]='.'; path.data[path.sz++]='*'; } path.data[path.sz]=0;

#ifdef WIN32  //int ReadDir(MString dir, ISLine &list){ 
	HANDLE find_handle; WIN32_FIND_DATA ff; char* d=(char*)&bf;
	find_handle=FindFirstFile(MODUNICODE(path),&ff);
	if(find_handle==INVALID_HANDLE_VALUE) return 0;
#else
	DIR *d=opendir(path ? MString(path) : "./"); if(!d) return 0;
#endif
	while(path.sz>0){ if(*(path.end()-1)=='/' || *(path.end()-1)=='\\') break; path.sz--; }

	while(d){
#ifdef WIN32
	if(fn.data) if(!FindNextFile(find_handle, &ff)) break;
#ifdef UNICODE // #unicode
	MString fnuni=MODLPWSTR_R(ff.cFileName); fn=fnuni;
#else
	fn=MODLPWSTR(ff.cFileName); 
#endif // #e unicode

#else
		struct dirent *dr=readdir(d); if(!dr) break; fn=dr->d_name;
#endif

if(fn!="."){
	if(path.sz+fn.sz+1>=S1K) break;
	if(fs){ pos=0; if(!ef.rts(fn, pos)) continue; }
	memcpy(path.end(), fn, fn); *(path.end()+fn.sz)=0;
	if(lstat64(path, &stt)) break;
	//else{ stat64tomstat(stt, st); }

	Add(fn, stt);
	//if(line.size==line.asize()) line.Add(READDIRSZ);
		
	//line.n().key.set(&ls.el(ls.addnf(fn, fn)), fn); line.n().stt=stt;
		//line.n().is=(ff.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)>0;
		//line.n().size=ff.nFileSizeLow;
	//line.size++;
}	//list.insert(ff.cFileName, ();
	}
#ifdef WIN32
	FindClose(find_handle);
#else
	closedir(d);
#endif
	return 1;}


/*

#define RD_NOP		S1M
#define RDS_NAME	1
#define RRDS_UNAME	2

int Resort(int type = RD_NOP | RDS_NAME){
	//HLString tls; VSiLine tline; tline.Add(line.size); memcpy(tline.data, line.data, line.size*sizeof(VSi));
	if(type & RD_NOP){
		for(unsigned int i=0; i<list.Size(); i++){
			if(list[i].key == ".")
				list.Del(i);
				//memcpy(list.Pos(i), &line.data[i+1], (line.size-i)*sizeof(VSi));
				//line.size--;
		}
	}

	myfastsort_VSiLine(list);

	// What???
	//for(unsigned int i=0; i<list.Size(); i++){
	//		if(list[i].key==".."){
	//			VSi vs = list[i];
	//			memcpy(&line.data[1], &line.data[0], (i)*sizeof(VSi)); // line.size--;
	//			line.data[0]=vs;
	//		}
	//}
return 1;
}


	void operator = (Readdir &rd){
		for(unsigned int i = 0; i < rd.size(); i++){
			Add(rd[i].key, rd[i].stt);
		}
		return ;
	}

	VSi& operator[](int p){
		return list[p];
	}

	operator unsigned int(){
		return list.Size();
	}

	unsigned int size(){
		return list.Size();
	}
	*/

	void Readdir::Clean(){
		ls.Clean();
		list.Clear();

		if(handle){
#ifdef WIN32
			FindClose(handle);
#else
			closedir(handle);
#endif
			handle = 0;
		}
	}

	Readdir::~Readdir(){
		Clean();
	}

/*
};
*/

#ifdef USEMSV_MEMORYCONTROL

void msvcore_memcon_print_count(LStringX<S32K> &ls, int count_size, int mem_size, int max_size, VString name, MsvCoreMemoryControlCount &count){
	const char s[] = "                                                                                      ";
	const char p[] = "......................................................................................";

	int64 ucount = count.acount - count.fcount;

	ls + name + VString(p, max_size - name.size() + 1)
		+ VString(p, count_size - itossz(count.acount)) + count.acount
		+ VString(p, count_size - itossz(ucount)) + ucount
		+ VString(p, count_size - itossz(count.fcount)) + count.fcount
		+ "\r\n";

#ifdef USEMSV_MEMORYCONTROL_INFO
		ucount = count.amemory - count.fmemory;

		ls + VString(s, max_size + 1 - 3 - 3 *(mem_size - count_size)) + "mem"
			+ VString(s, mem_size - itossz(count.amemory)) + count.amemory
			+ VString(s, mem_size - itossz(ucount)) + ucount
			+ VString(s, mem_size - itossz(count.fmemory)) + count.fmemory
			+ "\r\n";
#endif

	return ;
}

void msvcore_memcon_print(const char* tofile){
	MemConLock();

	VString s("                                                                                      ");
	VString p("......................................................................................");

	LStringX<S32K> ls;
	int count_size = 10;
	int mem_size = 13;
	int max_size = 0;
	int name_size;

	HFILE fl = 0;
	
	if(tofile){
		fl = CreateFile(tofile, O_RDWR | O_CREAT, S_IREAD | S_IWRITE); //  | S_IRGRP | S_IROTH
		if(!ishandle(fl)){
			MemConUnLock();
			return ;
		}

		SetFilePointer(fl, 0, FILE_END);
	}


	ls + "\r\n" "Memory Control: ";

	if(tofile){
		MTime mt;
		ls + mt.date("d.m.y H:i:s");
	}

	ls + "\r\n";

	for(unsigned int i = 0; i < MsvCoreMemoryControl.GetTypeSz(); i++)
		if(max_size < (name_size = strlen(MsvCoreMemoryControl.GetType(i).name)))
			max_size = name_size;

	if(max_size < 15)
		max_size = 15;

	if(max_size > 48)
		max_size = 48;
	else{
		count_size = minel(mem_size, ((79 - max_size) / 3));		
	}

	// Info
	name_size = 5;
	ls + "#Name" + (name_size <= max_size ? s.str(0, max_size - name_size + 1) : VString());
	ls + s.str(0, count_size - 5) + "Alloc"
		+ s.str(0, count_size - 3) + "Use"
		+ s.str(0, count_size - 4) + "Free"
		+ "\r\n";

	// Types
	for(unsigned int i = 0; i < MsvCoreMemoryControl.GetTypeSz(); i++){
		MsvCoreMemoryControlType &el = MsvCoreMemoryControl.GetType(i);
		msvcore_memcon_print_count(ls, count_size, mem_size, max_size, el.name, el);
//		
//		name_size = strlen(el.name);
//		ucount = el.acount - el.fcount;
//
//		ls + el.name + (name_size <= max_size ? p.str(0, max_size - name_size + 1) : VString());
//		ls + p.str(0, count_size - itossz(el.acount)) + el.acount
//			+ p.str(0, count_size - itossz(ucount)) + ucount
//			+ p.str(0, count_size - itossz(el.fcount)) + el.fcount
//			+ "\r\n";
//		umemory = el.amemory - el.fmemory;
//		
//#ifdef USEMSV_MEMORYCONTROL_INFO
//		ls + s.str(0, max_size + 1 - 3 - 3 *(mem_size - count_size)) + "mem"
//			+ s.str(0, mem_size - itossz(el.amemory)) + el.amemory
//			+ s.str(0, mem_size - itossz(umemory)) + umemory
//			+ s.str(0, mem_size - itossz(el.fmemory)) + el.fmemory
//			+ "\r\n";
//#endif

		// Output
		if(ls.Size() >= S32K - S4K){
			if(!tofile)
				print((VString) ls);
			else
				WriteFile(fl, ls.OneLine(), ls.Size());

			ls.Clean();
		}
	}

	msvcore_memcon_print_count(ls, count_size, mem_size, max_size, "All ", MsvCoreMemoryControl);
	msvcore_memcon_print_count(ls, count_size, mem_size, max_size, "MemCon use ", msvcore_memcon_use);

	//ls + "Malloc: " + msvcore_memcon_malloc_count + ", use: " + (msvcore_memcon_malloc_count - msvcore_memcon_free_count) + ", free: " + msvcore_memcon_free_count + "\r\n";

	if(!tofile)
		print((VString) ls);
	else
		WriteFile(fl, ls.OneLine(), ls.Size());

	MemConUnLock();

	if(tofile)
		CloseHandle(fl);

	return ;
}

void msvcore_memcon_print_stack(const char* tofile){
	MemConLock();

	VString s("                                                                                      ");
	LStringX<S32K> ls;
	MTime mt;
	int m = 0;

	HFILE fl = 0;
	
	if(tofile){
		fl = CreateFile(tofile, O_RDWR | O_CREAT, S_IREAD | S_IWRITE); //  | S_IRGRP | S_IROTH
		if(!ishandle(fl)){
			MemConUnLock();
			return ;
		}

		SetFilePointer(fl, 0, FILE_END);
	}

	ls + "\r\n" "------------- Memory Control StackTrace: ";
	ls + mt.date("d.m.y H:i:s");
	ls + " ------------------------------------- \r\n";

	for(int i = 0; i < MsvCoreMemoryControl.GetStackSz(); i++){
		MsvCoreMemoryControlStack *sel = MsvCoreMemoryControl.GetStackPos(i);
		ls + "AUF: " + sel->acount + "/" + (sel->acount - sel->fcount) + "/" + sel->fcount
			+ ". MEM: " + sel->amemory + "/" + (sel->amemory - sel->fmemory) + "/" + sel->fmemory
			+ ". CRC: " + sel->sid
			+ "\r\n" + VString(sel->stacktrace) +  "\r\n\r\n";


		// Output
		if(ls.Size() >= S32K - S4K){
			if(!tofile)
				print((VString) ls);
			else
				WriteFile(fl, ls.OneLine(), ls.Size());

			ls.Clean();
		}
	}

	// Output
	if(!tofile)
		print((VString) ls);
	else
		WriteFile(fl, ls.OneLine(), ls.Size());

	MemConUnLock();

	if(tofile)
		CloseHandle(fl);
	return ;
}
#endif