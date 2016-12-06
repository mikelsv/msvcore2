/*
 * Company:	MSV (MikelSV) code name Vyris(x)
 * This: MString - My String class
 *		 My version &CString class
 *
/*
  --------------------------------------------------------------------------------

	COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:

	MString version 1.4.5 10/April/2005
	MString version 0.1.5.0 24/Febrary/2007
	MString version 0.2.0.0 25/November/2012

	MString : Copyright (C) 2004 - 2100, Mishael Senin
  --------------------------------------------------------------------------------
  --------------------------------------------------------------------------------
 */

#define		MSTRING_VER "0.2.0.0"

extern VString _vstringnull_;

class MString: public VString{
	// --------------------------------------------------- Data	 Данные
protected:
//	unsigned char *data;
//	unsigned int sz;

	// --------------------------------------------------- Constructor  Конструктор
public:	
	void Init();
	MString();
	~MString();
	
protected:
	void StringNew(const void*line);
	void StringDel(unsigned char *line);

	// --------------------------------------------------- Create object  Создание обьекта
public:
	MString(const char string);
	MString(const char *line);
	MString(const unsigned char *line);
	MString(const char *line, const unsigned int s);
	MString(const unsigned char *line, const unsigned int s);

	MString(const short string);
	MString(const short *line, int size);
	MString(const int code);
	MString(const unsigned int code);
	MString(const unsigned int code, const unsigned int sz);
	
	MString(const VString &line);
	MString(const MString &line);

#if __cplusplus >= 201103L || WIN32
	MString(MString &&line);
#endif
	
	// --------------------------------------------------- Set	Установка

// + const MString& operator=... ??
	const MString& operator=(char string);
	const MString& operator=(const char* line);
	const MString& operator=(const unsigned char* line);
	const MString& operator=(const VString& line);
	const MString& operator=(const MString& line);
	//MString& operator=(const MString& line){ sz=line; unsigned char *odata=data; MStringNew(line); _deln(odata); return *this; }
	
	MString& set(const VString line);
	MString& set(const char* line, const unsigned int size);
	MString& set(const unsigned char* line, const unsigned int size);
	MString& setu(const unsigned char* line, const unsigned int size);

	// --------------------------------------------------- Add  Добавление	

	//const MString& operator+=(const MString& string);
	const MString& operator+=(const VString& line);
	const MString& operator+=(const char line);
	MString& Add(VString o, VString t, VString th=VString(), VString f=VString(), VString fi=VString());
	MString& Add(VString l1, VString l2, VString l3, VString l4, VString l5, VString l6, VString l7=VString(), VString l8=VString(), VString l9=VString());
	MString& Add(VString l1, VString l2, VString l3, VString l4, VString l5, VString l6, VString l7, VString l8, VString l9,
		VString l10, VString l11=VString(), VString l12=VString(), VString l13=VString(), VString l14=VString(), VString l15=VString(), VString l16=VString(), VString l17=VString(), VString l18=VString(), VString l19=VString());
	MString& AddR(VString &l1, VString &l2=_vstringnull_, VString &l3=_vstringnull_, VString &l4=_vstringnull_, VString &l5=_vstringnull_, VString &l6=_vstringnull_, VString &l7=_vstringnull_, VString &l8=_vstringnull_, VString &l9=_vstringnull_);

	//---------------------------------------------------- Move   Перемещение

	MString& operator-=(MString& line);
	void move (MString& line);
		
	//---------------------------------------------------- Operators  Операторы

	// use VString operators

	//---------------------------------------------------- Index  Указатели

	// use VString index

	//---------------------------------------------------- Association Обьединение

	friend MString operator+(const MString& ln1, const MString& ln2);
	friend MString operator+(const MString& ln1, const VString& ln2);
 	friend MString operator+(const MString& ln1, const char* ln2);
	friend MString operator+(const char* ln1, const MString& ln2);

	//---------------------------------------------------- Part  Часть

	// use VString str

	//---------------------------------------------------- Operations  Операции

	MString& Reserve(int size, int p=0);
	void CleanData(int p);
	void RClean(int sz, int p);

	int getbit(int b){ if(!data || (int)sz<(b+7)/8) return 0; return data[b/8]&1<<(b%8);  }
	int setbit(int b, int v){
		if(!data || (int)sz<(b+7)/8) return 0;
		if(v) data[b/8]|=1<<(b%8);
		else data[b/8]&=~(1<<(b%8));
		return v;
	}

	void Clean();

};

// Defines
#define MSTRING_TEMPLATE_ADD5										\
	sz = l1.size() + l2.size() + l3.size() + l4.size() + l5.size(); \
	if(!sz){ StringDel(data); data = 0; return *this; }				\
	unsigned char *odata = data;									\
	StringNew(0);													\
	unsigned char *ndata = data;									\
	memcpy(ndata, l1, l1); ndata += l1.size();						\
	memcpy(ndata, l2, l2); ndata += l2.size();						\
	memcpy(ndata, l3, l3); ndata += l3.size();						\
	memcpy(ndata, l4, l4); ndata += l4.size();						\
	memcpy(ndata, l5, l5); ndata += l5.size();						\
	*ndata = 0;														\
	StringDel(odata);												\
	return *this;

#define MSTRING_TEMPLATE_ADD9										\
	sz = l1.size() + l2.size() + l3.size() + l4.size() + l5.size() + l6.size() + l7.size() + l8.size() + l9.size(); \
	if(!sz){ StringDel(data); data = 0; return *this; }				\
	unsigned char *odata = data;									\
	StringNew(0);													\
	unsigned char *ndata = data;									\
	memcpy(ndata, l1, l1); ndata += l1.size();						\
	memcpy(ndata, l2, l2); ndata += l2.size();						\
	memcpy(ndata, l3, l3); ndata += l3.size();						\
	memcpy(ndata, l4, l4); ndata += l4.size();						\
	memcpy(ndata, l5, l5); ndata += l5.size();						\
	memcpy(ndata, l6, l6); ndata += l6.size();						\
	memcpy(ndata, l7, l7); ndata += l7.size();						\
	memcpy(ndata, l8, l8); ndata += l8.size();						\
	memcpy(ndata, l9, l9); ndata += l9.size();						\
	*ndata = 0;														\
	StringDel(odata);												\
	return *this;

#define MSTRING_TEMPLATE_ADDR9										\
	sz = l1.size() + l2.size() + l3.size() + l4.size() + l5.size() + l6.size() + l7.size() + l8.size() + l9.size(); \
	if(!sz){ StringDel(data); data = 0; return *this; }				\
	unsigned char *odata = data;									\
	StringNew(0);													\
	unsigned char *ndata = data;									\
	memcpy(ndata, l1, l1); l1.data = ndata; ndata += l1.size();		\
	memcpy(ndata, l2, l2); l2.data = ndata; ndata += l2.size();		\
	memcpy(ndata, l3, l3); l3.data = ndata; ndata += l3.size();		\
	memcpy(ndata, l4, l4); l4.data = ndata; ndata += l4.size();		\
	memcpy(ndata, l5, l5); l5.data = ndata; ndata += l5.size();		\
	memcpy(ndata, l6, l6); l6.data = ndata; ndata += l6.size();		\
	memcpy(ndata, l7, l7); l7.data = ndata; ndata += l7.size();		\
	memcpy(ndata, l8, l8); l8.data = ndata; ndata += l8.size();		\
	memcpy(ndata, l9, l9); l9.data = ndata; ndata += l9.size();		\
	*ndata = 0;														\
	StringDel(odata);												\
	return *this;

#define MSTRING_TEMPLATE_ADD19										\
	sz = l1.size() + l2.size() + l3.size() + l4.size() + l5.size() + l6.size() + l7.size() + l8.size() + l9.size()	\
		+ l10.size() + l11.size() + l12.size() + l13.size() + l14.size() + l15.size() + l16.size() + l17.size()		\
		+ l18.size() + l19.size();									\
	if(!sz){ StringDel(data); data = 0; return *this; }				\
	unsigned char *odata = data;									\
	StringNew(0);													\
	unsigned char *ndata = data;									\
	memcpy(ndata, l1, l1); ndata += l1.size();						\
	memcpy(ndata, l2, l2); ndata += l2.size();						\
	memcpy(ndata, l3, l3); ndata += l3.size();						\
	memcpy(ndata, l4, l4); ndata += l4.size();						\
	memcpy(ndata, l5, l5); ndata += l5.size();						\
	memcpy(ndata, l6, l6); ndata += l6.size();						\
	memcpy(ndata, l7, l7); ndata += l7.size();						\
	memcpy(ndata, l8, l8); ndata += l8.size();						\
	memcpy(ndata, l9, l9); ndata += l9.size();						\
	memcpy(ndata, l10, l10); ndata += l10.size();					\
	memcpy(ndata, l11, l11); ndata += l11.size();					\
	memcpy(ndata, l12, l12); ndata += l12.size();					\
	memcpy(ndata, l13, l13); ndata += l13.size();					\
	memcpy(ndata, l14, l14); ndata += l14.size();					\
	memcpy(ndata, l15, l15); ndata += l15.size();					\
	memcpy(ndata, l16, l16); ndata += l16.size();					\
	memcpy(ndata, l17, l17); ndata += l17.size();					\
	memcpy(ndata, l18, l18); ndata += l18.size();					\
	memcpy(ndata, l19, l19); ndata += l19.size();					\
	*ndata = 0;														\
	StringDel(odata);												\
	return *this;



// Include headers
#include "crossplatform/ilink.h"
#include "crossplatform/time.h"


// Conf Line
class ConfLine{
public:
	MString data, file;

public:
	ConfLine();
	ConfLine(VString file);

	int Is();
    
	VString Load(VString fl);
	VString LoadData(VString fl, VString dt);
	VString Reload();
	int Save();
	int Save(VString fl);
	VString operator[](VString line);
	VString Get(VString line);
	VString Set(VString line, VString set);
	VString Set(VString line, int64 set);
	VString GetLine(unsigned int &p);
	VString GetData();
	unsigned int GetLine(VString &line, unsigned int p);
	unsigned int GetLine(VString &key, VString &val, unsigned int p);
	void Clear();
	void ClearData();
};

