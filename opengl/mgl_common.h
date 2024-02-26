class MgGenChan{
	char _gen_chan_name[10];
	char _gen_ichan_name[11];
	char _gen_rchan_name[22];
	char _gen_buff_name[10];

public:
	MgGenChan();
	VString GenChanName(int id);
	VString GenIChanName(int id);
	VString GenIChanResName(int id);
	VString GenBuffName(int id);
};


bool IsImage(VString file);
bool IsVideo(VString file);


#ifdef USEMSV_MSVCORE

int MgCmdHelp(){
	print("\r\nMaticals Render help:\r\n");

	print(LString() + 
"-w, -window	: window resolution. Format: -window width:height:fullscreen" "\r\n"
"-l, -load		: load shader. Format: -load path_to_folder" "\r\n"
"-f, -fps		: set frames per sec. Format: -fps 30. (v.0.0.1.1)" "\r\n"
"-o, output		: save as video. Foramt: -output file.mp4. (v.0.0.1.1)" "\r\n"
"-t, -time		: start, stop times in sec. Format: -time 10, 25. (v.0.0.2.1)" "\r\n"
"-i, -import	: import from shadertoy. Format: -import https://www.shadertoy.com/view/shader_id" "\r\n"
"-c, -config	: use configure file. By default: glsl_render.json. Format: -config file.json. (v.0.0.1.1)" "\r\n"
"-h				: this help" "\r\n"
"-v				: version information" "\r\n"
"\r\n"
	);
	return 1;
}

int MgCmdVer(){
	print("\r\nMaticals Render versions:\r\n");
	int s = sizeof(PROJECTVER) / sizeof(Versions);

	for(int i = 0; i < s; i ++){
		print(LString() + PROJECTVER[i].ver + " \t" + PROJECTVER[i].date + "\r\n");
	}

	print("\r\n");
	return 1;
}

#endif