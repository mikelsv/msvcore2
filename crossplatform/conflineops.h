/* Options format:
key value
key     value
key			value
key value{
	key value
}

key
{
value
}


value } // NOT! Move '}' to new line.

*/

// Read Only

class ConfLineOption{
public:
	VString key, val;
	ConfLineOption *next, *up;
};

class ConfLineOptions{
	// Data
	LString0 confdata;
	
	// Options
	ConfLineOption *options;

	// Config file
	VString cfile;

	// Error
	MString err;

public:
	// Constructor
	ConfLineOptions();
	ConfLineOptions(VString file);

	// Load
	int LoadFile(VString file, int included = 0);
	int Load(VString line, int included = 0);

	// Pointer
	ConfLineOption* GetFirst();
	ConfLineOption* GetEnd();
	ConfLineOption* Next(ConfLineOption *opt);

	// Find
	ConfLineOption* FindOption(VString name, ConfLineOption *opt, ConfLineOption *parent = 0);
	ConfLineOption* FindOption2(VString name, VString name2, ConfLineOption *opt, ConfLineOption *parent = 0);

	// Get
	VString GetOption(VString name, ConfLineOption *opt = 0, ConfLineOption *parent = 0);

	VString DelSpace(VString line);
	VString GetKeyVal(VString line, VString &val);
	VString GetError();

protected:

	ConfLineOption* NewOption();
	VString NewOptionVal(VString line);
	void Read(VString &data, ConfLineOption *parent, int &lnum, int level = 0);
	void Error(VString line, int lnum);

public:
	~ConfLineOptions();
};
