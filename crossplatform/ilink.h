//struct DUMS{ MString key, val; };
struct DUVS{ VString key, val; };
//struct DUMSi{ MString key, val; unsigned int i; };

// ILink
class ILink{
public:
	//MString data;
	const VString proto, domain, path, file, iquest, target;
	DUVS *ilinkq;
	MString mdata; // << link + ilinkq
	//TLine ilinkq;
	unsigned short port, largs, args; BYTE portsz; bool ltype;

	ILink();
	ILink(const VString link, int type = 0);
	~ILink();

private:
	ILink(const ILink &ilink);

public:

	void Clean();
	//void Ilink(const MString &ilink){ return Ilink(VString(ilink)); }
		
	void Link(const VString ilink, int tp=0);
	void MoveData(char*ln, int mo, int pr, int dom, int pa, int fi, int iq, int tg);
	void AnalysIQuest(const VString &_iquest);
	void AnalysIQuestSet(const VString &_iquest);

	// Get
	VString name();
	VString ext();
	// Find
	DUVS* Find(const VString &line);

	char* ProtoPos();
	char* DomainPos();
	char* PathPos();
	char* FilePos();
	char* QuestPos();

	void SetProto(const VString nproto);
	void SetDomain(const VString &nfile);
	void SetFile(const VString &nfile);
	void SetPath(const VString &npath);
	void SetQuest(const VString &nquest);


	//VString GetFullPath();
	VString GetFullPath();
	VString GetOnlyPath();
	VString GetPathFile();
	VString GetProtoDomainPath();
	VString GetPathFileQuest();

	void FirstArg();
	VString GetArg();
	VString GetLastArg();
	VString GetArg(VString ret);
	//VString GetArgI(VString key);
	int GetArgSz();
};