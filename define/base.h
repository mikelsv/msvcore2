// Size
#define S1K 1024
#define S2K 2048
#define S4K 4096
#define S8K 8192
#define S16K 16384
#define S32K 32768
#define S64K 65536
#define S64KM 65535
#define S128K 131072
#define S256K 262144
#define S512K 524288
#define S1M 1048576
#define S2M 2097152
#define S4M 4194304
#define S8M 8388608
#define S16M 16777216
#define S32M 33554432
#define S64M 67108864
#define S128M 134217728
#define S256M 268435456
#define S512M 536870912
#define S1G 1073741824
#define S2G 2147483648u
#define S2GM (S2G-1)
#define S3G (2147483648u+1073741824)
#define S4G 4294967296ull
#define S4GM 4294967295ull
#define S8G (4294967296ull*2)
#define S16G (4294967296ull*4)
#define S32G (4294967296ull*8)
//#define S32G (4294967296ull*8)

// This System
#define TSYS_WIN	1
#define TSYS_LIN	2
#define TSYS_ANDROID	3

// This system Codepage
#define TSYSC_WIN	1
#define TSYSC_DOS	2
#define TSYSC_UTF	3

extern int _TSYS;
// ~ This System

// WINCE & UNICODE
#ifdef WINCE
#include "crossplatform/wince-twoms.cpp"
#include "crossplatform/wince-two.h" // find it in my Center library
#else
#ifdef UNICODE
//#include "../crossplatform/wince-twoms.cpp"
#endif
#endif

// Unicode Problem
#ifdef WINCE
#else
#ifdef WIN32
#ifndef UNICODE
#define MODUNICODE(x) (x) //<< normal
//#define MODUNICODEV(x) VString(x)
#define MODUNICODEV(x, s) (LPCSTR)x, s
//(LPCSTR)x
#define MODLPWSTR	LPCSTR
#define MODLPWSTR_R(x) x
#define MODUNICODET(x) x
#else
#define MODLPWSTR_R(x) stoc((unsigned short*)x, wcslen((wchar_t*)x))
#endif
#else
//#define MODUNICODE(x) LPCSTR(x) //<< normal
#define MODUNICODE(x) LPCSTR(x)
//#define MODUNICODEV(x) VString(x)
#define MODUNICODEV(x, s) (LPCSTR)x, s
//(LPCSTR)x
#define MODLPWSTR	LPCSTR
#define MODUNICODET(x) x
#endif
#endif


// bits operation
#define divur(a, b) (a/b+(a%b)>0)
#define reval(a, b) {a^=b; b^=a; a^=b;}
#define minel(a, b) ((a<=b) ? a : b)
#define maxel(a, b) ((a>=b) ? a : b)

// Num & char
#define isnumd(val) ((unsigned char)val < 48 || (unsigned char)val > 57 ? 0 : 1)
#define isnumdh(val) (((unsigned char)val < 48 || (unsigned char)val > 57) && ((unsigned char)val < 'a' || (unsigned char)val > 'z' ) ? 0 : 1)
inline unsigned char lowd(const unsigned char ch){ return ((ch>=65 && ch<=90 || ch>=192 && ch<=223) ? ch+32 : ch); }
inline unsigned char upd(const unsigned char ch){ return ((ch>=97 && ch<=122 ||  ch>=224) ? ch-32 : ch); }

// handle
#define IsHandle(h) ((h) && ((int)h)!=-1)
#define ishandle(h) ((h) && ((int)h)!=-1)

// Test time: tbtime(); tctime(); tetime(); int sec=sec, millim=micro sec.
#define tbtime timeb ft, fte; int tbtime_sec, tbtime_millim, tbtime_result; ftime(&ft); // print("Time: ", itos(sec*1000+millitm), "ms.\r\n");
#define tctime ftime(&ft); // continue
#define tetime ftime(&fte); tbtime_sec=(int)(fte.time-ft.time); tbtime_millim=fte.millitm-ft.millitm; if(fte.millitm<ft.millitm){ tbtime_millim+=1000; tbtime_sec--; } tbtime_result = tbtime_sec * 1000 + tbtime_millim;


// OMatrixTemplates //

// Add to end (_p, _n);
#define OMatrixTemplateAdd(_a, _e, el)				\
	if(!_a){ _a=el; _e=el; el->_p=0; el->_n=0; }		\
	else {	el->_n=0; el->_p=_e; _e->_n=el; _e=el; }

// Add to end (only _n);
#define OMatrixTemplateAddO(_a, _e, el)				\
	if(!_a){ _a=el; _e=el; el->_n=0; }				\
	else {	el->_n=0; _e->_n=el; _e=el; }

// Add to end (parent: _a, element: _p, n, to: _p, el);
#define OMatrixTemplateAdd_A_PN_PEL(_a, prev, el)		\
	if(!_a){ _a=el; el->_p=0; el->_n=0; }				\
	else {	el->_n=0; el->_p=prev; prev->_n=el; }

// Add to end (Set _p, _n name);
#define OMatrixTemplateAddF(_a, _e, el, _p, _n)			\
	if(!_a){ _a=el; _e=el; el->_p=0; el->_n=0; }			\
	else {	el->_n=0; el->_p=_e; _e->_n=el; _e=el; }

// Add to before p;
#define OMatrixTemplateAddP(_a, _e, p, el)				\
	if(!_a){ _a=el; _e=el; el->_p=0; el->_n=0; }			\
	else if(!p){										\
		el->_p=0; el->_n=_a; _a->_p=el; _a=el;			\
	} else {											\
		el->_p=p; el->_n=p->_n;							\
		p->_n=el; if(el->_n) el->_n->_p=el; else _e=el;	\
	}

// Del
#define OMatrixTemplateDel(_a, _e, el)								\
	if(el->_n) el->_n->_p=el->_p; else if(el==_e) _e=el->_p;		\
	if(el->_p) el->_p->_n=el->_n; else if(el==_a) _a=el->_n;

#define OMatrixTemplateDelF(_a, _e, el, _p, _n)						\
	if(el->_n) el->_n->_p=el->_p; else if(el==_e) _e=el->_p;		\
	if(el->_p) el->_p->_n=el->_n; else if(el==_a) _a=el->_n;


// Network
typedef unsigned char MACADDR[6];
typedef unsigned char IPV6[16];
struct MACADDR_S{ MACADDR mac; };
typedef struct {
  DWORD adr[4];
  operator bool(){ return adr[0] || adr[1] || adr[2] || adr[3]; }
} IP6_ADDRESS;


// Global Error
int globalerror();
int globalerror(const char*line);

// Time
unsigned int time();
timeb alltime();
int timemi(timeb&o, timeb&t);
int timemit(timeb&t);
int64 sectime();

// Data //
static const char cb16[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#ifdef USEMSV_MEMORYCONTROL
#include "memcon.h"
#endif

// New
#define msvcorenew(a, b) a = (b*) malloc(sizeof(b)); new(a)b;
#define msvcoredelete(a, b) a->~b(); free(a);