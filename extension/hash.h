// Base64
static const char b58digits_ordered[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

class Base64{
public:
	static TString btos(const VString line);
	static TString stob(const VString line);
	static TString mbtos(const VString line);
	static TString mstob(const VString line);

	// Base 58
	static TString btos58(const VString line);
};

// MD5 - 6.12.05 1.0.0 MSVSPL Specila Library
#include "../crypto/md5.h"
#include "../crypto/rmd160.h"
//#include "msv/sha1.h"
#include "../crypto/sha.h"

#define MHASHT_MD5		1
#define MHASHT_RMD160	2
#define MHASHT_SHA1		4
#define MHASHT_SHA256	8
#define MHASHT_ALL		255

#define MHASHT_MAXDATA	32
//#define USEMSV_MHASHT_GETALL	// for generate all

TString md5(VString line);
TString md5h(VString line);

class MHash{
	unsigned char data[MHASHT_MAXDATA*3]; int type;
#ifndef USEMSV_MHASHT_GETALL
	union{
#endif
		md5_state_t md5;  
		RMD160_CTX rmd;
		SHA1Context sha1;
		SHA256Context sha256;
#ifndef USEMSV_MHASHT_GETALL	
	};
#endif

public:
	VString ret, reth;

	MHash(int t);
	int Type(int t);
	int Type();

	void Init();

	void Add(VString line);
	void Finish();
	void Finish(int t); // finish for type
	VString Get();
    VString GetH();
	VString FastH(int type, VString line);
};