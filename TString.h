// TString by MikelSV
// 0.0.0.1  19.10.2015 13:03

class TString : public VString{
	// --------------------------------------------------- Data	 ������
//	unsigned char *data;
//	unsigned int sz;

	// --------------------------------------------------- Constructor  �����������
public:	
	TString();
	~TString();
	void Init();

protected:
	void StringNew(const void *line);
	void StringDel(unsigned char *line);

	// --------------------------------------------------- Create object  �������� �������
public:
	TString(const char string);
	TString(const char *line);
	TString(const unsigned char *line);
	TString(const char *line, const unsigned int s);
	TString(const unsigned char *line, const unsigned int s);

	TString(const short string);
	TString(const short *line, int size);
	TString(const int code);
	TString(const unsigned int code);
	TString(const unsigned int code, const unsigned int sz);
	
	TString(const VString &line);
	//TString(const MString &line);
	TString(const TString &line);

//#if __cplusplus >= 201103L || WIN32 && !__GNUC__
	TString(TString &&line);
//#endif

	// --------------------------------------------------- Set	���������

	const TString& operator=(char string);
	const TString& operator=(const char* line);
	const TString& operator=(const unsigned char* line);
	const TString& operator=(const VString& line);
	//const TString& operator=(const MString& line);

	TString& set(const VString line);
	TString& set(const char* line, const unsigned int size);
	TString& set(const unsigned char* line, const unsigned int size);
	TString& setu(const unsigned char* line, const unsigned int size);

	// --------------------------------------------------- Add  ����������	

	//const MString& operator+=(const MString& string);
	const TString& operator+=(const VString& line);
	const TString& operator+=(const char line);
	TString& Add(VString o, VString t, VString th=VString(), VString f=VString(), VString fi=VString());
	TString& Add(VString l1, VString l2, VString l3, VString l4, VString l5, VString l6, VString l7=VString(), VString l8=VString(), VString l9=VString());
	TString& Add(VString l1, VString l2, VString l3, VString l4, VString l5, VString l6, VString l7, VString l8, VString l9,
		VString l10, VString l11=VString(), VString l12=VString(), VString l13=VString(), VString l14=VString(), VString l15=VString(), VString l16=VString(), VString l17=VString(), VString l18=VString(), VString l19=VString());
	TString& AddR(VString &l1, VString &l2=_vstringnull_, VString &l3=_vstringnull_, VString &l4=_vstringnull_, VString &l5=_vstringnull_, VString &l6=_vstringnull_, VString &l7=_vstringnull_, VString &l8=_vstringnull_, VString &l9=_vstringnull_);

	//---------------------------------------------------- Move   �����������

	TString& operator-=(TString& line);
	void move(TString& line);
		
	//---------------------------------------------------- Association �����������

	//friend TString operator+(const TString& ln1, const TString& ln2);
	//friend TString operator+(const TString& ln1, const VString& ln2);
 	//friend TString operator+(const TString& ln1, const char* ln2);
	//friend TString operator+(const char* ln1, const TString& ln2);

	//---------------------------------------------------- Operations  ��������

	TString& Reserve(int size, int p=0);
	//MString& ReservM(int size, int p=0); // Reserv More
	void CleanData(int p);
	void RClean(int sz, int p);
	//void RSize(int size, int p=0);

	void Clean();
};


#define THREADSTRINGINIT ThreadStringInit();
#define THREADSTRINGDEST ThreadStringDest();


void ThreadStringInit();
void ThreadStringDest();
unsigned char * ThreadStringNew(int sz);
void ThreadStringDel(unsigned char *p);




// Int library
//int stoi(char *line, unsigned int size=0);
TString itos(int64 val, int radix=10, int null=1);
TString itob(unsigned int val, int null=0);
TString dtos(double val, int ml=5);
//MString htob(VString line);
TString gditos(double i);
int itossz(int64 val);
int bsize16(int64 val);

TString htob(VString line);
TString btoh(VString line);

// Replace
TString Replace(VString line, VString fr, VString to, unsigned int cnt = 0);

//Shost to char
unsigned char stoc(const unsigned short s);
TString stoc(const unsigned short *s, const int sz);

// Char to Short
unsigned short ctos(const unsigned char s);
TString ctos(const unsigned char*s, const int sz);
TString ctos(VString line);