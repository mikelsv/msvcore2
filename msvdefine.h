// Include OS Defines
#include "define/sys.h"

// Include Base Defines
#include "define/base.h"

// Include Virtual Strings
#include "VString.h"

// Include Multi Type
#include "crossplatform/mte.h"

// Include Lock
#include "crossplatform/lock.h"

// Include Threads
#include "crossplatform/threads.h"

// Include Real Strings
#include "MString.h"

#ifdef UNICODE
	#include "crossplatform/wince-twoms.h"
#endif

// Include Temp or Thread String
#include "TString.h"

// Include Stack String
#include "SString.h"
#include "SString.cpp"

// Include Auto List
#include "list/AList.h"

// Include Unlimited List
#include "list/UList.h"
#include "list/UList.cpp"

//Include Object List
#include "list/OList.h"

// Include String Defines
#include "define/string.h"

// Include Network
#include "crossplatform/net.h"

// Include Send & Recv
#include "crossplatform/send.h"

// Include Config Line Options
#include "crossplatform/conflineops.h"

// Include Ideal List
#include "list/IList.h"

// Include I/O
#include "crossplatform/io.h"



// Msv core main
int msvcoremain(int args, char* arg[], char* env[]);