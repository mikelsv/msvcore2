class RecvDataD{
public:
	MString data;
	unsigned int sz;

	RecvDataD(){
		sz=0;
	}

	void Clean(){
		data.Clean();
		sz=0;
		return ;
	}

	~RecvDataD(){
		Clean();
	}

};

class RecvData{
public:
	RecvDataD *data;
	VString abuff, buff;
	unsigned int dsz;

	RecvData(){
		data=0;
		dsz=0;
	}

	~RecvData(){
		End();
	}

	int Read(RecvDataD &d, VString b, VString read){
		data=&d;
		buff=abuff=b;

		if(/*!data ||*/ data->sz + read.sz > buff)
			return 0;

		memcpy(buff, data->data, data->sz);
		memcpy(buff.data+data->sz, read, read);

		buff.sz=data->sz+read.sz;
		return 1;
	}

	int Del(int s){
		dsz+=s;
		return 1;
	}

	int Add(int s){
		if(buff.sz+s<=abuff.sz)
			buff.sz+=s;
		else
			buff.sz=abuff.sz;
		return 1;
	}

	int End(){
		if(dsz >= buff)
			buff.Clean();
		else{
			buff.data+=dsz;
			buff.sz-=dsz;
		}
		dsz=0;

		if(data->data.sz < buff){
			data->data.Reserve(buff.sz+S1K-buff.sz%1024);
		}
		
		memcpy(data->data, buff, buff);
		data->sz=buff;
		return 1;
	}

	VString GetBuffer(){
		return buff.str(dsz);
	}

	VString GetFree(){
		return VString(buff.endu(), abuff.sz-buff);
	}
};



// SendDataGLock - send data [one global lock]. Rebuild SendDataL class.
// Created 21.10.2016 16:55
#ifndef MSVCORE_SENDDATA_BUFSZ
	#define MSVCORE_SENDDATA_BUFSZ S8K
#endif

TLock SendDataLock;

struct SendDataD{
	SendDataD *n;
	unsigned int asz, usz, ssz; // all size, use size, send size;
	unsigned char data[0];
}; // 10, 12, 16. [align 1, 4, 8]

//unsigned short sz[CNT_SDPSZ], asz[CNT_SDPSZ], ssz[CNT_SDPSZ]; // size, all size, send size
//char*data[CNT_SDPSZ]; BYTE tpos, spos;

class SendData{
public:
	SendDataD *a, *e;
	unsigned int asz, usz, dsz; // all size, use size, del size(sended)
	
	SendData(){
		memset(this, 0, sizeof(SendDataD));
	}

	~SendData(){
		Clean();
	}

	void* Get(unsigned int &se){
		UGLOCK(SendDataLock);

		if(!a || !dsz){
			se = 0;
			return 0;
		}

		se = a->usz - a->ssz;
		
		return a->data + a->ssz;
	}

	unsigned int Get(void *dt, unsigned int se){
		if(!a || !dsz){
			return 0;
		}

		UGLOCK(SendDataLock);

		SendDataD *d = a;
		unsigned int s = 0, t;

		while(d && se){
			t = d->usz - d->ssz > se ? se : d->usz - d->ssz;
			memcpy(dt, d->data + d->ssz, t);
			s += t;
			dt = (char*)dt + t;
			se -= t;
			d = d->n;
		}

		return s;
	}
	
protected:
	SendDataD* New(unsigned int ns){ // prev lock
		if(ns % MSVCORE_SENDDATA_BUFSZ)
			ns = ns + MSVCORE_SENDDATA_BUFSZ - ns % MSVCORE_SENDDATA_BUFSZ;
			
		SendDataD *d = (SendDataD*)malloc(ns); //mcnew(s);
		d->asz = ns - sizeof(SendDataD);
		d->n = 0;
		d->usz = 0;
		d->ssz = 0;

		if(e){
			e->n = d;
			e = d;
		}
		else {
			a = d;
			e = d; 
		}		
		
		asz += d->asz;

		return d;
	}

public:
	unsigned int Set(const VString &line){
		return Set(line, line);
	}

	unsigned int Set(const void*dta, unsigned int s){
		if(!s || !dta)
			return 1;

		UGLOCK(SendDataLock);
		unsigned int ef, se = 0;

		// Use last element
		if(e){
			if(e->ssz == e->usz){
				usz -= e->usz;
				e->usz = 0;
				e->ssz = 0;
			}
			ef = e->asz - e->usz;
		}else
			ef = 0;

		while(s){
			if(ef){
				if(ef > s)
					ef = s;

				memcpy(e->data + e->usz, dta, ef);
				dta = (char*)dta + ef;
				dsz += ef;
				usz += ef;
				e->usz += ef;				
				s -= ef;
				se += ef;
			}
			if(!s)
				return se;

			if(!New(s))
				return se;

			ef = e->asz;
		}

		return se;
	}

	void Del(unsigned int se){
		if(!a || !dsz || !se){
			se = 0;
			return ;
		}

		UGLOCK(SendDataLock);
		unsigned int t;

		if(se > usz || se > dsz){
			while(a && se && se >= a->usz - a->ssz){
				se -= a->usz - a->ssz;
				dsz -= a->usz - a->ssz;
				a->ssz = a->usz;
				if(a->asz == a->usz)
					DelA();
			}
		}

		t= a ? (((se) < (a->usz - a->ssz)) ? (se) : (a->usz - a->ssz)) : 0;
		if(a && se && t){
			a->ssz += t;
			dsz -= t;
		}

		return ;
	}

	void DelA(){ // prev lock
		if(!a)			
			return ;

		SendDataD *d = a;

		a = a->n;

		if(!a)
			e = 0;

		asz -= d->asz;
		usz -= d->usz;
		dsz -= d->usz - d->ssz;
		
		free(d);

		return ;
	}

	unsigned int Size(){
		return dsz;
	}

	void Clean(){
		SendDataD *p, *d;

		{
			UGLOCK(SendDataLock);

			p = a;
			d = p;

			a = 0;
			e = 0;
			asz = 0;
			usz = 0;
			dsz = 0;
		}

		while(p){
			d = p;
			p = p->n;
			free(d);
		}

		return ;
	}

};


// Send Data Ring
TLock SendDataRingLock;

#define SENDDATALOCK_BUFSZ	S8K

class SendDataRing{
	MString buf;
	unsigned int pread, pwrite;
	
public:
	SendDataRing(){
		pread = 0;
		pwrite = 0;
	}

	// Max Read
	int IsRead(){
		return pwrite >= pread ? pwrite - pread : buf.sz - pread + pwrite;
		//return (buf.sz + pread - pread) % buf.sz;
	}

	int IsFree(){
		return buf.sz ? buf.sz - IsRead() : SENDDATALOCK_BUFSZ;
	}

	int Read(VString b){
		UGLOCK(SendDataRingLock);
		if(!b.sz)
			return 0;

		unsigned int ms = pwrite >= pread ? pwrite - pread : buf.sz - pread;
		unsigned int s = minel(ms, b.sz);
		memcpy(b, buf.data + pread, s);

		if(pwrite < pread && b.sz > ms){
			unsigned int ns = minel(pwrite, b.sz - s);
			memcpy(b.data + s, buf.data, ns);
			s+= ns;
		}

		return s;
	}
	
	VString ReadData(){
		UGLOCK(SendDataRingLock);
		if(pwrite >= pread)
			return VString(buf.data + pread, pwrite - pread);
		else
			return VString(buf.data + pread, buf.sz - pread);
	}

	int Readed(int sz){
		UGLOCK(SendDataRingLock);
		if(sz > IsRead())
			sz = IsRead();

		if(sz && buf.sz)
			pread = (pread + sz) % buf.sz;

		return 1;
	}

	int Write(VString b){
		UGLOCK(SendDataRingLock);
		if(!buf)
			Reserv();

		int as = 0;

		while(1){
			unsigned int f = buf.sz - IsRead();
			unsigned int s = minel(f, b.sz - as);
			if(!s)
				break;

			memcpy(buf.data + pwrite, b.data + as, s);

			pwrite = (pwrite + s) % buf.sz;
			as += s;
		}

		return as;
	}

	int Reserv(int sz = SENDDATALOCK_BUFSZ){
		UGLOCK(SendDataRingLock);
		return buf.Reserve(sz);
	}

	void Clean(){
		UGLOCK(SendDataRingLock);
		pread = 0;
		pwrite = 0;
		buf.Clean();
	}

	~SendDataRing(){
		UGLOCK(SendDataRingLock);
		buf.Clean();
	}

};