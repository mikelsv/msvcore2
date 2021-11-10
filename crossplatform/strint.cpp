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
	char *ln = line, *to = line + size, m = 1;

	if(ln < to && *line == '-')
		m = -1;

	for(ln; ln < to; ln++){
		if(*ln == '.')
			break;
	}

	double ret = (ln - line) > 0 ? (double)stoi64(line, ln - line, radix) : 0;
	
	if(ln + 1 < to){
		double pw = pow((double)radix, (int)(to - ln - 1));
		double dret = (double)stoi64(ln + 1, to - ln - 1, radix);
		ret = ret + dret / pw * m;
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


// strtod.c strtod() + string size
double stod_e(const char *str, char *end){
	double number;
	int exponent;
	int negative;
	char *p = (char*) str;
	double p10;
	int n;
	int num_digits;
	int num_decimals;

	// Skip leading whitespace
	while(p < end && isspace(*p))
		p ++;

	// Handle optional sign
	negative = 0;

	if(p < end)
		switch(*p){
			case '-': negative = 1; // Fall through to increment position
			case '+': p ++;
		}

	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;

	// Process string of digits
	while(p < end && isdigit(*p)) {
		number = number * 10. + (*p - '0');
		p ++;
		num_digits ++;
	}

  // Process decimal part
	if(p < end && *p == '.'){
		p ++;

		while(p < end && isdigit(*p)){
			number = number * 10. + (*p - '0');
			p ++;
			num_digits ++;
			num_decimals ++;
		}

		exponent -= num_decimals;
	}

	if(num_digits == 0) {
		errno = ERANGE;
		return 0.0;
	}

	// Correct for sign
	if(negative)
		number = -number;

	// Process an exponent string
	if(p < end && (*p == 'e' || *p == 'E')){
		// Handle optional sign
		negative = 0;
		switch(*++p){
			case '-': negative = 1;   // Fall through to increment pos
			case '+': p++;
		}

		// Process string of digits
		n = 0;
		while(p < end && isdigit(*p)){
			n = n * 10 + (*p - '0');
			p ++;
		}

		if(negative)
			exponent -= n;
		else
			exponent += n;
	}

#define DBL_MIN_EXP     (-1021)                 /* min binary exponent */
#define DBL_MAX_EXP     1024                    /* max binary exponent */

	if(exponent < DBL_MIN_EXP || exponent > DBL_MAX_EXP){
		errno = ERANGE;
		return HUGE_VAL;
	}

  // Scale the result
	p10 = 10.;
	n = exponent;
	if(n < 0)
		n = -n;

	while(n){
		if(n & 1){
			if(exponent < 0)
				number /= p10;
			else
				number *= p10;
		}

		n >>= 1;
		p10 *= p10;
	}

	// Return
	if (number == HUGE_VAL)
		errno = ERANGE;

	return number;
}


TString itos(int64 val, int radix, int null){
	TString ret; ret.Reserve(prmf_itos(0, 0, val, radix, null)); prmf_itos(ret, ret, val, radix, null); return ret;
}

TString dtos(double val, int ml){
	TString ret; ret.Reserve(prmf_dtos(0, 0, val, ml)); prmf_dtos(ret, ret, val, ml); return ret;
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

int itossz(int64 val){
	//if(!val) return 0;
	if(val<10) return 1;
	if(val<100) return 2;
	if(val<1000) return 3;
	if(val<10000) return 4;
	if(val<100000) return 5;
	if(val<1000000) return 6;
	if(val<10000000) return 7;
	if(val<100000000) return 8;
	if(val<1000000000) return 9;
	if(val<=10000000000ull) return 10;
	if(val<=100000000000ull) return 11;
	if(val<=1000000000000ull) return 12;
	// ...
	return 13;
}

int bsize16(int64 val){
	if(!val) return 0;

	int i=0;
	
	while(val){
		val/=16;
		i++;
	}	

return i;
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

// Short to utf
TString stoutf(unsigned short *ln, unsigned int sz){
	TString ret;
	int s=prmf_stoutf(0, 0, ln, sz);
	ret.Reserve(s); s=prmf_stoutf(ret, ret, ln, sz);
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


TString utftoc(const VString ln, int sys = 0){ // if(sys==4) return utftotr(ln);
	unsigned char *to=ln.uchar()+ln.size(), *line=ln.uchar(); unsigned int r; int lss=0;
	for(line; line<to; line++){
		if(*line<128) lss++;
		else if((*line&224)==192){ lss++; line++; } // twobyte
		else if((*line&240)==224){ line+=2; } // three byte
		else if((*line&248)==240){ line+=3; } // four byte
	}

	TString _ls; _ls.Reserve(lss); char *ls=_ls; to=ln.uchar()+ln.size(); line=ln.uchar(); lss=0; 
	for(line; line<to; line++){
		if(*line<128){ *(ls+lss)=*line; lss++; }
		else if((*line&224)==192){
			r=((*line&31)<<6)+(*(line+1)&63);
			//if(!sys)
			*(ls+lss)=stoc(r);
			//else if(sys==1) *(ls+lss)=stocdos(r);
			//else *(ls+lss)=stoclin(r);			
			line++; lss++;
		} // twobyte
		else if((*line&240)==224){ line+=2; } // three byte
		else if((*line&248)==240){ line+=3; } // four byte
	}
	return _ls;
}

int is_num(VString line){
	unsigned char *ln = line, *to = line.endu();
	if(ln < to && *ln == '-')
		ln ++;

	while(ln < to){
		if(*ln < 48 || *ln > 57)
			return 0;

		ln ++;
	}

	return 1;
}