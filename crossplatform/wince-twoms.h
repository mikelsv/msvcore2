class MStringS{
short *data;
unsigned int sz;

public:
MStringS(){data=0; Init();}
~MStringS();
void Init();

MStringS(const MString string);
MStringS(const VString string);
MStringS(const char* string);
MStringS(const wchar_t*);
operator wchar_t*(){return (wchar_t*)data;}
operator short*(){return (short*)data;}
int size()const{return sz;}
operator MString();

};


//#ifdef WINCE
#define MODUNICODE(x) MStringS(x)
//#define MODUNICODEV(x) MStringS(VString(x))
#define MODUNICODEV(x, s) MStringS(VString((unsigned char*)x, s)), s
#define MODLPWSTR	LPWSTR
#define MODUNICODET(x) L##x
//#define MODUNIN	MString