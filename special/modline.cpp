#define MODLINE_SAFE_SQL	1

#define SafeSQL(val) ModLine(val, MODLINE_SAFE_SQL)
//#define SafeSQL(val) ModLine(val, MODLINE_SAFE_SQL)

class ModLine : public SStringX<S4K>{

public:
	static int TestMail(VString line){
		unsigned char *ln = line, *to = line.endu();

		while(ln < to){
			if(!(*ln >= 'a' && *ln <= 'z' || *ln >= 'A' && *ln <= 'Z' || *ln >= '0' && *ln <= '9' || *ln == '.' || *ln == '@'))
				return 0;

			ln ++;
		}

		return 1;
	}

	MString XmppMsg(VString line){
		MString ret;
		ret.Reserve(line.sz * 3);
		
		unsigned char *ln = line, *to = line.endu(), *r = ret;
		while(ln<to){
			if(*ln >= 'a' && *ln <= 'z' || *ln >= 'A' && *ln <= 'Z' || *ln >= '0' && *ln <= '9' || *ln == '.' || *ln == '@')
				*r++ = *ln;
			else{
				*r++ ='%';
				*r++ = cb16[(*ln)/16];
				*r++ = cb16[(*ln)%16];			
			}

			ln ++;
		}

		return ret.str(0, r - ret.data);
	}

	int ctoi(char c){
		return (c >= '0' && c <= '9') ? c - 48 : (c >= 'a' && c <= 'z') ? c - 'a' + 10 : c - 'A' + 10;
	}

	MString TgToXmppMsg(VString line){
		SString ret;
		ret.Reserve(line.sz * 4);
		
		unsigned char *ln = line, *to = line.endu(), *r = ret;
		while(ln<to){
			if(ln + 6 <= to && *ln == '\\' && *(ln + 1) == 'u'
				&& isnumdh(*(ln + 2)) && isnumdh(*(ln + 3)) && isnumdh(*(ln + 4)) && isnumdh(*(ln + 5))){
					//memcpy(r, ln, 6);
					
					unsigned int utf = (ctoi(*(ln + 2)) << 16) + (ctoi(*(ln + 3)) << 8) + (ctoi(*(ln + 4)) << 4) + ctoi(*(ln + 5));
					
					if(utf < 0x80)
						*r ++ = utf;
					else if(utf < 0x800){
						*r ++ = 192 + (utf >> 6);
						*r ++ = 128 + (utf & 63);
					}
					else if(utf < 0x10000){
						*r ++ = 224 + (utf >> 12);
						*r ++ = 128 + ((utf >> 6) & 63);
						*r ++ = 128 + (utf & 63);
					}
					else if(utf <= 0x1fffff){
						*r ++ = 240 + (utf >> 24);
						*r ++ = 128 + ((utf >> 12) & 63);
						*r ++ = 128 + ((utf >> 6) & 63);
						*r ++ = 128 + (utf & 63);
					}

					ln += 5;
					//r += 5;
			} else if(*ln >= 'a' && *ln <= 'z' || *ln >= 'A' && *ln <= 'Z' || *ln != 60 && *ln != 62 && *ln >= 32 && *ln < 126)
				*r++ = *ln;			
			else{
				*r++ ='%';
				*r++ = cb16[(*ln)/16];
				*r++ = cb16[(*ln)%16];			
			}

			ln ++;
		}

		return ret.str(0, r - ret.data);
	}

};

/*

class ModLine_old{
protected:
	unsigned char sdata[S2K];
	MString data;
	VString ret;

public:

	ModLine_old(ModLine &val){
		if(val.ret.data == val.sdata){
			memcpy(sdata, sdata, val.ret.sz);
			ret.data = sdata;
			ret.sz = val.ret.sz;
		} else{
			data = val.data;
			ret = data;
		}

		return ;
	}

	ModLine(VString line, int op){
		if(op == MODLINE_SAFE_SQL)
			ModLineSafeSQL(line);
		//else if(op == MODLINE_SAFE_CGI)
		//	ModLineSafeCGI(line);
	}

	ModLine& ModLineSafeSQL(VString line){
		unsigned char *ln = line, *lln = ln, *to = line.endu();
		int cnt = 0;

		for(ln; ln < to; ln++)
			if(*ln == '\'')
				cnt++;

		if(!Reserv(line.sz + cnt))
			return *this;

		unsigned char *tn = ret;
		ln = lln;

		for(ln; ln < to; ln++){
			if(*ln == '\''){
				memcpy(tn, lln, ln-lln);
				tn += ln-lln;
				*tn++ = '\\';
				lln = ln;
			}
		}

		if(lln != ln){
			memcpy(tn, lln, ln-lln);
		}

		return *this;
	}

	int Reserv(int sz){
		if(sz <= sizeof(sdata))
			ret.setu(sdata, sz);
		else
			ret = data.Reserv(sz);

		if(ret.sz != sz)
			return 0;

		return 1;		
	}

	static int TestText(VString text){
		unsigned char *ln = text, *to = text.endu();
		while(ln < to){
			if(ln < ' ')
				return 0;
			ln ++;
		}

		return 1;
	}

	operator VString(){
		return ret;
	}

};
*/