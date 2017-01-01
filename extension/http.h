class MCookieD{
	MString data;
public:
	VString domain, path, name, value;
	unsigned int *expires;

	MCookieD();

	void Set(VString _domain, VString _path, VString _name, VString _value, unsigned int _expires);

};


class MCookie{
	OList<MCookieD> cookies;

public:
	// Constructor
	MCookie();
	MCookie(MCookie &cookie);

	// Get
	MString Get(VString domain, VString path);
	VString Get(VString domain, VString path, VString name);
protected:
	MCookieD* _Get(VString domain, VString path, VString name);	
	
	// Set
public:
	MCookie& operator=(MCookie &cookie);
	void Set(VString line, VString _domain, VString _path);
	void Set(VString domain, VString path, VString name, VString value, int expires = S2GM);

	// Time
	int GetTime(VString line);
	MString GetTime(int tm);
	
	// Copy
	void Copy(MCookie &cookie, int clear = 1);

	// Import
	void Import(VString line);

	// Export
	MString Export();
	MString ExportDO();
		
	// Clean
	void Clear();

	// Destructor
	~MCookie();
};

// MHttpValue struct
struct MHttpValue{
	MHttpValue *next;
	unsigned int k, v;
	char data[0];

	// Get
	VString key();
	VString val();

	// Next
	MHttpValue* Next(MHttpValue *&next);

	// Find
	VString Find(const VString &ln);
	MHttpValue* FindV(const VString &ln);
	MHttpValue* FindVU(const VString &ln);

	// Insert
	MHttpValue* Insert(LString &data, MHttpValue *&v, const VString &key, const VString &val);

};

// MHttpFile struct
struct MHttpFile{
	MHttpFile *next;
	VString name, fname, data;
	
	MHttpFile* Find(const VString &nm);
};

// MHttp class
class MHttp{
public:
	VString head, post, method, path, http;
	ILink ilink;
	MHttpValue *input, *iget, *ipost, *cookies, *headers;
	MHttpFile *ifile;

private:
	LString data;
	MCookie cookie;

public:
	// Constructor
	MHttp();

	// Get
	int GetCode();
	VString Input(VString key);
	VString GetHead(VString name);
	MString GetCookies();
	MString GetCookie(VString name);

	// Set
	void Set(VString line);

protected:
	void* DataAddKV(MHttpValue *&v, const VString &key, const VString &val);

public:
	// Clean
	void Clean();
};


// GetHttpReq class //
class GetHttpReq{
public:
	// Connect ip
	// ConIp ip;

	// Request
	MString r_method, r_agent, r_accept, r_post, r_boundary_id, r_boundary, r_cookies, r_referrer, r_header, page;

	// Responce 
	MString head, data;
	MHttp http;

	// Timeout, tepeat, usessl
	int timeout;
	int repeat;

protected:
	int usessl;

public:
	// All recv & send
	int64 arecv, asend;

	// Ssl
#ifdef USEMSV_OPENSSL
	MySSL ssl;
#endif

	// Constructor
	GetHttpReq();

	// Get
	int64 GetRecvSz();
	int64 GetSendSz();

	// Get / Set
	void SetMethod(VString v);
	VString GetMethod();
	
	void SetAgent(VString v);
	VString GetAgent();

	void SetAccept(VString v);
	VString GetAccept();

	void SetPost(VString v);
	VString GetPost();

	void SetBoundary(VString id, VString v);

	void SetHeader(VString v);

	void SetCookies(VString v);
	VString GetCookies();

	void SetReferrer(VString v);

	// Set
	void SetTimeout(int tm);
	void SetRepeat(int val);

	// Return
	MHttp& GetHead();
	VString GetHead(VString name);
	VString GetData();

	// Encode
	static MString EncodeUrl(VString line);
	static MString EncodePar(VString line);

	// Decode
	static TString DecodePostValue(VString val);

	// Boundary
	void AddBoundary(LString &ls, VString bnid, VString key, VString val);
	void AddBoundaryFile(LString &ls, VString bnid, VString key, VString name, VString val);
	void AddBoundaryEnd(LString &ls, VString bnid);

	// Analys
	virtual void AnalysHead(VString line, VString domain, VString path);

	// Request
	int Request(VString url);
protected:
	int DoRequest(VString url);
	int Recv(SOCKET sock);

public:
	// Recv / Send
	int recv(SOCKET sock, char *buf, int sz);
	int send(SOCKET sock, const char *buf, int sz);

	// Clean	
	void Clean();
};
// GetHttpReq class ~ //


// GetHttp class //
class GetHttp : public GetHttpReq{
public:
	GetHttp();
	void Clean();
};
// GetHttp class ~ //


// GetHttp2 class //
class GetHttp2 : public GetHttpReq{
	MCookie cookie;

public:
	GetHttp2();
	void Clean();
	void CleanCookie();

	MCookie& GetCookie();
	void SetCookie(MCookie &cook);

	virtual void AnalysHead(VString line, VString domain, VString path);

	int Request(VString url);
};
// GetHttp2 class ~ //