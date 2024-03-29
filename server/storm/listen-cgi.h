// Storm Cgi
//
// Idea:
// --> Command, env, input<realtime>.
// <-- Ret: head<max s16k> + body<realtime>

#ifndef USEMSV_CONSOLE
	#error Use #define USEMSV_CONSOLE
#endif

#define STORM_LISTEN_CGI_ERROR	-1
#define STORM_LISTEN_CGI_NONE	0
#define STORM_LISTEN_CGI_WRITE	1
#define STORM_LISTEN_CGI_READ	2
#define STORM_LISTEN_CGI_CLOSE	3
#define STORM_LISTEN_CGI_DELETE	4

#define STORM_LISTEN_CGI_PIPE	0
#define STORM_LISTEN_CGI_SOCK	1

#ifdef USEMSV_MSVCORE
	TLock listen_cgi_lock;
#endif

//#define PipeLine2 PipeLine

// Run:
// Run
// Write input
// Read output & errput
/*
class listen_cgi_el2{
	int state;

	PipeLine2 ppl;
	SendDataGLock input, output, eutput;
	//HLString hls, ehls;
	//int rc;

public:

	listen_cgi_el2(){
		state = STORM_LISTEN_CGI_NONE;
	}

	int State(){
		return state;
	}

	int Work(){
		return state == STORM_LISTEN_CGI_READ || state == STORM_LISTEN_CGI_WRITE;
	}

	int Run(VString cmd, VString env, int isinput = 0){
		if(state != STORM_LISTEN_CGI_NONE)
			return 0;

		// exec
		//ppl.SetEnv(env);
		//ppl.SetInput(input);
		//ppl.SetCont(0);

		int run = ppl.Run(cmd, "", env, 0);

		if(run > 0)
			if(isinput)
				state = STORM_LISTEN_CGI_WRITE;
			else
				state = STORM_LISTEN_CGI_READ;
		else
			state = STORM_LISTEN_CGI_ERROR;

		return run > 0;
	}

	int Write(VString data){


	}

	int Read(VString &read, VString &err){

	}

	int Readed(int sz, int esz){

		
	}

	int Close(){
		state = STORM_LISTEN_CGI_CLOSE;
		ppl.Clean();
		input.Clean();
		output.Clean();
		eutput.Clean();
	}

};
*/

class listen_cgi_el{
	PipeLine2 ppl;
	GetHttp2 gp;
	storm_socket sock;
	int state;
	int type;
	int gp_pos;

public:
	listen_cgi_el(){
		state = STORM_LISTEN_CGI_NONE;
		type = STORM_LISTEN_CGI_PIPE;
		gp_pos = 0;
	}

	int GetState(){
		return state;
	}

	int SetState(int s){
		return state = s;
	}

	SendDataRing& StdIn(){
		return ppl.StdIn();
	}

	SendDataRing& StdOut(){
		return ppl.StdOut();
	}

	SendDataRing& StdErr(){
		return ppl.StdErr();
	}

	void StdInClose(){
		ppl.StdInClose();
	}

	int Run(VString cmd, VString dir = VString(), VString env = VString(), int inp = 0){
		int res = 0;

		if(cmd.str(0, 7) == "http://" || cmd.str(0, 8) == "https://"){
			gp.SetPost(env);
			res = gp.Request(cmd);
			type = STORM_LISTEN_CGI_SOCK;
		} else {
			res = ppl.Run(cmd, dir, env, inp);
			type = STORM_LISTEN_CGI_PIPE;
		}		

		if(!res){
			state = STORM_LISTEN_CGI_DELETE;
		} else
			state = STORM_LISTEN_CGI_READ;

		return res;
	}

	int Work(){
		if(type == STORM_LISTEN_CGI_PIPE){
			if(ppl.Process())
				return 1;
		} else {
			int wr = ppl.StdOut().Write(gp.GetData().str(gp_pos));
			gp_pos += wr;
			return wr;
		}
		
		return 0;
	}

};

DWORD StormListenCgiThread(LPVOID);

class listen_cgi : public TLock{ // : public storm_item{
	OList<listen_cgi_el> list;

public:
	listen_cgi(){
		StartThread(StormListenCgiThread);
	}

	listen_cgi_el* New(){
		return list.NewE();
	}

	void Free(listen_cgi_el* del){
		del->SetState(STORM_LISTEN_CGI_DELETE);

		return ;

		//OListP<listen_cgi_el> el = list;

		//while(el.Next()){
		//	if(del == el){
		//		el->SetState(STORM_LISTEN_CGI_DELETE);
		//		return ;
		//	}
		//}

		//if(list.Is(del))
		//	el->SetState(STORM_LISTEN_CGI_DELETE);
		//else
		//	print("Warning! Logic error! #54fa91 \r\n");


		return ;
	}

	int Thread(){
		while(1){
			OListP<listen_cgi_el> el(list);

			while(el.Next()){
				//if(el->GetState() == STORM_LISTEN_CGI_READ)
				//	el->Work();

				if(el->GetState() == STORM_LISTEN_CGI_DELETE){
					el.Free();
					//list.Del(el);
					//el = 0;
				}
			}

			Sleep(10);
		}

		return 0;
	}

	//STORM_DEFINE_NEW_DELETE(listen_cgi);
};

#ifdef USEMSV_MSVCORE
	class listen_cgi listen_cgi;
#endif

//STORM_DEFINE_NEW_DELETE_OK(listen_cgi);

#ifdef USEMSV_MSVCORE

DWORD StormListenCgiThread(LPVOID){
	listen_cgi.Thread();
	return 0;
}

#endif