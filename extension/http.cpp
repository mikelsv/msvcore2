//#include "http-old.cpp"

MString HTI(const MString &line){
LString ret;
int sz=line.size();
for(int i=0;i<sz;i++){
	if(line[i]=='+') {ret+=" "; continue;}
if(line[i]=='%'){
	int ar[2];char car[2];
	car[0]=line[i+1];car[1]=line[i+2]; ar[0]=0; ar[1]=0;
	for(int ii=0;ii<2;ii++){
		if (int(car[ii])>47 && int(car[ii])<58) ar[ii]=int(car[ii])-48;
		if (int(car[ii])>64 && int(car[ii])<71) ar[ii]=int(car[ii])-55;
	}
ret+=MString(char(ar[0]*16+ar[1]));i++;i++;
}
else { ret + line.str(i, 1); }
}
return MString(ret.oneline(), ret.size());
}

MString HTInp(const MString &line){
LString ret;
int sz=line.size();
for(int i=0;i<sz;i++){
//	if(line[i]=='+') {ret+=" "; continue;}
if(line[i]=='%'){
	int ar[2];char car[2];
	car[0]=line[i+1];car[1]=line[i+2]; ar[0]=0; ar[1]=0;
	for(int ii=0;ii<2;ii++){
		if (int(car[ii])>47 && int(car[ii])<58) ar[ii]=int(car[ii])-48;
		if (int(car[ii])>64 && int(car[ii])<71) ar[ii]=int(car[ii])-55;
	}
ret+=MString(char(ar[0]*16+ar[1]));i++;i++;
}
else { ret + line.str(i, 1); }
}
return MString(ret.oneline(), ret.size());
}

MString CleanVal(MString val){
val=Replace(val, "&#032;", " ");
val=Replace(val, "&", "&amp;");
val=Replace(val, "<", "&lt;");
val=Replace(val, ">", "&gt;");
val=Replace(val, "\\n", "<br>");
val=Replace(val, "$", "&#036;");
val=Replace(val, "\\r", "");
val=Replace(val, "!", "&#33;");
val=Replace(val, "\"", "&quot;");
val=Replace(val, "'", "&#39;");
//val=Replace(val, "%25", "%");
//val=Replace(val, "%40", "@");
//val=Replace(val, "%3A", ":");
//val=Replace(val, "%2F", "/");
return val;
}

MString CleanValN(MString val){
val=Replace(val, "&#032;", " ");
val=Replace(val, "&lt;", "<");
val=Replace(val, "&gt;", ">");
val=Replace(val, "\\n", "<br>");
val=Replace(val, "&#036;", "$");
val=Replace(val, "\\r", "");
val=Replace(val, "&#33;", "!");
val=Replace(val, "&quot;", "\"");
val=Replace(val, "&#39;", "'");
val=Replace(val, "&apos;", "'");
val=Replace(val, "&nbsp;", " ");
val=Replace(val, "&amp;", "&");
return val;
}

MString CleanValR(MString val){
val=Replace(val, "&", "%26");
return val;
}

MString HTIEncode(VString line){
	MString ret;
	ret.Reserve(line.sz*3);

	unsigned char*ln=line, *to=line.endu(), *r=ret;
	while(ln<to){
		*r++='%';
		*r++=cb16[(*ln)/16];
		*r++=cb16[(*ln)%16];
		ln++;
	}

	return ret;
}

MString HtiGetEncode(VString line){
	MString ret;
	ret.Reserve(line.sz * 3);

	unsigned char *ln = line, *to = line.endu(), *r = ret;
	while(ln<to){
		if(*ln == ' ' || *ln == '/' || *ln == '&' || *ln == '+' || *ln == '\r' || *ln == '\n'){
			*r++ ='%';
			*r++ = cb16[(*ln)/16];
			*r++ = cb16[(*ln)%16];
			
		} else
			*r++ = *ln;

		ln ++;
	}

	return ret.str(0, r - ret.data);
}

// MCookieD class //
MCookieD::MCookieD(){
	expires = 0;
}

void MCookieD::Set(VString _domain, VString _path, VString _name, VString _value, unsigned int _expires){
	domain=_domain; path=_path; name=_name; value=_value;
	VString exp((char*)&_expires, sizeof(int));
	data.AddR(domain, path, name, value, exp);
	expires = (unsigned int*)exp.data;
	return ;
}
// MCookieD class ~ //


// MCookie class //
// Constructor
MCookie::MCookie(){}
MCookie::MCookie(MCookie &cookie){ Copy(cookie); }

// Get
MString MCookie::Get(VString domain, VString path){
	LString ls;
	MCookieD *p = 0; //, *d;
	unsigned int tm = time();

	while(p=cookies.Next(p)){
		if(*p->expires < tm){
			// d=p; p=cookies.Prev(p); cookies.Del(d);
			continue;
		}

		if((domain.str(domain.size() - p->domain.sz)==p->domain || p->domain[0]=='.' && p->domain.str(1)==domain) && path.incompare(p->path))
			ls+p->name+"="+p->value+"; ";
	}
	return VString(ls.oneline(), ls.size());
}

VString MCookie::Get(VString domain, VString path, VString name){
	UGLOCK(cookies);
	MCookieD *p=0;
	while(p=cookies.Next(p)){
		if(p->domain==domain && p->path==path && p->name==name)
			return p->value;
	}
	return VString();
}

MCookieD* MCookie::_Get(VString domain, VString path, VString name){
	MCookieD *p=0;
	while(p=cookies.Next(p)){
		if(p->domain==domain && p->path==path && p->name==name)
			return p;
	}
	return p;
}

// Set
MCookie& MCookie::operator=(MCookie &cookie){
	Copy(cookie);
	return *this;
}

void MCookie::Set(VString line, VString _domain, VString _path){
	VString name, value, domain = _domain, path = _path, o, t;
	unsigned int expires = S2GM, first = 1;

	domain = PartLineO(domain, ":");

	// username=aaa13; expires=Friday, 31-Dec-99 23:59:59 GMT; path=/; domain=www.citforum.ru;\n\n";
	// remixlang=0; expires=Fri, 12 Jan 2018 12:22:50 +0300; path=/; domain=.vk.com


	while(line){
		dspacev(line, 7);
		o=PartLine(line, line, ";");
		o=PartLine(o, t, "=");

		if(first){
			name = o;
			value = t;
			first = 0;
		}else if(o.compareu("path"))
			path = t;
		else if(o.compareu("domain"))
			domain = t;
		else if(o.compareu("expires"))
			expires = GetTime(t);
		else if(o.compareu("secure"))
			{} // none
	}

	if(domain && path && name)
		//cookies.NewE()->
		Set(domain, path, name, value, expires);

	return ;
}

void MCookie::Set(VString domain, VString path, VString name, VString value, int expires){
	UGLOCK(cookies);
	MCookieD * p = _Get(domain, path, name);
	if(!p){
		if(value == "DELETED" || expires < time())
			return ;

		p = cookies.NewE();
	}else{
		if(value == "DELETED" || expires < time()){
			cookies.Free(p);
			return ;
		}
	}
	p->Set(domain, path, name, value, expires);
	return ;
}

// Time
int MCookie::GetTime(VString line){
	MTime mt;
	return mt.gmttotime(line);
}

MString MCookie::GetTime(int tm){
	MTime mt;
	return mt.date("r", tm, 1);
}

// Copy
void MCookie::Copy(MCookie &cookie, int clear){
	UGLOCKD(cookies, cookie.cookies);

	if(clear)
		Clear();

	MCookieD *p=0, *e;
	while(p=cookie.cookies.Next(p)){
		e=_Get(p->domain, p->path, p->name);
		if(!e)
			e=cookies.NewE();
		e->Set(p->domain, p->path, p->name, p->value, *p->expires);			
	}

	return ;
}

// Import
void MCookie::Import(VString line){
	UGLOCK(cookies);

	while(line){
		VString o=PartLine(line, line, "\r\n");
		Set(o, "", "");
	}

	return ;
}

// Export
MString MCookie::Export(){
	UGLOCK(cookies);
	LString ls;
		
	MCookieD *p=0;
	while(p=cookies.Next(p)){
		ls+p->name+"="+p->value+"; expires="+GetTime(*p->expires)+"; path="+p->path+"; "+"domain="+p->domain+"\r\n";
	}

	return VString(ls.oneline(), ls.size());
}

MString MCookie::ExportDO(){
	UGLOCK(cookies);
	LString ls;
		
	MCookieD *p=0;
	while(p=cookies.Next(p)){
		ls+p->name+"="+p->value+"; ";
	}

	return VString(ls.oneline(), ls.size());
}


// Clean
void MCookie::Clear(){
	cookies.Clean();
}

// Destructor
MCookie::~MCookie(){
	Clear();
}
// MCookie class ~ //


// MHttpValue struct //
// Get
VString MHttpValue::key(){
	if(!this)
		return VString();
	return VString((char*)&data, k);
}

VString MHttpValue::val(){
	if(!this)
		return VString();
	return VString(&data[k], v);
}

// Next
MHttpValue* MHttpValue::Next(MHttpValue *&next){
	if(next)
		next = next->next;
	return next;
}

// Find
VString MHttpValue::Find(const VString &ln){
	MHttpValue *p=(MHttpValue*)this;
	while(p){ if(compare(p->data, p->k, ln, ln)) return p->val(); p=p->next; }
	return VString();
}

MHttpValue* MHttpValue::FindV(const VString &ln){
	MHttpValue *p=(MHttpValue*)this;
	while(p){ if(compare(p->data, p->k, ln, ln)) return p; p=p->next; }
	return 0;
}

MHttpValue* MHttpValue::FindVU(const VString &ln){
	MHttpValue *p=(MHttpValue*)this;
	while(p){ if(compareu(p->data, p->k, ln, ln)) return p; p=p->next; }
	return 0;
}

// Insert
MHttpValue* MHttpValue::Insert(LString &data, MHttpValue *&v, const VString &key, const VString &val){
	MHttpValue *d=(MHttpValue*)&data.el(data.addnf(0, sizeof(void*)+sizeof(int)*2+key.size()+val.size()));
	//char *p=d+sizeof(void*);
	d->k=key.size(); d->v=val.size();
	memcpy((char*)&d->data, key, key);
	memcpy(&d->data[key.size()], val, val);
	if(!v){ v=d; return d; }
	MHttpValue *el=(MHttpValue*)v; 
	while(el){ if(!el->next){ el->next=(MHttpValue*)d; break; } el=el->next; }
	return d;
}
// MHttpValue struct ~//


// MHttpFile struct //
MHttpFile* MHttpFile::Find(const VString &nm){
	MHttpFile *p=(MHttpFile*)this;
	while(p){ if(compare(nm, nm, p->name, p->name)) return p; p=p->next; }
	return 0;
}



// MHttp class //
// Constructor
MHttp::MHttp(){
	input = 0;
	iget = 0;
	ipost = 0;
	cookies = 0;
	headers = 0;
	ifile = 0;
}

// Get
int MHttp::GetCode(){
	return stoi(head.str(9, 3));
}

VString MHttp::Input(VString key){
	return input->Find(key);
}

VString MHttp::GetHead(VString name){
	return headers->Find(name);
}

MString MHttp::GetCookies(){
	LString ls;
	MHttpValue *c = cookies;
	while(c){
		ls+c->key()+"; ";
		c=c->next;
	}
	return ls;
}

MString MHttp::GetCookie(VString name){
	MHttpValue *c = cookies;
	while(c){
		if(PartLineO(c->key(), "=")==name) return PartLineOT(c->key(), "=");
		c=c->next;
	}
	return VString();
}

// Set
void MHttp::Set(VString line){
	head=PartLine(line, post, "\r\n\r\n");
	method=PartLineO(head, "\r\n");
	method=PartLine(method, path, " ");
	path=PartLine(path, http, " ");

	MString p=NormalHttpPath(path); ilink.Link(p, 1);
		
if(method=="GET"){
	for(int i=0; i<ilink.args; i++){
		MString v=HTI(ilink.ilinkq[i].val);
		DataAddKV(iget, ilink.ilinkq[i].key, v);
		v=CleanVal(v);
		DataAddKV(input, ilink.ilinkq[i].key, v);
		//input.insert(ilink.ilinkq[i].key, CleanVal(()));
	}
}
if(method=="POST"){
	ilink.SetQuest(post);
	for(int i=0; i<ilink.args; i++){
		MString v=HTInp(ilink.ilinkq[i].val);
		DataAddKV(ipost, ilink.ilinkq[i].key, v);
		v=CleanVal(v);
		DataAddKV(input, ilink.ilinkq[i].key, v);
	//input.insert(ilink.ilinkq[i].key, CleanVal(HTI(ilink.ilinkq[i].val)));
	}
}

VString ln=head;
while(ln){
	VString o, t;
	o=PartLine(ln, ln, "\r\n");
	o=PartLine(o, t, ": ");
	DataAddKV(headers, o, t);
	if(o=="Set-Cookie"){
		VString k, v;
		k=PartLine(t, v, ";");
		DataAddKV(cookies, k, v);
		// new
		cookie.Set(t, "", "");
	}
}

	return ;
}


void* MHttp::DataAddKV(MHttpValue *&v, const VString &key, const VString &val){
	MHttpValue *d=(MHttpValue*)&data.el(data.addnf(0, sizeof(void*)+sizeof(int)*2+key.size()+val.size()));
	//char *p=d+sizeof(void*);
	d->k=key.size(); d->v=val.size();
	memcpy((char*)&d->data, key, key);
	memcpy(&d->data[key.size()], val, val);
	if(!v){ v=d; return d; }
	MHttpValue *el=(MHttpValue*)v; while(el){ if(!el->next){ el->next=(MHttpValue*)d; break; } el=el->next; }
	return d;
}

// Clean
void MHttp::Clean(){
	input=0; iget=0; ipost=0; cookies=0; headers=0; ifile=0;
	head.Clean(); post.Clean(); method.Clean(); path.Clean(); http.Clean(); ilink.Clean(); data.Clean();
}


// GetHttpReq class //
GetHttpReq::GetHttpReq(){
	timeout = 30;
	repeat = 1;
	usessl = 0;

	arecv = 0;
	asend = 0;
}

// Get
int64 GetHttpReq::GetRecvSz(){ return arecv; }
int64 GetHttpReq::GetSendSz(){ return asend; }

// Get / Set
void GetHttpReq::SetMethod(VString v){ r_method = v; }
VString GetHttpReq::GetMethod(){ return r_method; }
	
void GetHttpReq::SetAgent(VString v){ r_agent=v; }
VString GetHttpReq::GetAgent(){ return r_agent; }

void GetHttpReq::SetAccept(VString v){ r_accept=v; }
VString GetHttpReq::GetAccept(){ return r_accept; }

void GetHttpReq::SetPost(VString v){ r_post=v; }
VString GetHttpReq::GetPost(){ return r_post; }

void GetHttpReq::SetBoundary(VString id, VString v){
	r_boundary_id = id;
	r_boundary = v;
}

void GetHttpReq::SetHeader(VString v){
	r_header = v;
}

void GetHttpReq::SetCookies(VString v){ r_cookies=v; }
VString GetHttpReq::GetCookies(){ return r_cookies; }

void GetHttpReq::SetReferrer(VString v){ r_referrer=v; }


// Set
void GetHttpReq::SetTimeout(int tm){ timeout = tm; }
void GetHttpReq::SetRepeat(int val){ repeat = val; }

// Return
MHttp& GetHttpReq::GetHead(){ return http; }
VString GetHttpReq::GetHead(VString name){ return http.GetHead(name); }
VString GetHttpReq::GetData(){ return data; }

// Encode
MString GetHttpReq::EncodeUrl(VString line){
	return Replace(Replace(Replace(Replace(line,
		" ", "%20"),
		"!", "%21"),
		"<", "%3C"),
		">", "%3E")
		;		
}

MString GetHttpReq::EncodePar(VString line){
	return Replace(Replace(Replace(Replace(Replace(Replace(Replace(line,
		" ", "%20"),
		"!", "%21"),
		"<", "%3C"),
		">", "%3E"),
		"&", "%26"),
		"+", "%2B"),
		"#", "%23")
		;		
}

// Decode
TString GetHttpReq::DecodePostValue(VString val){
	return HttpToVal(val);
}

// Boundary
void GetHttpReq::AddBoundary(LString &ls, VString bnid, VString key, VString val){
	ls + "--" + bnid + "\r\n" + "Content-Disposition: form-data; name=\"" + key + "\"" "\r\n" "\r\n" + val + "\r\n";
}

void GetHttpReq::AddBoundaryFile(LString &ls, VString bnid, VString key, VString name, VString val){
	ls + "--" +  bnid + "\r\n" + "Content-Disposition: form-data; name=\"" + key + "\"; " +  "filename=\"" + name + "\"" "\r\n" + "Content-Type: application/octet-stream" "\r\n" "\r\n" + val + "\r\n";
}

void GetHttpReq::AddBoundaryEnd(LString &ls, VString bnid){
	ls + "--" + bnid + "--" "\r\n";
}

// Analys
void GetHttpReq::AnalysHead(VString line, VString domain, VString path){
	http.Clean();
	http.Set(line);

	return ;
}

// Request
int GetHttpReq::Request(VString url){
	int ret = 0, r = repeat + 1;
	while(r){
		ret = DoRequest(url);
		if(ret)
			break;
		r--;
	}

	return ret;
}


int GetHttpReq::DoRequest(VString url){
	LString ls;
	ILink il;

	arecv = 0;
	asend = 0;

	//
	il.Link(url);
#ifdef USEMSV_OPENSSL
	if(il.proto=="https://"){
		if(!il.port)
			il.port=443;
		usessl = 1;
	}else
		usessl = 0;
#endif

	// ls+ r_method ? r_method : ( r_post ? "POST" : "GET" );
	if(r_method)
		ls + r_method;
	else
		if(r_post || r_boundary)
			ls + "POST";
		else
			ls + "GET";

	//ls+" "+EncodeUrl(il.GetPathFileQuest())+" HTTP/1.0\r\n";
	ls + " " + il.GetPathFileQuest() + " HTTP/1.0" "\r\n";
		
	ls + "Host: " + il.domain + "\r\n";
		
	ls + "User-Agent: ";
	if(r_agent)
		ls + r_agent;
	else
		ls +"Mozilla/5.0 (Windows NT 6.1; WOW64; rv:31.0) Gecko/20100101 Firefox/22.0";
	ls + "\r\n";

	ls + "Accept: ";
	if(r_accept)
		ls + r_accept;
	else
		ls + "*/*";
	ls + "\r\n";

	ls + "Accept-Language: ru-ru,ru;q=0.8,en-us;q=0.5,en;q=0.3\r\n";
	ls + "Accept-Encoding: none\r\n";
	ls + "Connection: close\r\n";

	if(r_cookies){
		ls + "Cookie: " + r_cookies + "\r\n";
	}
		
	if(r_post){
		ls + "Content-Length: " + r_post.size() + "\r\n";
		ls + "Content-Type: application/x-www-form-urlencoded\r\n";
	}

	if(r_boundary){
		ls + "Content-Length: " + r_boundary.size() + "\r\n";
		ls + "Content-Type: multipart/form-data; boundary=" + r_boundary_id + "\r\n";
	}

	if(r_referrer)
		ls + "Referrer: " + r_referrer + "\r\n";

	if(r_header)
		ls + r_header;

	ls+"\r\n";


	// Connect
	ConIp ip;

	ip.Ip(il.domain);
	ip.port = il.port ? il.port : 80;

	SOCKET sock = ip.Connect();
	if(sock <= 0)
		return 0;

#ifdef USEMSV_OPENSSL
	if(usessl){
		if(!ssl.Connect(sock)){
			closesocket(sock);
			return 0;
		}
	}
#endif

	int snd = send(sock, ls.oneline(), ls.size());
	if(!snd)
		return 0;

	asend += snd;

	if(r_post){
		snd = send(sock, r_post, r_post);
		asend += snd;
	}

	if(r_boundary){
		snd = send(sock, r_boundary, r_boundary);
		asend += snd;
	}

	if(!snd)
		return 0;

	int ret = Recv(sock);
	if(ret)
		AnalysHead(head, il.domain, il.path);

#ifdef USEMSV_OPENSSL
	if(usessl)
		ssl.Close();
#endif
	closesocket(sock);

	return ret;
}

int GetHttpReq::Recv(SOCKET sock){
	unsigned char buf[S8K];
	int bufs = 0, bufms = sizeof(buf);
	//unsigned int rdata = 0;
	int tm = time(), ltm = tm;
	int64 conlen = 0;
	int headread = 0;
	LString ls;

	while(1){
		if(bufs == bufms)
			return 0;

		// timeout
		ltm = time();
		if(ltm - tm > timeout)
			return 0;

		// recv
		if(!ifrecv(sock, 300))
			continue;

		int rcv = recv(sock, (char*)buf + bufs, bufms - bufs);

		if(rcv < 0)
			return 0;

		if(!rcv){
			if(conlen == -1){
				data = (VString)ls;
				return 1;
			}
			return 0;
		}

		arecv += rcv;
		bufs += rcv;
		tm = ltm;

		if(headread){
			ls.add((char*)buf, bufs);
			bufs = 0;

			if(conlen >= 0 && conlen <= ls.Size()){
				data = (VString)ls;
				return 1;
			}

			continue;
		}

		VString read(buf, bufs);
		if(!PartLineOT(read, "\r\n\r\n").data)
			continue;

		headread = 1;
		head.Add(PartLineO(read, "\r\n\r\n"), "\r\n\r\n");
			
		VString c = PartLineDouble(head, "Content-Length: ", "\r\n");
		conlen = c.data ? c.toi() : -1;

		memcpy(buf, buf + head.sz, bufs - head.sz);
		bufs -= head.sz;

		if(conlen == 0)
			return 1;

		ls.add((char*)buf, bufs);
		bufs = 0;

		if(conlen >= 0 && conlen <= ls.Size()){
			data = (VString)ls;
			return 1;
		}
	}

	return 0;
}

int GetHttpReq::send(SOCKET sock, const char *buf, int sz){
#ifdef USEMSV_OPENSSL
		if(usessl)
			return ssl.Send(buf, sz);
#endif
		return ::send(sock, buf, sz, 0);
}

int GetHttpReq::recv(SOCKET sock, char *buf, int sz){
#ifdef USEMSV_OPENSSL
		if(usessl)
			return ssl.Recv(buf, sz);
#endif
	return ::recv(sock, buf, sz, 0);
}

void GetHttpReq::Clean(){
	timeout = 30;
	repeat = 1;
	usessl = 0;

	r_method.Clean();
	r_agent.Clean();
	r_accept.Clean();
	r_post.Clean();
	r_boundary_id.Clean();
	r_boundary.Clean();
	r_cookies.Clean();
	r_referrer.Clean();
	r_header.Clean();
	page.Clean();

	head.Clean();
	data.Clean();

	http.Clean();

	arecv = 0;
	asend = 0;

#ifdef USEMSV_OPENSSL
	ssl.Release();
#endif
	return ;
}
// GetHttpReq class ~ //


// GetHttp class //
GetHttp::GetHttp(){ }
void GetHttp::Clean(){ GetHttpReq::Clean(); }
// GetHttp class ~ //

// GetHttp2 class //
GetHttp2::GetHttp2(){ }

void GetHttp2::Clean(){
	GetHttpReq::Clean();
}

void GetHttp2::CleanCookie(){
	cookie.Clear();
}

MCookie& GetHttp2::GetCookie(){
	return cookie;
}

void GetHttp2::SetCookie(MCookie &cook){
	cookie = cook;
}

void GetHttp2::AnalysHead(VString line, VString domain, VString path){
	GetHttpReq::AnalysHead(line, domain, path);

	VString o, t;
	while(line){
		o = PartLine(line, line, "\r\n");
		o = PartLine(o, t, ": ");

		if(o == "Set-Cookie")
			cookie.Set(t, domain, path);
	}

	return ;
}

int GetHttp2::Request(VString url){
	ILink il(url);
	r_cookies = cookie.Get(il.domain, il.path);

	return GetHttpReq::Request(url);
}
// GetHttp2 class ~ //