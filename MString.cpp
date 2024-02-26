// Include Defines
#include "msvdefine.h"

// Include Code
#include "crossplatform/ilink.cpp"
#include "crossplatform/time.cpp"

// Include Config Line
#include "crossplatform/confline.cpp"

// Include Config Line Options
//#include "crossplatform/conflineops.cpp"

VString _vstringnull_;

// Class MString: public VString {
// --------------------------------------------------- Constructor  Конструктор

MString::MString(){ data=0; Init(); } // MSVMEMORYCONTROLC
MString::~MString(){ free(data); } // MSVMEMORYCONTROLD
void MString::Init(){ free(data); data = 0; sz=0; }

// --------------------------------------------------- Create object  Создание обьекта
void MString::StringNew(const void*line){
	if(!sz){ data=0; return ; }
	data = (unsigned char*)malloc(sz+1);
	if(!data){ sz=0; return ; }
	if(line) memcpy(data, line, sz);
	data[sz]=0;
	return ;
}

void MString::StringDel(unsigned char *line){
	free(line);
}

MString::MString(const char string){ sz=sizeof(string); StringNew(&string); return ; }
MString::MString(const char *line){ if(line){ sz=strlen((const char*)line); StringNew(line); } else { data=0; sz=0; } }
MString::MString(const unsigned char *line){ if(line){ sz=strlen((const char*)line); StringNew(line); } else { data=0; sz=0; } }
MString::MString(const char *line, const unsigned int s){ sz=s; StringNew(line); }
MString::MString(const unsigned char *line, const unsigned int s){ sz=s; StringNew(line); }

MString::MString(const short string){ sz=sizeof(string); StringNew(&string); return ; }
// short
MString::MString(const short *string, int size){
	if(!size){ data=0; return ; }
	MString ret=stoc((unsigned short*)string, size);
	data=ret.data; ret.data=0;
	sz=ret.sz; ret.sz=0;
	return ;
}
// int 
MString::MString(const int code){ sz=1; StringNew(&code); return ; }
MString::MString(const unsigned int code){ sz=1; StringNew(&code); return ; }
MString::MString(const unsigned int code, const unsigned int size){ sz=size; StringNew(0); memset(data, code, sz); return ; }

MString::MString(const VString &line){ sz=line.sz; StringNew(line); }
MString::MString(const MString &line){ sz=line.sz; StringNew(line); }

//#if __cplusplus >= 201103L || WIN32 && !__GNUC__
MString::MString(MString &&line){
	data = line.data;
	sz = line.sz;
	line.data = 0;
	line.sz = 0;
}
//#endif

// --------------------------------------------------- Set	Установка

const MString& MString::operator=(const char string){ unsigned char* odata=data; sz=1; StringNew(&string); free(odata); return *this; }
const MString& MString::operator=(const char* line){ unsigned char *odata=data; sz=line ? strlen((const char*)line) : 0; StringNew(line); free(odata); return *this; }
const MString& MString::operator=(const unsigned char* line){ unsigned char *odata=data; sz=line ? strlen((const char*)line) : 0; StringNew(line); free(odata); return *this; }
const MString& MString::operator=(const VString& line){ unsigned char *odata=data; sz=line; StringNew(line); free(odata); return *this; }
const MString& MString::operator=(const MString& line){ unsigned char *odata=data; sz=line; StringNew(line); free(odata); return *this; }

MString& MString::set(const VString line){ unsigned char *odata=data; sz=line.sz; StringNew(line); free(odata); return *this; }
MString& MString::set(const char* line, const unsigned int size){ unsigned char *odata=data; sz=size;  StringNew(line); free(odata); return *this; }
MString& MString::set(const unsigned char* line, const unsigned int size){ unsigned char *odata=data; sz=size;  StringNew(line); free(odata); return *this; }
MString& MString::setu(const unsigned char* line, const unsigned int size){ unsigned char *odata=data; sz=size; StringNew(line); free(odata); return *this; }

// --------------------------------------------------- Add  Добавление	

const MString& MString::operator+=(const VString& string){
	if(!string)
		return *this;

	unsigned char* odata = data;
	int szo = sz, szt = string.sz;
	sz = szo + szt; 

	StringNew(0);

	memcpy(data, odata, szo);
	memcpy(data + szo, string.data, szt);

	if(sz)
		data[sz] = 0; 

	free(odata);

	return *this;
}

const MString& MString::operator+=(const char string){
	unsigned char* odata = data;
	int szo = sz, szt = sizeof(string);
	sz = szo + szt; 

	StringNew(0);

	memcpy(data, odata, szo);
	memcpy(data + szo, &string, szt);

	if(sz)
		data[sz] = 0; 

	free(odata);

	return *this;
}

MString& MString::Add(VString l1, VString l2, VString l3, VString l4, VString l5){
	MSTRING_TEMPLATE_ADD5;
}

MString& MString::Add(VString l1, VString l2, VString l3, VString l4, VString l5, VString l6, VString l7, VString l8, VString l9){
	MSTRING_TEMPLATE_ADD9;
}

MString& MString::Add(VString l1, VString l2, VString l3, VString l4, VString l5, VString l6, VString l7, VString l8, VString l9,
	VString l10, VString l11, VString l12, VString l13, VString l14, VString l15, VString l16, VString l17, VString l18, VString l19){
		MSTRING_TEMPLATE_ADD19;
}

MString& MString::AddR(VString &l1, VString &l2, VString &l3, VString &l4, VString &l5, VString &l6, VString &l7, VString &l8, VString &l9){
	MSTRING_TEMPLATE_ADDR9;
}

//---------------------------------------------------- Move   Перемещение

void MString::move(MString& string){
	free(data);
	sz=string.sz; data=string.data;
	string.data=0; string.sz=0;
	return ;
}

MString& MString::operator-=(MString& string){
	free(data);
	sz=string.sz; data=string.data;
	string.data=0; string.sz=0;
	return *this;
}

//void MString::newsize(unsigned int s){ sz=s; return ; }

//---------------------------------------------------- Operators  Операторы
//---------------------------------------------------- Index  Указатели
//---------------------------------------------------- Association Обьединение

MString operator+(const MString& string, const MString& stringt){
	int szo=string.sz; int szt=stringt.sz;

    MString ret; ret.sz=szo+szt;
	ret.StringNew(0); if(!ret.data) return 0;
	memcpy(ret.data, string.data, szo);
	memcpy(ret.data+szo, stringt.data, szt);
	ret.data[ret.sz]=0;
	return ret;
}

MString operator+(const MString& string, const VString& stringt){
	int szo=string.sz; int szt=stringt.sz;

    MString ret; ret.sz=szo+szt;
	ret.StringNew(0); if(!ret.data) return 0;
	memcpy(ret.data, string.data, szo);
	memcpy(ret.data+szo, stringt.data, szt);
	ret.data[ret.sz]=0;
	return ret;
}


MString operator+(const MString& string, const char* lpsz){
	int szo=string.sz; if(!lpsz) return string;
	int szt=(unsigned int)strlen(lpsz);

	MString ret; ret.sz=szo+szt;
	ret.StringNew(0);
	memcpy(ret.data, string.data, szo);
	memcpy(ret.data+szo, lpsz, szt);
	ret.data[ret.sz]=0;
	return ret;
}

MString operator+(const char* lpsz, const MString& string){
	int szo=string.sz; int szt=(unsigned int)strlen(lpsz);

	MString ret; ret.sz=szo+szt;
	ret.StringNew(0);
	memcpy(ret.data, lpsz, szt);
	memcpy(ret.data+szt, string.data, szo);
	ret.data[ret.sz]=0;
	return ret;
}

//---------------------------------------------------- Part  Часть

//---------------------------------------------------- Operations  Операции

MString& MString::Reserve(int size, int p){
	if(size==sz) return *this; if(size<=0){ free(data); data=0; sz=0; return *this; }
	unsigned char*odata=data; int s=sz; sz=size;
	StringNew(0); int m=minel(s, size);
	memcpy(data, odata, m); free(odata);
	memset(data+m, p, sz-m);
	data[size]=0;
	return *this;
}

void MString::RClean(int sz, int p){
	Reserve(sz);
	CleanData(p);
}

void MString::CleanData(int p){
	memset(data, p, sz);
}

void MString::Clean(){
	Init();
}

// }; end MString



#include "TString.cpp"