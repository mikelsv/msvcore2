// Global include
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <new>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <wchar.h>

#ifndef WINCE
#include <sys/timeb.h>
#endif
// ~ Global Include

// OS Include
#ifdef WIN32 // Win32 include
	#ifdef __GNUC__
		#define _WIN32_WINNT  0x501
	#endif

	#include <direct.h>
	#include <winsock2.h>
	#include <windows.h>	
	#include <ws2tcpip.h>

	#define S_IFLNK 0xC000
#ifndef __GNUC__
	#define S_IRUSR 0x0100
	#define S_IWUSR 0x0080
	#define S_IXUSR 0x0040
	#define S_IRGRP 0x0020
	#define S_IWGRP 0x0010
	#define S_IXGRP 0x0008
	#define S_IROTH 0x0004
	#define S_IWOTH 0x0002
	#define S_IXOTH 0x0001
#endif

	typedef __int64 int64;
	typedef unsigned __int64 uint64;

	int stdmkdir(const char *path, int mode=755);

#else // Linux include
	#include <arpa/inet.h>
	#include <errno.h>
	#include <dirent.h>
	#include <netdb.h>
	#include <sys/ioctl.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/utsname.h>
	#include <sys/wait.h>
	#include <termios.h>
	#include <utime.h>

	#define memcpy memmove
	#define closesocket(sock) close(sock)

	#define FILE_BEGIN		0
	#define FILE_CURRENT	1
	#define FILE_END		2

	#define O_BINARY 0

	#define _O_CREAT O_CREAT
	#define _O_RDONLY O_RDONLY
	#define _O_WRONLY O_WRONLY
	#define _O_RDWR O_RDWR
	#define _O_EXCL O_EXCL
	#define _O_BINARY O_BINARY

	#define _S_IREAD S_IREAD //???
	#define _S_IWRITE S_IWRITE //???
	#define _S_IFDIR S_IFDIR //???

	typedef int				SOCKET;
	typedef int				HFILE;
	typedef unsigned int 	HANDLE;
	typedef unsigned char	BYTE;
	typedef unsigned short	WORD;
	typedef unsigned long	DWORD;
	typedef long			LONG;
	typedef unsigned int	UINT;
	typedef void*			LPVOID;
	typedef unsigned long*	LPDWORD;
	typedef int				BOOL;
	typedef int				INT;
	typedef short			SHORT;
	typedef WORD			WCHAR;
	typedef char			CHAR;
	typedef unsigned char	UCHAR;
	typedef unsigned short	USHORT;
	typedef unsigned long	ULONG;
	typedef unsigned int	HWND;
	typedef const CHAR		*LPCSTR;
	typedef float			FLOAT;
	typedef double			DOUBLE;
	typedef long long		int64;
	typedef unsigned long long uint64;
	typedef sockaddr_in		SOCKADDR_IN;
	typedef sockaddr		SOCKADDR;
	typedef sockaddr		*LPSOCKADDR;

	typedef struct _FILETIME {
		DWORD dwLowDateTime;
		DWORD dwHighDateTime;
	} FILETIME;
	
	void Sleep(unsigned int i);
	HANDLE GetCurrentThread();
	DWORD GetCurrentThreadId();
	void GetThreadTimes(HANDLE threadid, FILETIME *c, FILETIME*e, FILETIME *k, FILETIME *u);
	int stdmkdir(const char *path, int mode = 755);
#endif
// ~ OS Include

#ifdef __GNUC__
	#include <pthread.h>
	#include <unistd.h>
#endif


// Types
//typedef unsigned char  BYTE; // Allready
//typedef unsigned long  DWORD; // Allready

// u
typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define VOIDSIZE (sizeof(void*))
#define UMSZ (sizeof(char*)+sizeof(int))
#define UMCSZ sizeof(char*)

#ifndef NULL
#define NULL	0
#endif
					
#ifndef TRUE
#define	TRUE true
#endif

#ifndef FALSE
#define	FALSE false
#endif

//#ifdef __GNUC__
#ifndef max
//#define max(a,b)            (((a) > (b)) ? (a) : (b))
inline bool max(int a, int b){ return a > b ? a : b; }
#endif
#ifndef min
//#define min(a,b)            (((a) < (b)) ? (a) : (b))
inline bool min(int a, int b){ return a < b ? a : b; }
#endif
//#endif

#ifdef USEMSV_ANDROID
	#define __S_ISUID       04000   /* Set user ID on execution.  */
	#define __S_ISGID       02000   /* Set group ID on execution.  */
	#define __S_ISVTX       01000   /* Save swapped text after use (sticky).  */
	#define __S_IREAD       0400    /* Read by owner.  */
	#define __S_IWRITE      0200    /* Write by owner.  */
	#define __S_IEXEC       0100    /* Execute by owner.  */

	#define S_IREAD		__S_IREAD
	#define S_IWRITE	__S_IWRITE
#endif


