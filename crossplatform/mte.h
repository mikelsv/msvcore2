// New Mtev
#define MTEVAT_INT		1
#define MTEVAT_FLOAT	2
#define MTEVAT_STR		3

class MTEVA{ public:
	unsigned int tp, sz;
	union{ int64 i; double f; MACADDR_S m; }; VString v;

	// Insert	
	MTEVA(){ tp=0; sz=0; i=0; }

	MTEVA(unsigned char v){ i=v; sz=1; tp=MTEVAT_INT; }
	MTEVA(unsigned short v){ i=v; sz=2; tp=MTEVAT_INT; }
	MTEVA(short v){ i=v; sz=2; tp=MTEVAT_INT; }
	MTEVA(unsigned int v){ i=v; sz=4; tp=MTEVAT_INT; }
	MTEVA(int v){ i=v; sz=4; tp=MTEVAT_INT; }
	MTEVA(int64 v){ i=v; sz=8; tp=MTEVAT_INT; }
	MTEVA(uint64 v){ i=v; sz=8; tp=MTEVAT_INT; }
	MTEVA(unsigned long v){ i=v; sz=sizeof(unsigned long); tp=MTEVAT_INT; }
	MTEVA(float v){ f=v; sz=4; tp=MTEVAT_FLOAT; }
	MTEVA(double v){ f=v; sz=8; tp=MTEVAT_FLOAT; }
	//MTEVA(VString s){ v=s; sz=8; tp=MTEVAT_STR; }
	MTEVA(const VString s){ v=s; sz=8; tp=MTEVAT_STR; }
	MTEVA(const char* s){ v=VString(s); sz=8; tp=MTEVAT_STR; }
	MTEVA(MACADDR_S s){ i=0; memcpy(&m, &s, 6); sz=8; tp=MTEVAT_INT; }
	MTEVA(IP6_ADDRESS &s){ v.data=(unsigned char*)&s; sz=16; tp=MTEVAT_STR; }

	// Return 
	operator unsigned int(){ return (unsigned int)(int64)*this; }
	operator int64(){
		switch(tp){
			case MTEVAT_INT: return i; break;
			case MTEVAT_FLOAT: return (int64)f; break;
			case MTEVAT_STR: return stoi64(v);
			default : return 0;
		}
	}

	operator double(){
		switch(tp){
			case MTEVAT_INT: return (double)i; break;
			case MTEVAT_FLOAT: return f; break;
			case MTEVAT_STR: return stod(v);
			default : return 0;
		}
	}

	operator VString(){
		switch(tp){
			case MTEVAT_INT: return VString(); break;
			case MTEVAT_FLOAT: return VString(); break;
			case MTEVAT_STR: return v;
			default : return VString();
		}
	}

	operator IP6_ADDRESS(){
		if(tp==MTEVAT_STR && sz==16){ return *(IP6_ADDRESS*)v.data; }
		IP6_ADDRESS ip6; memset(&ip6, 0,16); return ip6;
	}

	//operator MACADDR_S(){
	//	if(tp==MTEVAT_INT && sz==7){ return *(MACADDR_S*)&i; }
	//	return MACADDR_S();
	//}

};