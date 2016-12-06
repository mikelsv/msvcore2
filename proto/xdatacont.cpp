// Don't include! use #define USEMSV_XDATACONT 

#define USEMSV_XDATACONT_USED
//#define XMLIT

//#ifndef USEMSV_DATIXCONT_USED
//#error Please define: #define USEMSV_DATIXCONT
//#endif

class XMLel{ public:
VString key;
VString val;
#ifndef XMLIT // XML int type
XMLel *r, *p, *n, *a, *l; // special, prev, next, a(up), line(up).
#define XMLN	0
#else
short r, p, n, a, l;
#define XMLN	-1 // -1
#endif

#ifndef XMLIT // CFL int type
inline XMLel* get(XMLel *cf) { return cf; }
inline XMLel* set(XMLel *cf) { return cf; }
#else
inline XMLel * get(XMLel * cf){ return cf; }
inline unsigned int ps(int cf){ return cf; }
#endif

short type; // text, tag

	XMLel* FindX(XMLel*el, VString line){
		for(el; el; el=get(el->n)){
			if(el->key==line) return el;
		}
	return 0;
	}

	XMLel* Get(VString line){
		XMLel *el=this; if(!this) return 0;
		VString o, t; int l=0;

		while(el && line.is()){
			o=PartLineO(line, "."); t=PartLineO(line, ",");
			if(o.data && o.sz<t.sz){ o=PartLine(line, line, "."); l=0; } else{ o=PartLine(line, line, ","); l=1; }

			el=el->FindX(el, o);

			if(el != 0 && line)
				el = !l ? el->a : el->l;

			if(line.sz==1 && *line.data==':') return el;
		}
		return el;
	}

	VString GetVal(VString line){ XMLel *el=Get(line); if(el) return el->val; return VString(); }

	XMLel* fa(){ if(this) return a; return 0; }

/*
	XMLel * operator[](VString line){
		XMLel *el=this; if(!this) return 0;
		el=el->a;
		while(el && line.is()){
			VString o=PartLine(line, line, ".");
			el=el->FindX(el, o);
			if(el && line) el=el->a; if(line.sz==1 && *line.data==':') return el;
		}
		return el;
	}
	*/

};


XMLel* XMLGet(VString line, XMLel *el){
	if(!el) return 0;
	while(el && line.is()){
		VString o=PartLine(line, line, ".");
		el=el->FindX(el, o);
		if(el) el=el->a;
	}
	return el;
}

/*
void XMLtoCont(XMLel *el, Cont &ct){
	if(!el) return ;
	//el=el->a;
	
	while(el){
		ct.Add(el->key, el->a ? el->a->val : "");
		el=el->n;
	}
	//ct.Read(ct.ls);
	ct.Read(VString(ct.ls.oneline(), ct.ls.size()));
	return ;
}*/


// eXtended Data Element
class XDataEl{
public:
	XDataEl *_p, *_n, *_a, *_e, *_l;
	VString key, val;

	XDataEl* n()const{ if(this) return _n; return 0; }
	XDataEl* a()const{ if(this) return _a; return 0; }

	VString k()const{ if(this) return key; return VString(); }
	VString v()const{ if(this) return val; return VString(); }

	VString GetVal(VString line){
		XDataEl*el=Get(line);
		if(el) return el->val;
		return VString();
	}

	VString GetLVal(VString line){
		XDataEl*el=GetL(line);
		if(el) return el->val;
		return VString();
	}

	XDataEl* GetByPos(int pos){
		XDataEl*el=this; if(el) el=el->_a; if(!el) return 0;
		while(el && pos){
			el=el->n(); pos--;
		}

		return el;
	}

	XDataEl* GetByKey(VString key, VString val){
		XDataEl*el=this; if(el) el=el->_a; if(!el) return 0;
		while(el){
			if(el->GetVal(key)==val)
				return el;
			el=el->n();
		}

		return el;
	}


	// find Find(_a-> ->_n ... )
	XDataEl* Get(VString line){
		XDataEl *el=this; if(el) el=el->_a; if(!el) return 0;
		VString o, t; int l=0;

		while(el && line.is()){
			o=PartLineO(line, "."); t=PartLineO(line, ",");
			if(o && o.sz<t.sz){ o=PartLine(line, line, "."); l=0; } else{ o=PartLine(line, line, ","); l=1; }

			if(o.sz==1 && *o.data==':'){
				el=el->a();
				continue;
			}

			el=el->FindX(el, o);
			if(el != 0 && line) el= !l ? el->_a : el->_l;
			if(line.sz==1 && *line.data==':') return el;
		}
		return el;
	}

	XDataEl* FindX(XDataEl*el, VString line){
		for(el; el; el=el->_n){
			if(el->key==line) return el;
		}
		return 0;
	}

	XDataEl* GetL(VString line){
		if(!this)
			return 0;

		XDataEl*el;
		for(el=_l; el; el=el->_n){
			if(el->key==line)
				return el;
		}
		return 0;
	}

	int GetSize()const{
		int sz=0;
		for(XDataEl *el=_a; el; el=el->_n)
			sz++;

	return sz;
	}

	void AddToList(XDataEl *el){
		if(!_a){
			_a = el;
			_e = el;
			el->_p = 0;
			el->_n = 0;
		} else{
			_e->_n = el;
			el->_p = _e;
			_e = el;
			el->_n = 0;
		}

		return ;
	}

};

class XDataPEl{
	XDataEl *el;

public:

	XDataPEl(){
		el=0;
	}

	XDataPEl(XDataEl *e){
		el=e;
	}

	XDataPEl p(){
		if(!el)
			return 0;
		return XDataPEl(el->_p);
	}

	XDataPEl n(){
		if(!el)
			return 0;
		return XDataPEl(el->_n);
	}

	XDataEl* a(){
		if(!el)
			return 0;
		return el->_a;
	}

	operator bool(){
		return el!=0;
	}

	operator XDataEl*(){
		return el;
	}

	VString key(){
		if(!el)
			return VString();
		return el->key;
	}

	VString val(){
		if(!el)
			return VString();
		return el->val;
	}

	int Size(){
		if(el)
			return el->GetSize();
		return 0;
	}

	VString operator [](const VString key){
		if(!el)
			return VString();
		return el->GetVal(key);
	}

	VString operator [](const char *key){
		if(!el)
			return VString();
		return el->GetVal(key);
	}

	XDataEl* GetByPos(const int pos){
		return el->GetByPos(pos);
	}

	XDataPEl GetByKey(const VString key, VString val){
		return el->GetByKey(key, val);
	}

	XDataEl* operator ()(const VString key){
		if(!el)
			return 0;
		return el->Get(key);
	}


};



// eXtended Data Container
class XDataCont{
	//XDataLine data;
	UList<XDataEl, 0, 1, S16K> data;

	int sz;

public:
	XDataCont(){ sz=0; }
	~XDataCont(){ Clear(); }

	XDataCont(VString line){
		sz=0;
		Read(line);
		return ;
	}

	// Get Data
	operator XDataEl*(){ if(data.Size()) return data.Get(0); return 0; }
	operator XDataPEl(){ if(data.Size()) return data.Get(0); return 0; }
	XDataEl* GetData(){ if(data.Size()) return data.Get(0); return 0; }

	// GetValue
	VString GetValue(VString line){ return GetData()->GetVal(line); }
	XDataEl* Get(VString line){ return GetData()->Get(line); }
	VString operator[](VString line){ return GetData()->GetVal(line); }

	XDataEl* operator ()(const VString key){
		return Get(key);
	}


	void Release(){ Clear(); return ; }
	
	// Read
	int Read(VString data, bool noclear=0){
		if(data[0]=='{' || data[0]=='[')
			return ReadJson(data, noclear);

		if(data[0]=='<')
			return ReadXML(data, noclear);

		Clear();
		return 0;
	}


	// Json
	int ReadJson(VString data, bool noclear=0){
		return ReadJson(data, data, noclear);
	}

	int ReadJson(unsigned char*data, unsigned int size, bool noclear=0){
		if(!noclear){ Clear(); }

		ReadJsonDo(0, data, data+size);

		return 1;
	}

	void ReadJsonDo(XDataEl*parent, unsigned char*&line, unsigned char*to){
		VString k, v; unsigned char *lline=line;
		XDataEl*fel=0, *eel=0, *el; int sh;

		while(line<to){

			// data "key" "val"
			if(*line=='"'){
				lline=++line; sh=0;
				while(line<to){
					if(*line=='"')
						break;
					if(*(line)=='\\'){						
						if(line+1<to)
							line++;
						sh=1;
					}
					line++;
				}
				v.setu(lline, line-lline);
				line++; continue;
			}
			else if(*line=='\''){
				lline=++line;  sh=0;
				while(line<to){
					if(*line=='\'')
						break;
					if(*(line)=='\\'){						
						if(line+1<to)
							line++;
						sh=1;
					}
					line++;
				}
				v.setu(lline, line-lline);
				line++; continue;
			}


			// data 0-9 or val
			if(*line=='-' || *line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z'){
				lline=line; 
				while(line<to){ if(!(*line=='-' || *line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z')) break; line++; }
				v.setu(lline, line-lline);
				if(v=="null")
					v.Clean();
				continue;
			}

			// key:val
			if(*line==':'){ k=v; v.sz=0; line++; continue; }
		
			// space
			if(*line==' '){ line++; continue; }

			// next, up, down
			if(*line==',' || *line==']' || *line=='[' || *line=='{' || *line=='}'){
			
				if(k || v || *line=='{' || *line=='['){
					// add element
					el = data.Add(); // &data.inserta();
					el->key=k; el->val=v; k.sz=0; v.sz=0;

					if(fel){ // first element
						el->_p=eel; eel->_n=el; eel=el;
						if(parent)
							parent->_e=el;
					} else{
						if(parent){
							parent->_a=el; parent->_e=el; // update parent
						}
						fel=el; eel=el; // this is first
					}
				}

				// up, down
				if(*line==','){ line++; continue; }
				if(*line=='[' || *line=='{'){ line++; ReadJsonDo(el, line, to); continue; }
				if(*line==']' || *line=='}'){ line++; return ; }
			}

			int what_the_sumbol=1;

			// what the sumbol? ignore
			line++;
		}

		return ;
	}

	// Unknown Format. Hmm... T1 -> TF(irst)
	int ReadTF(VString data, bool noclear=0){
		return ReadTF(data, data, noclear);
	}

	int ReadTF(unsigned char*data, unsigned int size, bool noclear=0){
		if(!noclear){ Clear(); }

		ReadTFDo(0, data, data+size);

		return 1;
	}

	void ReadTFDo(XDataEl*parent, unsigned char*&line, unsigned char*to){
		VString k, v; unsigned char *lline=line;
		XDataEl*fel=0, *eel=0, *el;

		while(line<to){

			// data "key" "val"
			if(*line=='"'){
				lline=++line; 
				while(line<to){ if(*line=='"' && ( line<=lline || *(line-1)!='\\')) break; line++; }
				v.setu(lline, line-lline);
				line++; continue;
			}

			// data 0-9 or val
			if(*line=='-' || *line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z'){
				lline=line; 
				while(line<to){ if(!(*line=='-' || *line>='0' && *line<='9' || *line>='a' && *line<='z' || *line>='A' && *line<='Z')) break; line++; }
				v.setu(lline, line-lline);
				continue;
			}

			// key:val
			if(*line==':'){ k=v; v.sz=0; line++; continue; }
		
			// space
			if(*line==' '){ line++; continue; }

			// next, up, down
			if(*line==',' || *line==']' || *line=='[' || *line=='{' || *line=='}'){
			
				if(k || v || *line=='{' || *line=='['){
					// add element
					el = data.Add(); //&data.inserta();
					el->key=k; el->val=v; k.sz=0; v.sz=0;

					if(fel){ // first element
						el->_p=eel; eel->_n=el; eel=el;
						if(parent) parent->_e=el;
					} else{
						if(parent){
							parent->_a=el; parent->_e=el; // update parent
						}
						fel=el; eel=el; // this is first
					}
				}

				// up, down
				if(*line==','){ line++; continue; }
				if(*line=='[' || *line=='{'){ line++; ReadJsonDo(el, line, to); continue; }
				if(*line==']' || *line=='}'){ line++; return ; }	
			}

			int what_the_sumbol=1;

			// what the sumbol? ignore
			line++;
		}

		return ;
	}


	// XML
	int ReadXML(VString data, bool noclear=0){
		return ReadXML(data, data, noclear);
	}

	int ReadXML(unsigned char*data, unsigned int size, bool noclear=0){
		if(!noclear){ Clear(); }

		ReadXMLDo(0, data, data+size);

		return 1;
	}

	void ReadXMLDo(XDataEl*parent, unsigned char*&line, unsigned char*to){
		VString k, v; unsigned char *lline=line;
		XDataEl *pel=0, *fel=0, *eel=0, *el=0, *nppl = 0;

		int type=0;

		while(line<to && *line!='<'){ line++; }

		if(line != lline){
			pel = data.Add(); // &data.inserta();
			pel->val.setu(lline, line-lline);

			if(parent){
				if(parent->_a){
					pel->_p=parent->_e;
					parent->_e->_n=pel;
					parent->_e=pel;
					pel->_n=0;
				}else{
					parent->_a=pel;
					parent->_e=pel;
				}
			} else{
				if(nppl)
					nppl->_n = pel;
				nppl = pel;
			}
		}
		
		if(line>=to)
			return ;

		// <tag>text</tag>
		//if(line+1<to && *(line+1)=='/'){
		//	if(line-lline && parent)
		//		parent->val.setu(lline, line-lline);
		//	return ;
		//}

		type=0; lline=++line;
		
		while(line<to){
			if(*line==' ' || *line=='\t' || *line=='\r' || *line=='\n' || *line=='>' || *line=='/'){
				if(type==0){
					pel = data.Add(); // &data.inserta();
					pel->key.setu(lline, line-lline);

					if(parent){
						if(parent->_a){
							pel->_p=parent->_e;
							parent->_e->_n=pel;
							parent->_e=pel;
							pel->_n=0;
						}else{
							parent->_a=pel;
							parent->_e=pel;
						}
						//pel->_a=0;
						//pel->_e=0;
						//pel->_l=0;
					} else{
						if(nppl)
							nppl->_n = pel;
						nppl = pel;
					}

					type=1;
				}else if(type==1){
					if(line!=lline)
						v.setu(lline, line-lline);

					if(k && v){
						//ReadXMLDoSetKV();
						el = data.Add(); //&data.inserta();
						el->key=k; el->val=v;
						
						if(!pel->_l){
							pel->_l=fel=eel=el;
						}else{
							eel->_n=el; el->_p=eel; eel=el;
						}
						
						k.Clean(); v.Clean();
						//
					}
				}

				if(*line=='>'){
					if(line>lline && *(line-1)=='/')
						type=2;
					
					line++;
					break;
				}

				lline=line+1;
			}
			else if(*line=='='){
				if(line!=lline){
					v.setu(lline, line-lline); lline=line+1;
				}
				k=v;
				v.Clean();
			}
			else if(*line=='\''){
				lline=++line;
				while(line<to){
					if(*line=='\'' && (line<=lline || *(line-1)!='\\')){
						v.setu(lline, line-lline); lline=line+1;
						break;
					}
					line++;
				}
			}
			else if(*line=='"'){
				lline=++line;
				while(line<to){
					if(*line=='"' && (line<=lline || *(line-1)!='\\')){
						v.setu(lline, line-lline); lline=line+1;
						break;
					}
					line++;
				}
			}
			else if(*line=='>'){
				if(line!=lline)
					v.setu(lline, line-lline);

				if(k && v){
					//ReadXMLDoSetKV();
					el = data.Add(); //&data.inserta();
					el->key=k; el->val=v;
						
					if(!pel->_l){
						pel->_l=fel=eel=el;
					}else{
						eel->_n=el; el->_p=eel; eel=el;
					}
						
					k.Clean(); v.Clean();
					//
				}

				if(line>lline && *(line-1)=='/')
					type=2;

				line++;
				break;
			}

			// what the sumbol? ignore
			line++;
		}

		if(type==1 && pel)
			ReadXMLDo(pel, line, to);

		// </tag>

		while(line<to){
			if(*line=='>')
				break;
			line++;
		}

		return ;
	}


	int ReadHtml(VString data, bool noclear=0){
		return ReadHtml(data, data, noclear);
	}

	int ReadHtml(unsigned char *data, unsigned int size, bool noclear = 0){
		if(!noclear)
			Clear();

		XDataEl *el = this->data.Add();

		ReadHtmlDo(el, data, data + size);

		return 1;
	}

	XDataEl* ReadHtmlAdd(XDataEl *parent){
		XDataEl *el = data.Add();

		if(parent){
			if(!parent->_a){
				parent->_a = el;
				parent->_e = el;
				//el->_p = 0;
				//el->_n = 0;
				//el->_l = 0;
			} else{
				parent->_e->_n = el;
				el->_p = parent->_e;
				parent->_e = el;
				//el->_n = 0;
				//el->_l = 0;
			}
		}

		return el;
	}

	XDataEl* ReadHtmlAddL(XDataEl *parent, XDataEl *lparent){
		if(!parent)
			return 0;

		XDataEl *el = data.Add();

		if(!lparent){
			parent->_l = el;
		} else{
			lparent->_l = el;
		}

		lparent = el;

		return el;
	}

	struct ReadHtmlPrev{
		ReadHtmlPrev *prev;
		VString tag;
		int toit;
		//XDataEl *parent;

		ReadHtmlPrev(){
			prev = 0;
			toit = 0;
		}

		void GoTo(VString tag){
			ReadHtmlPrev *p = prev;

			while(p){
				if(p->tag == tag){
					p->toit = 1;
					return ;
				} else
					p->toit = 0;
			}

			if(prev)
				prev->toit = 1;
		}

	};

	void ReadHtmlDo(XDataEl *parent, unsigned char *&line, unsigned char *to, ReadHtmlPrev *prev = 0){
		XDataEl *el, *lel;
		VString k, v;
		unsigned char *lline = line;

		// Prev
		ReadHtmlPrev nprev;
		nprev.prev = prev;

		int opt_cl; // </tag>
		int opt_kv; // key(0) = val(1)

		while(line < to){
			lline = line;

			// Find <
			while(line < to && *line != '<')				
				line ++;
			
			if(line != lline){				
				el = ReadHtmlAdd(parent);
				el->val = VString(lline, line - lline);
				lline = line;
			}

			// <tag>
			line ++;
			
			// </tag>
			if(line < to && *(line) == '/'){
				line ++;
				opt_cl = 2;
			}
			else
				opt_cl = 0;

			lline = line;

			// tag name
			while(line < to && ( *line != ' ' && *line != '\t' && *line != '\n'
				 && *line != '\r' && *line != '/' && *line != '>'))
				line ++;

			if(opt_cl){
				nprev.GoTo(VString(lline, line - lline));
				line ++;
				break;
			}

			el = ReadHtmlAdd(parent);
			nprev.tag = VString(lline, line - lline);
			el->key = nprev.tag;
			lline = line;
			opt_kv = 0;

			lel = 0;
		
			// to end tag
			while(line < to && *line != '>'){

				// string
				if(*line >= 'a' && *line <= 'z' || *line >= 'A' && *line <= 'Z' || *line >= '0' && *line <= '9'){
					lline = line;
					while(line < to && (*line >= 'a' && *line <= 'z' || *line >= 'A' && *line <= 'Z' || *line >= '0' && *line <= '9'))
						line ++;

					if(!opt_kv){
						k.setu(lline, line - lline);
					} else {
						v.setu(lline, line - lline);
						opt_kv = 2;
					}
				}

				// "text"
				else if(*line == '"'){
					lline = ++line;

					while(line < to){
						if(*line == '"')
							break;

						if(*line == '\\'){						
							if(line + 1 < to)
								line ++;
	
						}
						line ++;
					}
					v.setu(lline, line - lline);
					if(opt_kv)
						opt_kv = 2;
					line ++;
					continue;
				}

				// 'text'
				else if(*line == '\''){
					lline = ++line;

					while(line < to){
						if(*line == '\'')
							break;

						if(*line == '\\'){						
							if(line + 1 < to)
								line ++;
	
						}
						line ++;
					}
					v.setu(lline, line - lline);
					if(opt_kv)
						opt_kv = 2;
					line ++;
					continue;
				}

				else if(*line == '=' && k){
					opt_kv = 1;
					line ++;
				}

				//if(*line != ' ' || *line != '\t' || *line != '\n'
				// || *line != '\r' || *line != '/' || *line != '>'){
				//	 lline = line;
				//}

				else
					line ++;

				if(opt_kv == 2){
					lel = ReadHtmlAddL(el, lel);
					lel->key = k;
					lel->val = v;
					opt_kv = 0;
					k.Clean();
				}
			}

			if(line < to){
				if(*(line - 1) == '/')
					opt_cl = 1;
				line ++;
			}

			if(opt_cl == 0){
				ReadHtmlDo(el, line, to, &nprev);
				if(!nprev.toit)
					return ;
			} /*else if(opt_cl == 2){
				nprev.GoTo();
				break;
			}*/
		}

		return ;
	}


	// XML Valid. bad: -1, EOF: 0, Ok: 1.
	static int ValidateXML(VString data){ return ValidateXML(data, data.sz); }

	static int ValidateXML(unsigned char *data, unsigned int sz){
		int l=0;

		unsigned char *line=data, *to=data+sz;
		while(line<to){
			if(*line=='<'){
				line++;
				if(line>=to) return 0;
				if(*line=='/') l--; else l++;

				while(line<to){
					if(*line=='>'){
						if(*(line-1)=='/') l--;
						line++;
						break;
					}
					line++;
				}

				if(l<0) return -1;
			}

			line++;
		}

		if(l<0) return -1;
		return l==0;
	}


	void Clear(){ data.Clean(); sz=0; }

};




class XDataXmlEl{
public:
	// Prev / Next elements
	XDataXmlEl *_p, *_n;

	// Up First / End elements
	XDataXmlEl *_a, *_e;

	// Line values <key=val>
	XDataXmlEl *_l;

	// Key / Val
	VString key, val;

	XDataXmlEl(){
		Clean();
	}

	XDataXmlEl* p(){
		if(this)
			return _p;
		return 0;
	}

	XDataXmlEl* n(){
		if(this)
			return _n;
		return 0;
	}

	XDataXmlEl* a(){
		if(this)
			return _a;
		return 0;
	}

	XDataXmlEl* e(){
		if(this)
			return _e;
		return 0;
	}

	XDataXmlEl* l(){
		if(this)
			return _l;
		return 0;
	}

	VString k(){
		if(this)
			return key;
		return VString();
	}

	VString v(){
		if(this)
			return val;
		return VString();
	}

	XDataXmlEl* FindX(XDataXmlEl *el, VString line){
		for(el; el; el = el->_n){
			if(el->key == line)
				return el;
		}

		return 0;
	}

	XDataXmlEl* Get(VString line){
		XDataXmlEl *el = a();
		VString l;

		while(el != 0 && line){
			l = PartLine(line, line, ".");
			el = el->FindX(el, l);

			if(el != 0 && line)
				el = el->_a;
		}

		return el;
	}

	VString GetVal(VString line){
		XDataXmlEl *el = Get(line);

		if(el)
			return el->val;

		return VString();
	}

	XDataXmlEl* GetL(VString line){
		if(!this)
			return 0;

		return FindX(_l, line);
	}

	VString GetLVal(VString line){
		XDataXmlEl *el = GetL(line);

		if(el)
			return el->val;

		return VString();
	}

	void Clean(){
		_p = 0;
		_n = 0;
		_a = 0;
		_e = 0;
		_l = 0;
	}

};

class XDataXml{
	AListAllocUList<XDataXmlEl> list;
	XDataXmlEl pel;

public:
	XDataXml(){}
	XDataXml(VString line){
		Read(line);
	}

	XDataXmlEl* First(){
		return &pel;
	}

	operator XDataXmlEl*(){
		return &pel;
	}

	void Read(VString line){
		Clean();

		unsigned char *ln = line;

		ReadDo(&pel, ln, line.endu());
	}

	int ReadDo(XDataXmlEl *pel, unsigned char *&ln, unsigned char *to){
		XDataXmlEl *plel = 0;
		unsigned char *lln = ln;

		// Text
		while(ln < to && *ln != '<')
			ln++;

		if(lln != ln){
			pel->val.setu(lln, ln - lln);
			lln = ln;
		}

		if(ln >= to)
			return 0;

		// Tag name
		lln = ++ ln;

		while(ln < to && ( *ln != ' ' && *ln != '/' && *ln != '>'))
			ln++;

		if(ln >= to)
			return 0;

		// Closed tag
		if(*lln == '/'){
			while(ln < to && *ln != '>')
				ln++;

			if(ln < to)
				ln ++;
			return -2;
		}

		// Create tag
		XDataXmlEl *tel = list.AllocNew();
		VString key, val;

		tel->key = VString(lln, ln - lln);
		OMatrixTemplateAdd(pel->_a, pel->_e, tel);
		
		while(ln < to){
			SkipSpace(ln, to);

			if(ln >= to)
				return 0;

			if(*ln == '>')
				break;

			key = GetPar(ln, to);

			SkipSpace(ln, to);

			if(ln < to && *ln == '='){
				XDataXmlEl *lel = list.AllocNew();
				OMatrixTemplateAdd_A_PN_PEL(tel->_l, plel, lel);
				lel->key = key;
				lel->val = GetPar(++ln, to);
				plel = lel;
			}
			else if(key){
				XDataXmlEl *lel = list.AllocNew();
				OMatrixTemplateAdd_A_PN_PEL(tel->_l, plel, lel);
				lel->key = key;
				plel = lel;
			}
		}

		if(ln >= to)
			return 0;

		if(ln > lln && *(ln - 1) == '/'){
			ln ++;
			return -1;
		}

		ln ++;

		// Next tag
		while(1){
			int ret = ReadDo(tel, ln, to);

			if(ret < -1)
				return ret + 1;

			if(!ret)
				break;
		}

		return 0;
	}

	void SkipSpace(unsigned char *&ln, unsigned char *to){
		while(*ln == ' ' || *ln =='\t' || *ln =='\r' || *ln == '\n' || *ln =='/')
			ln ++;

		return ;
	}

	VString GetPar(unsigned char *&ln, unsigned char *to){
		unsigned char *lln;

		SkipSpace(ln, to);

		if(ln >= to)
			return VString();

		if(*ln == '/' || *ln == '>')
			return VString();

		// '
		if(*ln == '\''){
			lln = ++ ln;
			while(ln < to){
				if(*ln == '\\')
					ln ++;
				else if(*ln == '\'')
					break;

				ln ++;
			}

			if(ln < to){
				ln ++;
				return VString(lln, ln - lln - 1);
			}

			return VString();
		}		
		
		// "
		if(*ln == '\"'){
			lln = ++ ln;
			while(ln < to){
				if(*ln == '\\')
					ln ++;
				else if(*ln == '\"')
					break;

				ln ++;
			}

			if(ln < to){
				ln ++;
				return VString(lln, ln - lln - 1);
			}

			return VString();
		}

		// Normal
		lln = ln;

		while(ln < to){
			if(*ln == ' ' || *ln == '=' || *ln == '\t' || *ln == '\r' || *ln == '\n' || *ln == '/' || *ln == '>')
				break;

			ln ++;
		}

		if(ln < to){
			return VString(lln, ln - lln);
		}

		return VString();
	}

	void Clean(){
		pel.Clean();
		list.Clean();
	}

};


// Fast XML Parser

struct XMLFCall{
public:
	virtual void start() = 0;
	virtual void end() = 0;

	virtual void tag(VString name) = 0;
	virtual void etag(VString name) = 0;

	virtual void attr(VString key, VString val) = 0;
	virtual void text(VString text) = 0;
};




// GetLine
void GetLine(XDataEl &val, LString &ls);

MString GetLine(XDataEl &val){
	if(!val._a) return MString();
	LString ls;
	GetLine(val, ls);
	return ls;
}

MString GetLine(XDataEl *val){
	if(!val) return MString();
	LString ls;
	GetLine(*val, ls);
	return ls;
}

void GetLine(XDataEl *val, LString &ls){
	if(!val) return ;
	GetLine(*val, ls);
	return ;
}

void GetLine(XDataEl &val, LString &ls){
	int a = val.a() != 0 && val.a()->key, f=1;
	if(a)
		ls+"{";
	else
		ls+"[";

	XDataEl *v=val._a;
	while(v){
		if(!f) ls+","; else f=0;
		if(v->_a){
			if(a)
				ls+"\""+v->key+"\":";
			GetLine(*v, ls);
		} else{
			if(v->key)
				ls+"\""+v->key+"\":";
			if(!v->val) ls+"null";
			else ls+"\""+v->val+"\"";
		}
		//ls+v->key+":"+v->val;
		v=v->_n;
	}

	if(a)
		ls+"}";
	else
		ls+"]";
	return ;
}