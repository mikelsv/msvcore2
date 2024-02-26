//#include "../../opensource/msvcore2/msvcore.cpp"
#include "mgl_common.h"

MgGenChan::MgGenChan(){
	memcpy(_gen_chan_name, "channel", 7);
	memcpy(_gen_ichan_name, "iChannel", 8);
	memcpy(_gen_rchan_name, "iChannelResolution[", 19);
	memcpy(_gen_buff_name, "buffer-", 7);
}

VString MgGenChan::GenChanName(int id){
	_gen_chan_name[7] = '0' + id;
	_gen_chan_name[8] = 0;

	return VString(_gen_chan_name, 8);
}

VString MgGenChan::GenIChanName(int id){
	_gen_ichan_name[8] = '0' + id;
	_gen_ichan_name[9] = 0;

	return VString(_gen_ichan_name, 9);
}

VString MgGenChan::GenIChanResName(int id){
	_gen_rchan_name[19] = '0' + id;
	_gen_rchan_name[20] = ']';
	_gen_rchan_name[21] = 0;

	return VString(_gen_rchan_name, 21);
}

VString MgGenChan::GenBuffName(int id){
	_gen_buff_name[7] = 'a' + id;
	_gen_buff_name[8] = 0;

	return VString(_gen_buff_name, 8);
}


bool IsImage(VString file){
	return (file.str(-3) == "bmp" || file.str(-3) == "jpg" || file.str(-3) == "png") && IsFile(file);
}

bool IsVideo(VString file){
	return (file.str(-3) == "mp4" || file.str(-3) == "avi" || file.str(-4) == "mpeg" || file.str(-4) == "webm") && IsFile(file);
}