// Versions
class Versions{ public: VString ver, date; };

// Msv Core State
class MsvCoreState{
public:
	// Process upped
	unsigned int procupped;

	// Process arguments & environment
	char **argv, **arge;
	int args;

	// Windows path
	MString winpath;

	// Process path
	MString procpath;
	
	// This path
	MString thispath;

	// This path ilink
	ILink link;

	MsvCoreState();
	void Main(int args, char* arg[], char* arge[]);

	VString NormalPath(char *path);
	void GetWindowsDirectory();
};

extern MsvCoreState msvcorestate;


// LString
template<int stacksize>
class LStringX : public UList<unsigned char, 0, stacksize>{
public:
	LStringX();
	LStringX(LStringX &ls);

// emulate LString, pleasse DON't use it
	void add(const char data);
	void add(char *data, unsigned int sz);
	int addnf(unsigned char *data, unsigned int sz);
	VString addnf(VString line);
	unsigned char* addnfv(unsigned int sz);

	unsigned int size();
	char& el(int p);

	operator bool();
	operator const VString();

	VString String();

//#ifndef WIN32
	operator const MString();
//#endif

	char* oneline();

	int Send(SOCKET sock);
	void String(MString &line);

// end emulate LString, pleasse DON't use it

	LStringX& operator=(LStringX& lst);

	LStringX& operator+(const short i);
	LStringX& operator+(const unsigned short i);
	LStringX& operator+(const int i);
	LStringX& operator+(const unsigned int i);
	LStringX& operator+(const int64 i);
	LStringX& operator+(const uint64 i);

	LStringX& operator+(const double i);
	LStringX& operator+(const float i);

	LStringX& operator+(const MString& string);
	LStringX& operator+(const VString& string);
	LStringX& operator+(const char* lpsz);
	LStringX& operator+(LStringX& lst);

	LStringX& operator+=(const VString& string);

};


///////// LString
//typedef UList<char, 0, 0, 0> LString2;

template<int stacksize>
LStringX<stacksize>::LStringX(){
}

template<int stacksize>
LStringX<stacksize>::LStringX(LStringX &ls){
	*this + ls;
}

template<int stacksize>
void LStringX<stacksize>::add(char *data, unsigned int sz){
	this->Add((unsigned char*)data, sz);
}

template<int stacksize>
void LStringX<stacksize>::add(const char data){
	this->Add((unsigned char*)&data, 1);
}

template<int stacksize>
unsigned int LStringX<stacksize>::size(){
	return this->Size();
}

template<int stacksize>
int LStringX<stacksize>::addnf(unsigned char *data, unsigned int sz){
	int s = this->Size();
	this->Add(data, sz, 1);
	return s;
}

template<int stacksize>
VString LStringX<stacksize>::addnf(VString line){
	return VString(this->Add(line, line, 1), line);
}

template<int stacksize>
char& LStringX<stacksize>::el(int p){
	return *(char*)this->Get(p);
}

template<int stacksize>
unsigned char* LStringX<stacksize>::addnfv(unsigned int sz){
	return this->Add(0, sz, 1);
}

template<int stacksize>
char* LStringX<stacksize>::oneline(){
	return (char*)this->OneLine();
}

template<int stacksize>
LStringX<stacksize>::operator bool(){
	return this->Size() > 0;
}

template<int stacksize>
LStringX<stacksize>::operator const VString(){
	return VString(this->OneLine(), this->Size());
}

template<int stacksize>
VString LStringX<stacksize>::String(){
	return VString(this->OneLine(), this->Size());
}


//#ifndef WIN32
template<int stacksize>
LStringX<stacksize>::operator const MString(){
	return MString(this->OneLine(), this->Size());
}
//#endif

template<int stacksize>
int LStringX<stacksize>::Send(SOCKET sock){
	this->OneLine();
	return send(sock, (char*)this->Get(0), this->Size(), 0);
}

template<int stacksize>
void LStringX<stacksize>::String(MString &line){
	line.set(this->OneLine(), this->Size());
}

// end emulate LString, pleasse DON't use it

#define LString_Int_Template(T) \
template<int stacksize> \
LStringX<stacksize>& LStringX<stacksize>::operator+(const T i){ \
	unsigned char data[S1K]; \
	int s=prmf_itos(data, S1K, i); \
	this->Add(data, s); \
	return *this; \
}

LString_Int_Template(short);
LString_Int_Template(unsigned short);
LString_Int_Template(int);
LString_Int_Template(unsigned int);
LString_Int_Template(int64);
LString_Int_Template(uint64);

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator+(const double i){
	unsigned char data[S1K];
	int s = prmf_dtos(data, S1K, i);
	this->Add(data, s);
	return *this;
}

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator+(const float i){
	unsigned char data[S1K];
	int s = prmf_dtos(data, S1K, i);
	this->Add(data, s);
	return *this;
}

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator+(const VString& string){ add(string, string); return *this; }

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator+(const MString& string){ add(string, string); return *this; }

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator+=(const VString& string){ add(string, string); return *this; }

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator+(const char* string){ if(string) this->Add((unsigned char*)string, strlen(string)); return *this; }

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator+(LStringX& string){
	for(unsigned int i = 0; i < string.Blocks(); i++){
		int sz;
		unsigned char * c = string.GetBlock(i, sz);
		this->Add(c, sz);
	}
	
	return *this;
}

template<int stacksize>
LStringX<stacksize>& LStringX<stacksize>::operator=(LStringX& string){
	LStringX::Clean();

	for(unsigned int i = 0; i < string.Blocks(); i++){
		int sz;
		unsigned char * c = string.GetBlock(i, sz);
		this->Add(c, sz);
	}
	
	return *this;
}

//LString2& LString2::operator+(LString& lst){ add(lst.operator MString(), lst.size()); return *this; }

typedef LStringX<1> LString;
typedef LStringX<1> LString0;
typedef LStringX<S4K> LStringS;