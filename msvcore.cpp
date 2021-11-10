/*	Copyright (C) 2004-2100 Mishael Senin, aka MikelSV.  All rights reserved. MIT Licence. * /

//>> Velcome to MSV Lib. <<
//>> New name MSV Core <<
//>> New name MSV Core Two [21.11.2016 15:27] <<

// Add to project: "path_to_msvcore2\VString.cpp" "path_to_msvcore2\MString.cpp"
// For me: "..\..\opensource\msvcore2\VString.cpp" "..\..\opensource\msvcore2\MString.cpp"

// ----------------------------------------------------------------- First cpp -----------------------------------
/*
#define USEMSV_MSVCORE // include msvcore cpp files.
// Add all msvcore extensions used in you proejects for include cpp files for it.

#include "path_to_msvcore2/msvcore.cpp"
*/
// ----------------------------------------------------------------- End of first cpp ----------------------------
// ----------------------------------------------------------------- Other cpp files  ----------------------------
/*
// include msvcore extensions used in this cpp.
#include "path_to_msvcore2/msvcore.cpp"
*/
// ----------------------------------------------------------------- End of other cpp files ----------------------


// ----------------------------------------------------------------- Simple Variant ---------------------------
/*
#define PROJECTNAME "projectname"
#define PROJECTVER PROJECTNAME ## _versions
#define USEMSV_MSVCORE

#include "path_to_msvcore2/msvcore.cpp"

Versions PROJECTVER[]={
	// new version to up
	"0.0.0.1", "10.10.2013 21:24"
};

int main(int args, char* arg[], char* env[]){
	msvcoremain(args, arg, env);
	print(PROJECTNAME, " v.", PROJECTVER[0].ver," (", PROJECTVER[0].date, ").\r\n");
	return 0;
}
*/

// ----------------------------------------------------------------- End of Simple Variant -----------------------

// Process info: msvcorestate (class MsvCoreState).


// Extensions

// Formats:
// #define USEMSV_XDATACONT - use XDataContainer ( Parsers: XML, Json )

// Other: 
// #define USEMSV_MSL_FL - use MSL Fast Line

// Extensions:
// #define USEMSV_PCRE - use PCRE functions // add to include path to pcre.h and pcre.lib
// #define USEMSV_MYSQL - use MySQL // add to include path to mysql.h and mysql.lib
// #define USEMSV_OPENSSL - use openssl // add to include path: ..\..\openssl-1.0.2\include

// Console:
// #define USEMSV_CONSOLE - use console functions

// Lists:
// [By Default] #define USEMSV_ILIST - use IList template dunamic array. Vector, one resized memory block.
// #define USEMSV_MLIST - use MList template dunamic array
// #define USEMSV_OLIST - use OList template dunamic array
// [By Default] #define USEMSV_ULIST - use UList template dynamic array. // defined by default 
// DELETED #define USEMSV_FSTRING - use FString class // defined by default 
// #define USEMSV_TRIELIST - Trie List

// Server:
// #define USEMSV_STORMSERVER - use storm server
// #define USEMSV_HTTP - use http functions
// #define USEMSV_WEBSOCKETS - use WebSockets client and listen_websockets for storm
// #define USEMSV_LIGHTSERVER - use light server

// Any
// #define USEMSV_HASH - hash functions: md5, sha.
// #define USEMSV_TGBOTS - Use telegramm bots class.
// #define USEMSV_INTERCEPT - interception functions

// Memory:
// #define USEMSV_MEMORYCONTROL - interception malloc() & free(). [Set as Global define!]


// OLD
// ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||
// \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/

// #define USEMSV_ANDOID - for android apps
// #define USEMSV_LOGPRINT - save output data to memory buffer
// #define USEMSV_FILEPRINT - save output data to file
// #define USEMSV_FLASH - use for Crossbridge compile

// Teplates:
// #define USEMSV_ITOS	- use Itos templates. Deprecated! Use: SString.

// Formats:
// +++  #define USEMSV_XDATACONT - use XDataContainer ( Parsers: XML, Json )
// #define USEMSV_AMF - use amf encoder/decoder
// #define USEMSV_NMEA - use nmea functions
// #define USEMSV_CJX - use CJX container

// Other: 
// +++ #define USEMSV_MSL_FL - use MSL Fast Line
// #define USEMSV_MSL_FV - use MSL Five
// #define USEMSV_CONFLINE - use ConfLine
// #define USEMSV_MODLINE - use ModLine
// #define USEMSV_CPPXCC - use CppXcc preprocessor

// +++ Lists:
// #define USEMSV_ILIST - use IList template dunamic array. Vector, one resized memory block.
// #define USEMSV_MLIST - use MList template dunamic array
// #define USEMSV_OLIST - use OList template dunamic array
// #define USEMSV_ULIST - use UList template dynamic array. // defined by default 
// DELETED #define USEMSV_FSTRING - use FString class // defined by default 


// Server:
// +++ #define USEMSV_STORMSERVER - use storm server
// +++ #define USEMSV_HTTP - use http functions
// +++ #define USEMSV_WEBSOCKETS - use WebSockets client and listen_websockets for storm
// +++ #define USEMSV_LIGHTSERVER - use light server

// #define USEMSV_TGBOTS		-	use Telegram Bots

// Network
// #define USEMSV_TRAFFIX - use Traffix class

// Extensions:
// +++ #define USEMSV_PCRE - use PCRE functions // add to include path to pcre.h and pcre.lib
// +++ #define USEMSV_MYSQL - use MySQL // add to include path to mysql.h and mysql.lib
// +++ #define USEMSV_OPENSSL - use openssl // add to include path: ..\..\openssl-1.0.2\include

// Console:
// +++ #define USEMSV_CONSOLE - use console functions
// #define USEMSV_CONSOLELINE - use console windows

// Windows and graphics:
// #define USEMSV_MWND			- use Mwnd Library
// #define USEMSV_HTMLRENDER	- use HtmlRender class.
// #define USEMSV_GAMEBOX		- use GameBox class.
	
// Memory:
// +++ #define USEMSV_MEMORYCONTROL - interception malloc() & free()
// +++ #define USEMSV_INTERCEPT_MALLOC - interception malloc() & free() // don't work

// API
// #define USEMSV_NESTAPI - use Nest API
// #define USEMSV_NESTAPI2 - use Nest API v.2

//
// #define USEMSV_UNSTABEBIGINT - unstable big int: val * 10 ^ pow.

// Special:
// #define USEMSV_BUFECHO - Input / Output buffer echo control
// #define USEMSV_TESTS - Testing

// Development.
// #define USEMSV_ASTRING_DEV - New AutoString classes.
// #define USEMSV_HASHLIST - Hash List.
// #define USEMSV_HASHTREE - Hash Tree.


// Include Defines
#include "msvdefine.h"

#ifdef USEMSV_MSVCORE
	// Include Code
	#include "msvdefine.cpp"
#endif

#ifdef USEMSV_TRIELIST
	#include "list/TrieList.cpp"
#endif

#ifdef USEMSV_HASH
	#include "extension/hash.h"
	#ifdef USEMSV_MSVCORE
		#include "extension/hash.cpp"
	#endif
#endif

#ifdef USEMSV_CONSOLE
	//#include "crossplatform/console.h"
	//#ifdef USEMSV_MSVCORE
		#include "crossplatform/console.cpp"
	//#endif
#endif

#ifdef USEMSV_XDATACONT
	#include "proto/xdatacont.cpp"
#endif

#ifdef USEMSV_OPENSSL
	#include "extension/openssl.h"	
	#ifdef USEMSV_MSVCORE
		#include "extension/openssl.cpp"
	#endif
#endif

#ifdef USEMSV_PCRE
	#include "special/pcre-include.h"
#endif

#ifdef USEMSV_MYSQL
	#include "proto/mysql-con.cpp"
#endif

#ifdef USEMSV_MWND
	#include "MWnd/MWndLib.h"
#endif

#ifdef USEMSV_HTTP
	#include "extension/http.h"
	#ifdef USEMSV_MSVCORE
		#include "extension/http.cpp"
	#endif
#endif

#ifdef USEMSV_MSL_FL
	#include "msl-fl/msl-fl.h"
#endif

#ifdef USEMSV_TGBOTS
	#include "proto/tgbots.cpp"
#endif

#ifdef USEMSV_WEBSOCKETS
	#include "proto/websockets.cpp"
#endif

#ifdef USEMSV_MODLINE
	#include "special/modline.cpp"
#endif

#ifdef USEMSV_STORMSERVER
	#include "server/storm/storm-include.h"
	//#ifdef USEMSV_MSVCORE
	//	#include "server/storm/storm-include.cpp"
	//#endif	
#endif

#ifdef USEMSV_LIGHTSERVER
	#include "server/light/light.h"
#endif

#ifdef USEMSV_INTERCEPT
#ifdef USEMSV_MSVCORE
	#include "special/asmp.h"
	#include "special/interception.h"
#endif
#endif