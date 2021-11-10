#define _del(v){ if(v){ free(v); } } //msv_memoty_control_destr(1, 0); } }
//#define _dela(v){ if(v){ delete[] v; msv_memoty_control_destr(1, 0); } }
#define _deln(v){ if(v){ free(v); v=0; } } //msv_memoty_control_destr(1, 0); } }
#define _newn(sz) (unsigned char*) malloc(sz) //msv_memoty_control_malloc(sz)



// Mbool
class mbool{
BYTE * data;
unsigned int sz;
public:
mbool();//{data=0; sz=0;}
mbool(void*val);//{data=(BYTE*)val; sz=0;}
void Del();//{_deln(data);}
bool* New(unsigned int size);//{ sz=size/8 + (size%8>0 ? 1 : 0); data=(BYTE*)GetBuffer( sz ); return (bool*)data; }
bool* operator=(bool*val);//{data=(BYTE*)val; return val;}
bool operator[](int num);//{return 0!=(data[num/8]&(1 << (num%8)));}
//inline bool operator[](int num, int s){return 0!=(data[num/8]&(1 << (num%8)));}
bool operator[](unsigned int num);//{return 0!=(data[num/8]&(1 << (num%8)));}
bool get(unsigned int num);//{return 0!=(data[num/8]&(1 << (num%8)));}
bool operator[](unsigned short num);//{return 0!=(data[num/8]&(1 << (num%8)));}
operator bool*();//{return (bool*)data;}
void set(int num, bool val);//{
	//data[num/8]|=(1 << (num%8));
	//data[num/8]^=(1 << (num%8));
	//data[num/8]|=val*(1 << (num%8));
//	if(!val) data[num/8]&=~(val*(1 << (num%8)));
//	else data[num/8]|=val*(1 << (num%8));
//	print("Set "+itos(num)+"="+itos(val)+" e:"+itos(data[num/8]&(1 << (num%8))));
//	return ;}
void i(int num, bool val);//{ set(num, val); return ; }
bool i(int num);//{ return 0!=(data[num/8]&(1 << (num%8))); }
bool is(unsigned int size);//{ size=size/8 + (size%8>0 ? 1 : 0); if(size==sz) return 1; return 0;}
void Null();//{memset(data, 0, sz); }
BYTE isbyte(const unsigned int b);//{return *(data+b);}
};


mbool::mbool(){data=0; sz=0;}
mbool::mbool(void*val){data=(BYTE*)val; sz=0;}
void mbool::Del(){_deln(data);}
bool* mbool::New(unsigned int size){ sz=size/8 + (size%8>0 ? 1 : 0); data=(BYTE*)malloc(sz); return (bool*)data; }
bool* mbool::operator=(bool*val){data=(BYTE*)val; return val;}
bool mbool::operator[](int num){return 0!=(data[num/8]&(1 << (num%8)));}
//inline bool operator[](int num, int s){return 0!=(data[num/8]&(1 << (num%8)));}
bool mbool::operator[](unsigned int num){return 0!=(data[num/8]&(1 << (num%8)));}
bool mbool::get(unsigned int num){return 0!=(data[num/8]&(1 << (num%8)));}
bool mbool::operator[](unsigned short num){return 0!=(data[num/8]&(1 << (num%8)));}
mbool::operator bool*(){return (bool*)data;}
void mbool::set(int num, bool val){
	if(!val) data[num/8]&=~(1 << (num%8));
	else data[num/8]|=1 << (num%8);
	return ;}
	//data[num/8]|=(1 << (num%8));
	//data[num/8]^=(1 << (num%8));
	//data[num/8]|=val*(1 << (num%8));
//	print("Set "+itos(num)+"="+itos(val)+" e:"+itos(data[num/8]&(1 << (num%8))));
void mbool::i(int num, bool val){ set(num, val); return ; }
bool mbool::i(int num){ return 0!=(data[num/8]&(1 << (num%8))); }
bool mbool::is(unsigned int size){ size=size/8 + (size%8>0 ? 1 : 0); if(size==sz) return 1; return 0;}
void mbool::Null(){memset(data, 0, sz); }
BYTE mbool::isbyte(const unsigned int b){return *(data+b);}



// Templates
template <class OMatrix>
class OMatrixT{ public:
OMatrix *_a, *_e; // +data

OMatrixT(){ _a=0; _e=0; }

	// IF _p as _a && _n as _e;
	bool OMAdd(OMatrix*el){
		if(!_a){ _a=el; _e=el; el->_p=0; el->_n=0; return 1;}
		//_e->_n=el; el->_p=_e; el->_n=0; _e=el; return 1;
		el->_n=0; el->_p=_e; _e->_n=el; _e=el; return 1;
	}

	static bool OMAddP(OMatrix *&_a, OMatrix *&_e, OMatrix*el){
		if(!_a){ _a=el; _e=el; el->_p=0; el->_n=0; return 1;}
		//_e->_n=el; el->_p=_e; el->_n=0; _e=el; return 1;
		el->_n=0; el->_p=_e; _e->_n=el; _e=el; return 1;
	}

	bool OMDel(OMatrix*el){
		if(el->_n) el->_n->_p=el->_p; else if(el==_e) _e=el->_p;
		if(el->_p) el->_p->_n=el->_n; else if(el==_a) _a=el->_n;
		return 1;
	}

	static bool OMDelP(OMatrix *&_a, OMatrix *&_e, OMatrix *el){
		if(el->_n) el->_n->_p=el->_p; else if(el==_e) _e=el->_p;
		if(el->_p) el->_p->_n=el->_n; else if(el==_a) _a=el->_n;
		return 1;
	}

	bool OMAddEx(OMatrix*p, OMatrix*el){
		if(!_a){ _a=el; _e=el; el->_p=0; el->_n=0; return 1;}

		if(!p){
			el->_p=0; el->_n=_a; _a->_p=el; _a=el;
			//if(_a==_e) _e=el; its fail
			return 1;
		}

		el->_p=p; el->_n=p->_n;
		p->_n=el; if(el->_n) el->_n->_p=el; else _e=el;
		return 1;
	}

	static bool OMAddExP(OMatrix *&_a, OMatrix *&_e, OMatrix*p, OMatrix*el){
		if(!_a){ _a=el; _e=el; el->_p=0; el->_n=0; return 1; }

		if(!p){
			el->_p=0; el->_n=_a; _a->_p=el; _a=el;
			//if(_a==_e) _e=el; its fail
			return 1;
		}

		el->_p=p; el->_n=p->_n;
		p->_n=el; if(el->_n) el->_n->_p=el; else _e=el;
		return 1;
	}

	OMatrix *OMNew(bool r=1){
		OMatrix *p=new OMatrix;
		if(r && p) OMAdd(p);
		return p;
	}

	void OMRealDel(OMatrix *d){
		OMatrix *p=_a;
		while(p){
			if(p==d){ OMDel(p); delete p; return ; }
			p=p->_n;
		}
		return ;
	}

	void OMClear(bool r=1){
		OMatrix *p=_a, *d=p;
		while(p){
			d=p; p=p->_n; OMDel(d); if(r) delete d;
		}
		 _a=0; _e=0;
	}

	void OMClearS(){ _a=0; _e=0; }
};


#undef X()