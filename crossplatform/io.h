#ifdef WIN32
	#include <io.h>
	#define lseek64 _lseeki64
	#define tell64 _telli64
	#define lstat64	_stati64
	#define _rename rename

#ifdef __GNUC__
	#define chsize64 ftruncate
#else
	#define chsize64 _chsize_s
#endif

	typedef struct _stati64 sstat64;
#else
	#define _open(...)	open(__VA_ARGS__)
	#define _read(...) read(__VA_ARGS__)
	#define _write(...) write(__VA_ARGS__)
	#define _close(...) close(__VA_ARGS__)
	#define _rename rename

	#define _fstati64 fstat64
	#define _stati64 stat64
	//#define sstat64 struct stat64
	typedef struct stat64 sstat64;

#endif

#ifdef WIN32
	extern HANDLE _std_input_handle;
	extern HANDLE _std_output_handle;

	void MsvCoreAllocConsole();
#endif

// Print
int print(const VString &line);

// Multi Print
inline int print(const VString &l1, const VString &l2);
inline int print(const VString &l1, const VString &l2, const VString &l3);
inline int print(const VString &l1, const VString &l2, const VString &l3, const VString &l4);
inline int print(const VString &l1, const VString &l2, const VString &l3, const VString &l4, const VString &l5
, const VString &l6=VString(), const VString &l7=VString(), const VString &l8=VString(), const VString &l9=VString(), const VString &l10=VString());


// Normalpath
#define FSCFSZ	(2048) // FileSystem path max size

#define MSVCORE_NORMALPATH(buf, file) unsigned char buf[FSCFSZ]; \
	{ int rsz = normalpath((char*)buf, FSCFSZ, file, file, 0, 1); file.set((char*)buf, rsz); } 

int normalpath(char*file, int asz, char *in, int ins, bool r=0, int opt=0);


// File Operations
MString LoadFile(VString file);
bool IsFile(VString file);
unsigned int SaveFile(VString file, VString data);
unsigned int SaveFileAppend(VString file, VString data);

bool SetFileTime(HFILE fl, time_t ctime, time_t atime, time_t mtime);

bool MakeDir(VString path, VString fold);

