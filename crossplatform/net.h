// Functions
void KeepAlive(SOCKET sock);
unsigned int GetIP(VString ipa);
unsigned int GetIPh(VString ipa);
bool ifrecv(SOCKET sock, unsigned int wt = 0);
bool ifsend(SOCKET sock, unsigned int wt = 0);
void gettip(SOCKET sock, unsigned int &ip, unsigned short &port);
void getcip(SOCKET sock, unsigned int &ip, unsigned short &port);

// ip <=> int
unsigned int ipstoi(const VString& line);
unsigned int ipstoi(char*data, unsigned int sz);
MString ipitos(unsigned int val);

// Connect Ip
class ConIp{
public:
	// Data
	unsigned int ip, bip;
	unsigned short port, bport;
	char reuse, keepalive, nolisten, set_nonblock;
	int type, proto;

public:
	// Constructor
	ConIp();
	ConIp(VString _ip, VString _bip = VString());
	ConIp(unsigned int _ip, unsigned short _port, unsigned int _bip = 0, unsigned short _bport = 0);

	// Init
	void Init();

	// Set
	void Ip(VString _ip, VString _bip = VString());
	void IpL(VString _ip, VString _bip = VString());

	// Options
	int ReUse();
	int ReUse(int v);
	int KeepAlive();
	int KeepAlive(int v);
	int NoListen();
	int NoListen(int t);

	void SetNonBlock(int v = 1);

	void Proto(int _type, int _proto);

	// Actions
	int Connect() const;
	SOCKET Listen();
};