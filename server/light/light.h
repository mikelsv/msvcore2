// Use child class. Listen();

Versions lightserver_versions[]={
	"0.0.0.2", "01.07.2015 19:41",
	"0.0.0.1", "28.09.2013 16:29"	
};

class LightServerAccept{
public:
	unsigned int cip, tip; unsigned short cport, tport; SOCKET sock;
};

class LightServer{
protected:
	MString open_ip;
	//unsigned int open_ip; unsigned short open_port;
	SOCKET open_sock;
	int stopserver;

	// Options
	int count_connect, max_connect;

	// Lock
	TLock lock;

public:
	LightServer(){
		open_sock = 0;
		count_connect = 0;
		max_connect = 0;
		stopserver = 0;
	}

	void SetPort(VString ip){
		open_ip = ip;
	}

	void SetMaxCon(int v){
		max_connect = v;
	}

	//void SetPort(unsigned short p, unsigned int ip=0){
	//	open_ip=ip; open_port=p;
	//}


	int StopServer(){
		stopserver = 1;
	}

	int Listen(){
		fd_set rfds, efds; SOCKET maxs; int tps=0, pos=0, lpos=0, sel;
		sockaddr_in from; int fromlen=sizeof(from); timeval tm; SOCKET socks; ConIp cip;

		// Listen		
		//cip.ip=open_ip; cip.port=open_port;
		cip.IpL(open_ip);
		cip.ReUse(1);
		
		open_sock = cip.Listen();
		if(!open_sock)
			return 0;

		// Run server
		stopserver = 0;

		while(!stopserver){
			FD_ZERO(&rfds); FD_ZERO(&efds); maxs=0;

			FD_SET(open_sock, &rfds);
			maxs = open_sock + 1;

			tm.tv_sec = 30;
			tm.tv_usec = 0;
			sel = select(maxs, &rfds, 0, 0, &tm);
			
			if(sel < 1){
				Sleep(100);
			}

			if(FD_ISSET(open_sock, &rfds)){
				socks = accept(open_sock, (struct sockaddr*)&from, (socklen_t*)&fromlen);
				if(socks <= 0){
					closesocket(open_sock);
					stopserver = 1;
				}

				if(max_connect && max_connect <= count_connect){
					closesocket(socks);
					continue;
				}

				UGLOCK(lock);
				count_connect ++;

				LightServerAccept acc;
				acc.sock=socks; acc.cip=ntohl(from.sin_addr.s_addr); acc.cport=htons(from.sin_port);
				getsockname(socks, (struct sockaddr*)&from, (socklen_t*)&fromlen);
				acc.tip=ntohl(from.sin_addr.s_addr); acc.tport=htons(from.sin_port);

				Accept(acc);
			}
		}

		return 0;
	}

	virtual int Accept(LightServerAccept &acc){
		SString it;
		char buf[S16K];
		//HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n

		int rcv = recv(acc.sock, buf, S16K, 0);

		it.Add("LightServer(", lightserver_versions[0].ver, ", ", lightserver_versions[0].date,")");
		send(acc.sock, it, it, 0);
		closesocket(acc.sock);

		// Count
		UGLOCK(lock);
		count_connect --;
		return 0;
	}

};


#ifdef USEMSV_LIGHTSERVER_HTTP_CALL
int USEMSV_LIGHTSERVER_HTTP_CALL(LightServerAccept &acc, VString head, VString post);
#endif


class LightServerHttp : public LightServer, public MSVMCOT{

	// certs
	static VString cert_cert, cert_key;

public:
	void SetCerts(VString cert, VString key){
		cert_cert = cert;
		cert_key = key;
	}

	virtual int Accept(LightServerAccept &acc){
		// Call Accept() in new Thread.

		LightServerAccept *nacc = new LightServerAccept;
		*nacc = acc;
		MCOTS(nacc);

		return 1;
	}

	virtual DWORD MCOT(LPVOID lp){
		LightServerAccept *acc = (LightServerAccept*)lp;
		unsigned int ip;
		unsigned short port;
		SString s;

		getcip(acc->sock, ip, port);
		s.Format("New connection: %ip!:%d!\r\n", ip, port);
		print(s);

		AcceptData(*acc);
		print("Connection close!\r\n");
		closesocket(acc->sock);
		delete acc;

		// Count
		UGLOCK(lock);
		count_connect --;

		return 0;
	}

	virtual int AcceptData(LightServerAccept &acc){
		unsigned char buf[S32K];
		unsigned int pos = 0;
		MySSL ssl;

		// Use ssl sertificate
		if(cert_cert && cert_key){
			ssl.Release();
			ssl.Accept(acc.sock, cert_cert, cert_key);
		} else
			ssl.AcceptNoSsl(acc.sock);

		while(1){
			// read data from socket
			if(ifrecv(acc.sock)){
				int rcv;

				if(cert_cert)
					rcv = ssl.Recv((char*)(buf + pos), S32K - pos);
				else
					rcv = recv(acc.sock, (char*)(buf + pos), S32K - pos, 0);

				if(rcv <= 0)
					break;

				pos += rcv;

				// analys data
				VString read(buf, pos);

				// get head
				VString head, post;
				head = PartLine(read, post, "\r\n\r\n");
				// get method
				VString method = PartLineO(read, " ");
				
				if(post.data){
					unsigned int cl = PartLineDoubleUp(head, "Content-Length: ", "\r\n").toi();
					if(cl && post.size() < cl)
						continue;
					post.sz = cl;
				}
				else
					continue;

				head.sz += 2;

#ifdef USEMSV_LIGHTSERVER_HTTP_CALL
				if(USEMSV_LIGHTSERVER_HTTP_CALL(acc, head, post)){
					SString shead;
					//VString shead = "HTTP/1.0 200 OK\r\n"
//						"Connection: close\r\n";
					
					int ka = PartLineDoubleUp(head, "Connection: ", "\r\n").compareu("Keep-Alive");

					VString sdata = "{}";
					shead.Format("HTTP/1.1 200 OK\r\n"
						"Connection: %s\r\n"
						"Content-Length: %d\r\n"
						"\r\n",
						ka ? "close" : "keep-alive",
						sdata.sz
						);

					ssl.Send(shead, shead);
					//ssl.Send("\r\n", 2);
					ssl.Send(sdata, sdata);

					// Keep-Alive
					if(ka){
						print("Keep-Alive.\r\n");
						pos = 0;
						continue;
					}

					ssl.Close();
					return 0;
				}
#endif

				// send answer
				VString shead = "HTTP/1.0 200 OK\r\n";
				VString sdata = "Hello world!";

				ssl.Send(shead, shead);
				ssl.Send("\r\n", 2);
				ssl.Send(sdata, sdata);

				ssl.Close();
				return 0;
			}
		}

		ssl.Close();
		return 0;
	}


};

VString LightServerHttp::cert_cert;
VString LightServerHttp::cert_key;

#ifdef USEMSV_LIGHTSERVER_WEBSOCKET_PROCESS
int USEMSV_LIGHTSERVER_WEBSOCKET_PROCESS(LightServerAccept *acc);
#endif

class LightServerWebsocket : public LightServer, public MSVMCOT{
protected:
	// certs
	static VString cert_cert, cert_key;
	// timeout
	int recv_timeout;

public:
	LightServerWebsocket(){
		recv_timeout = 0;
	}

	void SetCerts(VString cert, VString key){
		cert_cert = cert;
		cert_key = key;
	}

	void SetTimeout(int v){
		recv_timeout = v;
	}

	virtual int Accept(LightServerAccept &acc){
		// Call Accept() in new Thread.

		LightServerAccept *nacc = new LightServerAccept;
		*nacc = acc;
		MCOTS(nacc);

		return 1;
	}

	virtual DWORD MCOT(LPVOID lp){
		LightServerAccept *acc = (LightServerAccept*)lp;
		unsigned int ip;
		unsigned short port;
		SString s;

		getcip(acc->sock, ip, port);
		s.Format("New connection: %ip!:%d!\r\n", ip, port);
		print(s);

		AcceptData(*acc);
		print("Connection close!\r\n");
		closesocket(acc->sock);
		delete acc;

		// Count
		UGLOCK(lock);
		count_connect --;
		return 0;
	}

	virtual int AcceptData(LightServerAccept &acc){
		MySSL ssl;

		// Use ssl sertificate
		if(cert_cert && cert_key){
			ssl.Release();
			ssl.Accept(acc.sock, cert_cert, cert_key);
		} else
			ssl.AcceptNoSsl(acc.sock);

		return RecvData(acc, ssl);

	}

	virtual int RecvData(LightServerAccept &acc, MySSL &ssl){
		unsigned char buf[S32K];
		unsigned int pos = 0;
		int wstat = 0, readed = 0, close = 0;

		while(1){
			// read data from socket
			if(ifrecv(acc.sock, recv_timeout)){
				int rcv = ssl.Recv((char*)(buf + pos), S32K - pos);

				if(rcv <= 0)
					break;

				pos += rcv;

				// Analys
				while(!close && pos){
					readed = 0;

					storm_recv(acc, ssl, wstat, VString(buf, pos), readed, close);

					if(readed){
						memcpy(buf, buf + readed, pos - readed);
						pos -= readed;
					}

					if(!readed)
						break;
				}

				if(close)
					break;
			}
		}

		ssl.Close();
		return 0;
	}

	virtual void storm_recv(LightServerAccept &acc, MySSL &ssl, int &wstat, VString read, int &readed, int &close){
		if(!wstat){
			VString head, data;
			head = PartLine(read, data, "\r\n\r\n");

			if(data.data){
				readed = head.size()+4;
				head.sz += 4;

				return AnalysHead(acc, ssl, wstat, head, data);
			}
			else 
				return ;
		}else{
			int r, opcode;
			VString msg;

			while(1){
				r = WebSocketDecodeData(read, opcode, msg);
				if(r < 0)
					return ;
				if(r == 0){
					close = 1;
					return ;
				}

				AnalysData(acc, ssl, opcode, msg, close);
				readed = r;

				//if(!read || close)
				return ;
			}
		}

		return ;
	}

	virtual void AnalysHead(LightServerAccept &acc, MySSL &ssl, int &wstat, VString head, VString data){
		//VString path = PartLineDouble(head, "GET ", " HTTP");
		//VString host = PartLineDouble(head, "\r\nHost: ", "\r\n");
		VString key = PartLineDouble(head, "\r\nSec-WebSocket-Key: ", "\r\n");
		VString origin = PartLineDouble(head, "\r\nOrigin: ", "\r\n");

		SString ret;
		ret.Add("HTTP/1.1 101 Switching Protocols\r\n" \
			"Upgrade: WebSocket\r\n" \
            "Connection: Upgrade\r\n" \
            //"Sec-WebSocket-Origin: ", origin, "\r\n" 
			"Sec-WebSocket-Accept: ", WebSocketAcceptKey(key), "\r\n" \
			//"Sec-WebSocket-Protocol: chat" "\r\n" 
            "\r\n");

		ssl.Send(ret, ret);
		
		wstat = 1;
		return ;
	}

	virtual void AnalysData(LightServerAccept &acc, MySSL &ssl, int opcode, VString read, int &close){
		switch(opcode){
			case LWSOC_STRING:{
				MString ret = WebSocketEncodeData(LWSOC_STRING, LString() + "recieved: " + read);
				ssl.Send(ret, ret);
			}
			break;

			case LWSOC_BINARY:{
				MString ret = WebSocketEncodeData(LWSOC_STRING, LString() + "recieved: " + read);
				ssl.Send(ret, ret);
			}
			break;

			case LWSOC_PING:{
				MString ret = WebSocketEncodeData(LWSOC_PONG, MString());
				ssl.Send(ret, ret);
			}
			break;

			case LWSOC_CLOSE:
			default:
				close = 1;
			break;
		}

		return ;
	}
};

VString LightServerWebsocket::cert_cert;
VString LightServerWebsocket::cert_key;

#ifdef STORMSERVER_CORE_MODSTATE

class LightServerWebDebug : public LightServerWebsocket{

	void SendHello(LightServerAccept &acc, MySSL &ssl){
		_listen_http_modstate_dbg_templ hello;
		hello.Encode(STORM_DEBUG_CMD_HELLO, sizeof(hello));

		WebSocketEncodeS1K enc;
		enc.Encode(LWSOC_BINARY, hello, hello);
		ssl.Send(enc.GetData(), enc.GetSize());
	}

	void StatTime(LightServerAccept &acc, MySSL &ssl, unsigned int &stime){
		unsigned int tm = time(0);

		// Send new data
		if(stime + 3 < tm){
			stime = tm;

			SendState(acc, ssl);
		}
	}

	void SendState(LightServerAccept &acc, MySSL &ssl){
		_listen_http_modstate_dbg_state state;
		state.Encode(listen_http_modstate.GetState());

		WebSocketEncodeS1K enc;
		enc.Encode(LWSOC_BINARY, state, state);
		ssl.Send(enc.GetData(), enc.GetSize());
	}

	virtual int RecvData(LightServerAccept &acc, MySSL &ssl){
		unsigned char buf[S32K], dbuf[S32K];
		unsigned int pos = 0;
		int wstat = 0, hello = 0, readed = 0, close = 0;
		unsigned int stime = 0;

		// Debug
		unsigned int debug_rid = listen_http_modstate.StartDebug();// MyStormCore.StartDebug();

		while(1){
			// read data from socket
			if(ifrecv(acc.sock, 0)){
				int rcv = ssl.Recv((char*)(buf + pos), S32K - pos);

				if(rcv <= 0)
					break;

				pos += rcv;

				// Analys
				while(!close && pos){
					readed = 0;

					storm_recv(acc, ssl, wstat, VString(buf, pos), readed, close);

					if(readed){
						memcpy(buf, buf + readed, pos - readed);
						pos -= readed;
					}

					if(!readed)
						break;
				}

				if(close)
					break;		
			}

			// Send hello
			if(!hello && wstat){
				SendHello(acc, ssl);
				hello = 1;
			}

			if(wstat){
				StatTime(acc, ssl, stime);

				//RingTime(acc, ssl, debug_rid);
			}			

			// Debug data
			int count = listen_http_modstate.ReadDebug(debug_rid, dbuf, sizeof(dbuf));
			if(count){
				if(count < 0)
					break;

				WebSocketEncodeS1K enc;
				enc.Encode(LWSOC_BINARY, dbuf, count);

				ssl.Send(enc.GetData(), enc.GetSize());
			}
		}

		listen_http_modstate.EndDebug();
		ssl.Close();
		return 0;
	}

};

#endif