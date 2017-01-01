class RecvDataD{
public:
	MString data;
	unsigned int sz;

	RecvDataD();
	void Clean();
	~RecvDataD();
};


class RecvData{
public:
	RecvDataD *data;
	VString abuff, buff;
	unsigned int dsz;

	RecvData();

	VString GetFree();
	VString GetBuffer();
	
	int Read(RecvDataD &d, VString b, VString read);
	int Add(int s);
	int End();
	int Del(int s);

	~RecvData();
};


// SendDataGLock - send data [one global lock]. Rebuild SendDataL class.
// Created 21.10.2016 16:55
struct SendDataD{
	SendDataD *n;
	unsigned int asz, usz, ssz; // all size, use size, send size;
	unsigned char data[0];
}; // 10, 12, 16. [align 1, 4, 8]

class SendData{
public:
	SendDataD *a, *e;
	unsigned int asz, usz, dsz; // all size, use size, del size(sended)
	
	SendData();

	void* Get(unsigned int &se);
	unsigned int Get(void *dt, unsigned int se);

	unsigned int Size();
	
protected:
	SendDataD* New(unsigned int ns);

public:
	unsigned int Set(const VString &line);
	unsigned int Set(const void*dta, unsigned int s);

	void Del(unsigned int se);
	void DelA();

	void Clean();

	~SendData();
};


// Send Data Ring
// TLock SendDataRingLock;

#define SENDDATALOCK_BUFSZ	S8K

class SendDataRing{
	MString buf;
	unsigned int pread, pwrite;
	
public:
	SendDataRing();

	// Max Read
	int IsRead();
	int IsFree();

	int Read(VString b);
	
	VString ReadData();
	int Readed(int sz);

	int Write(VString b);

	int Reserv(int sz = SENDDATALOCK_BUFSZ);

	void Clean();

	~SendDataRing();
};