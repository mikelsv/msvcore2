// String to Int
int64 stoi64(VString val, int radix){
	return stoi64((char*)val.data, val.sz, radix);
}

int stoih(const VString &val){
	return stoi(val, val, 16);
}

int stoi(char *line, unsigned int size, int radix){
	return (int)stoi64(line, size, radix);
}

unsigned int stoui(char *line, unsigned int size, int radix){
	return (unsigned int)stoi64(line, size, radix);
}

int64 stoi64(char*line, unsigned int size, int radix){
int64 ret=0; if(!line) return 0; if(!size) size=(unsigned int)strlen(line);
if(!size) return 0;
char *to=line+size; bool min=0; int t;

while(line<to){
if(*line=='-'){min=1; line++; break;}
if(*line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z') break;
line++;
}
while(line<to){
	if(*line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z'){
		t=((*line>='0' && *line<='9') ? *line-48 : (*line>='a' && *line<='z') ? *line-'a'+10 : *line-'A'+10 );
		if(t>=radix) break;
		ret=ret*radix+t;
	}
else break; line++;
}

if(min) ret*=-1;
return ret;
}



double stod(VString val, int radix){
	return stod((char*)val.data, val.sz, radix);
}

double stod(char*line, unsigned int size, int radix){
	char *ln = line, *to = line + size;

	for(ln; ln < to; ln++){
		if(*ln == '.')
			break;
	}

	double ret = (double)stoi64(line, ln - line, radix);
	
	if(ln + 1 < to){
		double pw = pow((double)radix, (int)(to - ln - 1));
		double dret = (double)stoi64(ln + 1, to - ln - 1, radix);
		ret = ret + dret / pw;
	}

	return ret;

#ifdef NOOOOOO
double ret=0; if(!line) return 0; if(!size) size=(unsigned int)strlen(line);
if(!size) return 0;
char *to=line+size; bool min=0, dd=0; int d=0;

while(line<to){
if(*line=='-'){min=1; line++; break;}
if(*line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z') break;
line++;
}
while(line<to){ if(dd) d++;
	if(*line=='.'){ dd=1; }
	else
	if(*line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z')
		ret=ret*radix+((*line>='0' && *line<='9') ? *line-48 : (*line>='a' && *line<='z') ? *line-'a'+10 : *line-'A'+10 );
else break; line++;
}

while(d){ ret/=radix; d--;}

if(min) ret*=-1;
return ret;
#endif
}

TString itos(int64 val, int radix, int null){
	TString ret; ret.Reserve(prmf_itos(0, 0, val, radix, null)); prmf_itos(ret, ret, val, radix, null); return ret;
}

TString htob(VString line){
	TString ret; ret.Reserve(prmf_htob(0, 0, line, line)); prmf_htob(ret, ret, line, line); return ret;
}

TString btoh(VString line){
	TString ret; ret.Reserve(prmf_btoh(0, 0, line, line)); prmf_btoh(ret, ret, line, line); return ret;
}

TString itob(unsigned int val, int null){ // recreate it
	TString ret;
	int stp = 1, tmp;

	while(val > 0){
		tmp = val % 256;
		val /= 256;
		ret += VString((char*)&tmp, 1);	//pows -1
	}
	if(null > 0 && (unsigned int)null > ret.size()){
		TString s;
		s.RClean(null - ret.size(), 0);
		ret += s;
	}
	return ret;
}


// Replace
TString Replace(VString line, VString fr, VString to, unsigned int cnt){
	TString ret;
	int s=prmf_replace(0, 0, line, line, fr, fr, to, to, cnt);
	ret.Reserve(s); s=prmf_replace(ret, ret, line, line, fr, fr, to, to, cnt);
	return ret;
}


// SHORT to CHAR
unsigned char stoc(const unsigned short s){
	if(s>1039 && s<1104) return char(s-1040+192);
	if(s==1025) return 168; if(s==1105) return 184; // ¨, ¸.
	return (unsigned char)s;
} // short to char

TString stoc(const unsigned short *s, const int sz){	// short line to MString
	TString ret; ret.Reserve(sz); char *r=ret;
	for(int i=0; i<sz; i++){
		*r=stoc(*s); s++; r++;
	}
	return ret;
}

// CHAR to SHORT
unsigned short ctos(const unsigned char s){
	if(s>=192/* && s<255*/) return (unsigned short)s+1040-192; // if(v>='à' && v<'ÿ') return v-'à'+32+1040;
	if(s==168) return 1025; if(s==184) return 1105; // ¨, ¸
	return s;
} // char to short

TString ctos(const unsigned char*s, const int sz){	// short line to MString
	TString ret; ret.Reserve(sz*2); char *r=ret;
	for(int i=0; i<sz; i++){
		*(unsigned short*)r=ctos(*s); s++; r+=2;
	}
	return ret;
}

TString ctos(VString line){ return ctos(line, line); }