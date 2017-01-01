// Base 64 help functions
BYTE cbgetpos(unsigned char c){ if(c>='A' && c<='Z') return c-'A';
	if(c>='a' && c<='z') return c-'a'+26; if(c>='0' && c<='9') return c-'0'+52;
	if(c=='+') return 62; if(c=='/') return 63; return 0;
}

BYTE cbgetpost(unsigned char c){ if(c>='A' && c<='Z') return 1;
	if(c>='a' && c<='z') return 1; if(c>='0' && c<='9') return 1;
	if(c=='+') return 1; if(c=='/') return 1; return 0;
}

// Base 64
TString Base64::btos(const VString line){
	TString ret; ret.Reserve(line.size()*6/8+2, 0);
	unsigned char*ln=line.uchar(), *to=ln+line.size(), *r=ret.uchar(); int i=0; unsigned short s=0;
	while(to>ln && *(to-1)=='=') to--;

	while(ln<to){
		//*((short*)(r+i/8))+= GetBpos(*ln) << (i%8); i+=6; ln++;
		s+=cbgetpos(*ln) << (( (i%8<=6) ? 8-6-(i%8) : 0)+8);
		if(i%8+6>=8) {*(r+i/8)=*((char*)&s+1); s=s<<8;} i+=6; ln++;
	}
	ret.sz = i / 8;
	//ret.resize(i/8);
	return ret;
}

TString Base64::stob(const VString line){
	TString ret; ret.Reserve(line.size()*8/6+3, 0);
	unsigned char*l=line.uchar(), *ln=l, *to=ln+line.size(), *r=ret.uchar(); int i=0; unsigned short s=0, t;

	while(ln<to){
		//*r+= ((*(unsigned short*)ln) >> (i%8))&63;
		//s+= (((*(unsigned short*)ln) >> (i%8))&63) << ( (i%8<=6) ? 8-6-(i%8) : 0)+8;

		*((char*)&t+1)=*ln; *((char*)&t)=*(ln+1);
		*r= (t >> (8 + ((i%8<=6) ? 8-6-(i%8) : 0))&63); //&(63<<8) ;//<< ( +8
		//*r=*((char*)&s+1); s=s<<8;
		if(i%8+6>=8) ln++; r++; i+=6;
	} //if(i%8) r--; if(i%8) to++;

	to=r; r=ret.uchar();
	while(r<to){ *r=cb64[*r]; r++; }
	if(i%8==4) {*r='='; r++; *r='='; r++; } if(i%8==2) {*r='='; r++;}

	//ret.resize(r-ret.uchar());
	ret.sz = r-ret.uchar();
	return ret;
}

TString Base64::mbtos(const VString line){
TString ret; ret.Reserve(line.size()*6/8+2, 0);
unsigned char*ln=line.uchar(), *to=ln+line.size(), *r=ret.uchar(); int i=0;
		
while(ln<to){
	*((short*)(r+i/8))+=cbgetpos(*ln) << (i%8); i+=6; ln++;
}
//ret.resize(i/8);
ret.sz = i / 8;
return ret;
}

TString Base64::mstob(const VString line){
TString ret; ret.Reserve(line.size()*8/6+3, 0);
unsigned char*l=line.uchar(), *ln=l, *to=ln+line.size(), *r=ret.uchar(); int i=0;

	while(ln<to){
		*r+= ((*(unsigned short*)ln) >> (i%8))&63;
		if(i%8+6>=8) ln++; r++; i+=6;
	}

	to=r; r=ret.uchar(); while(r<to){ *r=cb64[*r]; r++; }
	//ret.resize(r-ret.uchar());
	ret.sz = r-ret.uchar();
	return ret;
}



// MD5 - 6.12.05 1.0.0 MSVSPL Specila Library
//TString md5(VString line);
//TString md5h(VString line);
//TString md5b(VString line);

// md5
#include "../crypto/md5.c"
#include "../crypto/rmd160.c"
#include "../crypto/sha1.c"
#include "../crypto/sha224-256.c"

TString md5(VString line){
	md5_state_t state;
	TString ret;
	ret.Reserve(16);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)line.rchar(), line.size());
	md5_finish(&state, (md5_byte_t*)ret.rchar());
	
	return ret;
}
		   
TString md5h(VString line){
	md5_state_t state;
	TString ret;
	ret.Reserve(32);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)line.rchar(), line.size());
	md5_finish(&state, (md5_byte_t*)ret.rchar() + 16);

	unsigned char * ln = ret.uchar() + 16, *to = ret.endu(), *r = ret;
	while(ln < to){
		*r++ = (*ln / 16) < 10 ? (*ln / 16) + '0' : (*ln / 16) + 'a' - 10;
		*r++ = (*ln % 16) < 10 ? (*ln % 16) + '0' : (*ln % 16) + 'a' - 10;
		ln ++;
	}
	
	return ret;
}


	MHash::MHash(int t){ type=t; Init(); }
	int MHash::Type(int t){ type=t; Init(); return type;  }
	int MHash::Type(){ return type; }

	void MHash::Init(){ int ad=0;
#ifndef USEMSV_MHASHT_GETALL		
	int t=type; while(t){ if(t&1 && t>1){ globalerror("#define USEMSV_MHASHT_GETALL or this is you problem!"); break; } t>>=1; }
#endif

		if(type&MHASHT_MD5){ md5_init(&md5); ad=16; }
		if(type&MHASHT_RMD160){ RMD160Init(&rmd); ad=20; }
		if(type&MHASHT_SHA1){ SHA1Reset(&sha1); ad=SHA1HashSize; }
		if(type&MHASHT_SHA256){ SHA256Reset(&sha256); ad=SHA256HashSize; }
		
		if(ad){ ret.setu(data, ad); reth.setu(data+ad, ad*2); }
	}

	void MHash::Add(VString line){
		if(type&MHASHT_MD5) md5_append(&md5, line, line.size());
		if(type&MHASHT_RMD160) RMD160Update(&rmd, line, line.size());
		if(type&MHASHT_SHA1) SHA1Input(&sha1, line, line.size());
		if(type&MHASHT_SHA256) SHA256Input(&sha256, line, line.size());
	}

	void MHash::Finish(int t){
		type=t; Finish();
	}

	void MHash::Finish(){
		if(type==MHASHT_MD5){ md5_finish(&md5, ret); ret.sz=16;  }
		if(type==MHASHT_RMD160){ RMD160Final(ret, &rmd); ret.sz=20; }
		if(type==MHASHT_SHA1){ SHA1Result(&sha1, ret); ret.sz=20; }
		if(type==MHASHT_SHA256){ SHA256Result(&sha256, ret); ret.sz=32; }

		reth.sz=ret.sz*2;
		prmf_btoh(reth, reth, ret, ret);	
	}

	VString MHash::Get(){ return ret; }
	VString MHash::GetH(){ return reth; }

	VString MHash::FastH(int type, VString line){
		Type(type);
		Add(line);
		Finish();
		return GetH();
	}


