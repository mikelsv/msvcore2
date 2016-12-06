/*	Copyright (C) 2004-2100 Mishael Senin, aka MikelSV.  All rights reserved.

	Ћицензи€ попул€ризации автора. :]
	¬ы не должны измен€ть им€ и другую информацию о разработчике и не присваивать авторство себе.
	Ќе продавать и не брать денег за код из данной библиотеки.
	–азрешаетс€ модифицировать и дорабатывать.
	 од предоставлен как есть и автор не отвечает, даже если вы попытаетесь прострелить себе ногу.

	License popularizing author. [LPA]
	You should not change the name and other information about the developer and not assign authorship itself.
	Do not sell and do not take money for the code from this library.
	Permission is granted to modify and refine.
	Author bears no responsibility for any consequences of using this code.
	Responsibility for shooting through your leg lies entirely on you.
* /

//>> Velcome to MSV Lib. <<
//>> New name MSV Core <<
//>> New name MSV Core Two [21.11.2016 15:27] <<


// !!!!!>>>>>>>>>>> Add this to general cpp: #define USEMSV_GENERALCPP
//					Add to project: "..\..\opensource\msvcore2\VString.cpp" "..\..\opensource\msvcore2\MString.cpp"


// ----------------------------------------------------------------- Simple Variant ---------------------------
/*
#define USEMSV_GENERALCPP
#define PROJECTNAME "projectname"
#define PROJECTVER PROJECTNAME ## _versions

#include "../../opensource/msvcore2/msvcore.cpp"

Versions PROJECTVER[]={
	// new version to up
	"0.0.0.1", "10.10.2013 21:24"
};

int main(int args, char* arg[]){
	msvcoremain(args, arg);
	print(PROJECTNAME, " v.", PROJECTVER[0].ver," (", PROJECTVER[0].date, ").\r\n");
	return 0;
}
*/
// ----------------------------------------------------------------- End of Simple Variant -----------------------

// Process info: msvcorestate (class MsvCoreState).


// Extensions

// Formats:
// #define USEMSV_XDATACONT - use XDataContainer ( Parsers: XML, Json )

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
// #define USEMSV_MSL_FL - use MSL Fast Line
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
// #define USEMSV_MEMORYCONTROL - interception malloc() & free()
// #define USEMSV_INTERCEPT_MALLOC - interception malloc() & free() // don't work

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



#ifndef PROJECTNAME
	#error Please set #define PROJECTNAME "you_project_name"
#endif

// Include Defines
#include "msvdefine.h"

#ifdef USEMSV_GENERALCPP
	// Include Code
	#include "msvdefine.cpp"

	int msvcoremain(int args, char* arg[]){
		msvcorestate.Main(args, arg);
	return 1;
	}

#endif

#ifdef USEMSV_TRIELIST
	#include "list/TrieList.cpp"
#endif

#ifdef USEMSV_HASH
	#include "crossplatform/hash.cpp"
#endif

#ifdef USEMSV_CONSOLE
	#include "crossplatform/console.cpp"
#endif

#ifdef USEMSV_XDATACONT
	#include "proto/xdatacont.cpp"
#endif

#ifdef USEMSV_OPENSSL
	#include "crossplatform/openssl.cpp"
#endif

#ifdef USEMSV_PCRE
	#include "special/pcre-include.h"
#endif

#ifdef USEMSV_MYSQL
	#include "proto/mysql-con.cpp"
#endif

#ifdef USEMSV_HTTP
	//#include "crossplatform/iheader.cpp"
	#include "crossplatform/http.cpp"
#endif

#ifdef USEMSV_TGBOTS
	#include "proto/tgbots.cpp"
#endif

#ifdef USEMSV_STORMSERVER
	#include "server/storm/storm-include.h"
#endif

#ifdef USEMSV_LIGHTSERVER
	#include "server/light/light.h"
#endif