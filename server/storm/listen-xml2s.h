//TLock listen_xml_lock;
MString _xmpp_verify_uniq_string = md5h(LString()+time()+rand()+msvcorestate.procpath+"anystring");

#define XMPP_STATE_VERIFY_NO	0
#define XMPP_STATE_VERIFY_PRE	1
#define XMPP_STATE_VERIFY_UP	2
#define XMPP_STATE_VERIFY_OK	3

//#define XMPP_STATE_DVERIFY	-1

class listen_xml2s;

int listen_xml_srv_request(listen_xml2s *xmpp, XDataXmlEl *el);
int listen_xml_cli_request(listen_xml2s *xmpp, XDataXmlEl *el);

#ifndef USEMSV_LISTENXMPP_EXTFUNC
int listen_xml_srv_request(listen_xml2s *xmpp, XDataXmlEl *el){
	return 0;
}

int listen_xml_cli_request(listen_xml2s *xmpp, XDataXmlEl *el){
	return 0;
}
#endif

class listen_xml2s_auth{
	// To domain
	MString to;

	// Domain db:result;
	//MString dbres;

	// Cliend & server connection
	listen_xml2s *cli, *serv;

	listen_xml2s_auth(){
		cli = 0;
		serv = 0;
	}
};



class listen_xml2s : public listen_xml{
	MString xmpp_verify_code;
	storm_socket sock;

	// Verify con
	listen_xml2s *xmpp_verify_con;

	// Verify
	int xmpp_verify;

	// Auth
	//OList<listen_xml2s_auth> xmlstsa;
	
	// Events
	static listen_xml2s *events;

public:

	listen_xml2s(){
		xmpp_verify = 0;
		xmpp_verify_con = 0;
	}

	static void SetEvent(listen_xml2s *e){
		events = e;
	}

	virtual void storm_start(){
		listen_xml::storm_start();

		//xmpp_verify = 0;
		//xmpp_veryfy_con = 0;

		print("\r\n\r\n[STORM START]\r\n\r\n");
	}

	virtual void storm_setsock(StormCoreVirtual *core, storm_socket_data *data){
		sock.set(core, data);
	}

	virtual void storm_stop(){
		print("-------------- Jabber connestion closed!\r\n");
		print("-------------- count: ", itos(storm_items_list.Size() - 2), "\r\n");
		
		UGLOCK(storm_items_list);
		
		if(xmpp_verify_con){
			xmpp_verify_con->xmpp_verify_con = 0;
			xmpp_verify_con = 0;
		}

		listen_xml2s *p = 0;
		while(p = storm_items_list.Next(p)){
			if(p == this)
				continue;

			print("List: From: ", p->from, ", To: ", p->to, ", Ver: ", itos(p->xmpp_verify), ".\r\n");
		}

		print("\r\n");

		listen_xml::storm_stop();
	}

	virtual void storm_timeout(storm_work_el &wel){
		VString ping = "<ping/>";
		wel.send(ping);
		return ;
	}

	virtual void AnalysXmlLineCli(storm_work_el &wel, XDataXmlEl *el){
		VString act = el->key, res;
		SString it;

		if(listen_xml_cli_request(this, el))
			return ;

		if(act == "stream:features"){
			if(el->a() && el->a()->key=="starttls"){
				res="<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>";
				SendData(wel, res);
			} else {
				//if(xmpp_verify_con){
				//	it.Format("<db:result from='%s' to='%s'>%s</db:result>", from, to, XmppGenerateVerifyCode(from, to));
				//	SendData(wel, it);
				//}// else if(xmpp_verify_code){
				//	SString it; it.Format("<db:verify from='%s' to='%s'>%s</db:verify>", from, to, xmpp_verify_code);
				//	SendData(wel, it);
				//}

				//else{
					if(xmpp_verify_code){
						UGLOCK(storm_items_list);
						//MString code = GetVerifyCode(domain);

						//it.Format("<db:result from='%s' to='%s'>%s</db:result>", mydomain, domain, XmppGenerateCode(mydomain, domain));
						//SendData(wel, it);

						SString it; it.Format("<db:verify from='%s' to='%s'>%s</db:verify>", from, to, xmpp_verify_code);
						//it.Format("<db:result from='%s' to='%s'>%s</db:result>", mydomain, domain, "111-b4835385f37fe2895af6c196b59097b16862406db80559900d96bf6fa7d23df3");
						SendData(wel, it);


						if(!XmppIsVerifyDomain(to, from)){ // ?????????????????????????????????
							it.Format("<db:result from='%s' to='%s'>%s</db:result>", from, to, XmppGenerateVerifyCode(from, to));
							SendData(wel, it);
						}


					}else{
						if(!XmppIsVerifyDomain(to, from)){
							it.Format("<db:result from='%s' to='%s'>%s</db:result>", from, to, XmppGenerateVerifyCode(from, to));
							SendData(wel, it);
						}
					}
				//}
			}
		}
		else if(act == "proceed"){
			storm_ssl_connect(wel.data->sock);
			it.Format("<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:server' xmlns:db='jabber:server:dialback' from='%s' to='%s' version='1.0'>", to, from);
			SendData(wel, it);
		}
		else if(act == "db:result"){
			VString type = el->GetLVal("type");
			if(type){
				int ver = type == "valid";
				if(ver && xmpp_verify_con){
				//	XmppSetVerify(to, from, XMPP_STATE_VERIFY_UP, xmpp_verify_con->xmpp_verify_con);
					
					//it.Format("<db:result from='%s' to='%s' type='%s'/>", from, to, el->GetLVal("type"));
					//print(LString()+" >>>>> (" + (int)(!xmpp_state) + ") " + it + "\r\n\r\n");			
					//XmppSend(to, from, it, xmpp_verify_con);
				}
				//XmppVerifiedConnect(mydomain, domain, this, ver);
			}
		}
		else if(act == "db:verify"){
			if(el->GetLVal("type") == "valid"){
				//XmppSetVerify(from, to, XMPP_STATE_VERIFY_OK, xmpp_verify_con);
				
				//if(!XmppIsVerifyDomain(to, from)){
				//	it.Format("<db:result from='%s' to='%s'>%s</db:result>", from, to, XmppGenerateVerifyCode(from, to));
				//	SendData(wel, it);
				//}

				it.Format("<db:result from='%s' to='%s' type='%s'/>", from, to, el->GetLVal("type"));
				XmppSend(to, from, it, xmpp_verify_con);
			}
		}
		else if(act == "db:result !!!"){
			VString type = el->GetLVal("type");
			if(type){
				int ver = type == "valid";
				if(ver){
					XmppSetVerify(from, to, XMPP_STATE_VERIFY_UP);
					
					it.Format("<db:result from='%s' to='%s' type='%s'/>", from, to, el->GetLVal("type"));
					//print(LString()+" >>>>> (" + (int)(!xmpp_state) + ") " + it + "\r\n\r\n");			
					XmppSend(to, from, it, xmpp_verify_con);
				}
				//XmppVerifiedConnect(mydomain, domain, this, ver);
			}
		}
		else if(act == "db:verify !!!"){
			if(el->GetLVal("type") == "valid"){
				//XmppVerifiedConnect(domain, mydomain, con, el->GetLVal("type") == "valid");
				//SetDomainVerify(domain, con, el->GetLVal("type") == "valid");
				XmppSetVerify(to, from, XMPP_STATE_VERIFY_OK, xmpp_verify_con);

				// send my result
				//if(!XmppIsVerifyDomain(from, to)){
				//	it.Format("<db:result from='%s' to='%s'>%s</db:result>", from, to, XmppGenerateVerifyCode(from, to));
				//	SendData(wel, it);
				//}
			}

			it.Format("<db:result from='%s' to='%s' type='%s'/>", from, to, el->GetLVal("type"));
			//print(LString()+" >>>>> (" + (int)(!xmpp_state) + ") " + it + "\r\n\r\n");			
			XmppSend(to, from, it, xmpp_verify_con);

			if(xmpp_verify_con){
				if(xmpp_verify_con->xmpp_verify == XMPP_STATE_VERIFY_PRE)
					xmpp_verify_con->xmpp_verify = XMPP_STATE_VERIFY_OK;
				//it.Format("<db:result from='%s' to='%s'>%s</db:result>", from, to, XmppGenerateVerifyCode(from, to));
				//XmppSend(from, to, it, xmpp_verify_con);
				//SendData(wel, it);
			}

		}

		return ;
	}

	virtual void AnalysXmlLineServ(storm_work_el &wel, XDataXmlEl *el){
		VString act = el->key, res;
		SString it;

		if(listen_xml_srv_request(this, el))
			return ;

		if(!to && act != "stream:stream")
			return ;

		if(act == "stream:stream"){
			res="<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:server' xmlns:db='jabber:server:dialback' version='1.0'>";
			SendData(wel, res);

			if(!to){
				from=el->GetLVal("from");
				to=el->GetLVal("to");
			}

			if(!from || !to){
				wel.close();
				return ;
			}

			if(!ssl){
				res="<stream:features><starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/><c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='S4v2n+rKmTsgLFog7BtVvK2o660='/></stream:features>";
			} else{
				res="<stream:features><c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='S4v2n+rKmTsgLFog7BtVvK2o660='/></stream:features>";
			}

			SendData(wel, res);

//			if(!xmpp_verify_con)
//				GetAsyncCon(from, to);
		}
		else if(act == "starttls"){
			res="<proceed xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>";
			SendData(wel, res);

			// init ssl connection
			storm_ssl_accept_file(wel.data->sock, "zverus.ru.cert", "zverus.ru.key");
		}
		else if(act == "db:result"){
			//<db:result from='conference.jabber.ru' to='txmpp.ru'>9892754bb8ab1fe8c6a0ae0e086293ea57234f84b3e2c34847b42ae4b017d8f7</db:result>
			if(!el->GetLVal("type") && el->v()){
				it.Format("<db:verify from='%s' to='%s'>%s</db:verify>", to, from, el->v());
				if(listen_xml2s *con = XmppSend(to, from, it)){
					con->xmpp_verify_con = this;
					this->xmpp_verify_con = con;
				}
				else
					XmppConnect(to, from, this, el->v());
			}
		}
		else if(act == "db:verify"){
			if(!el->GetLVal("type") && el->v()){
				UGLOCK(storm_items_list);

				int ver = el->v() == XmppGenerateVerifyCode(to, from);
				if(ver && xmpp_verify_con){
					XmppSetVerify(to, from, XMPP_STATE_VERIFY_OK, xmpp_verify_con->xmpp_verify_con);
					XmppSetVerify(from, to, XMPP_STATE_VERIFY_OK, xmpp_verify_con);
				}

				it.Format("<db:verify from='%s' to='%s' id='%s' type='%s' />", to, from, el->GetLVal("id"), ver ? "valid" : "invalid");
				SendData(wel, it);
			}
		}

		else if(act == "db:result !!!"){
			if(!el->GetLVal("type")){
				UGLOCK(storm_items_list);
				listen_xml2s *con;

				// send my result
				if(!XmppIsVerifyDomain(to, from)){
					it.Format("<db:result from='%s' to='%s'>%s</db:result>", to, from, XmppGenerateVerifyCode(to, from));
					//SendData(wel, it);
				}

				it.Format("<db:verify from='%s' to='%s'>%s</db:verify>", to, from, el->v());

				if(con = XmppSend(to, from, it)){
					//print(LString()+" >>>>> (" + (int)(!xmpp_state) + ") " + it + "\r\n\r\n");
					con->xmpp_verify_con = this;
				}
				else
					XmppConnect(to, from, this, el->v());
			}
			else if(el->GetLVal("type") == "valid" && xmpp_verify == XMPP_STATE_VERIFY_PRE)
				xmpp_verify = XMPP_STATE_VERIFY_OK;
		}
		else if(act == "db:verify !!!"){
			if(!el->GetLVal("type")){
				UGLOCK(storm_items_list);

				int ver = el->v() == XmppGenerateVerifyCode(to, from);
				if(ver)
					XmppSetVerify(to, from, XMPP_STATE_VERIFY_PRE);

				it.Format("<db:verify from='%s' to='%s' id='%s' type='%s' />", to, from, el->GetLVal("id"), ver ? "valid" : "invalid");
				SendData(wel, it);
			}
		}

		// 
		else if(act == "message"){
			CallMsg(el);
		}

		else if(act == "presence"){
			if(el->GetLVal("type") != "unavailable")
				CallRoomOpen(el);
			else
				CallRoomClose(el);
		}

		return ;
	}

	void SendData(storm_work_el &wel, VString line){
		print(LString()+" >>>>> (" + xmpp_state + ") " + line + "\r\n\r\n");
		wel.send(line);
		return ;
	} 

	static int XmppConnect(VString mydomain, VString domain, listen_xml2s *con = 0, VString verify_code = VString());

	static MString GetVerifyCode(VString domain);

	static MString XmppGenerateVerifyCode(VString from, VString to){
		MString ret;
		ret.Add(md5h(ret.Add(_xmpp_verify_uniq_string, from, _xmpp_verify_uniq_string, to, _xmpp_verify_uniq_string)), md5h(ret.Add(to, from, "zverus", _xmpp_verify_uniq_string)));
		return ret;
	}

	static int AsyncCon(VString from, VString to, listen_xml2s *con){
		UGLOCK(storm_items_list);

		listen_xml2s *p = 0;
		while(p = storm_items_list.Next(p)){
			if(p->from == to && p->to == from && !p->xmpp_verify_con){
				p->xmpp_verify_con = con;
				con = p->xmpp_verify_con;
				return 1;
			}
		}

		return 0;
	}

	static int XmppIsConnection(VString from, VString to){
		UGLOCK(storm_items_list);

		listen_xml2s *p = 0;
		while(p = storm_items_list.Next(p)){
			if(p->from == from && p->to == to)
				return 1;
		}

		return 0;
	}

	static int XmppIsVerifyDomain(VString from, VString to){
		UGLOCK(storm_items_list);

		listen_xml2s *p = 0;
		while(p = storm_items_list.Next(p)){
			if(p->from == from && p->to == to && p->xmpp_verify)
				return 1;
		}

		return 0;
	}

	static int XmppSetVerify(VString from, VString to, int st, listen_xml2s *con = 0){
		UGLOCK(storm_items_list);

		listen_xml2s *p = 0;
		while(p = storm_items_list.Next(p)){
			if(p->from == from && p->to == to && (!con || p->xmpp_verify_con == con)){
				if(st == XMPP_STATE_VERIFY_UP){
					if(p->xmpp_verify == XMPP_STATE_VERIFY_PRE){
						p->xmpp_verify = XMPP_STATE_VERIFY_OK;
					}
				}else
					p->xmpp_verify = st;

				print("[][][][][] Verified: ", from, " => ", to, ". State: ", itos(p->xmpp_verify), "  [][][][][]\r\n\r\n");

				return 1;
			}
		}

		return 0;
	}

	static listen_xml2s* XmppSend(VString from, VString to, VString line, listen_xml2s *con = 0){
		UGLOCK(storm_items_list);

		listen_xml2s *p = 0;
		while(p = storm_items_list.Next(p)){
			if(p->from == from && p->to == to && (!con || p == con)){
				print("[SEND] (", itos(p->xmpp_state), ") ", line, "\r\n\r\n");
				p->sock.SendData(line);
				return p;
			}
		}

		return 0;
	}

	static listen_xml2s* XmppCanSend(VString from, VString to){
		VString fd = PartLineO(PartLineOT(from, "@"), "/");
		VString td = PartLineO(PartLineOT(to, "@"), "/");
		SString it;

		UGLOCK(storm_items_list);

		//if(td.incompare("conference."))
		//	td = td.str(11);
		int tm = time();
		int wait = 0;		

		listen_xml2s *p = 0;
		while(p = storm_items_list.Next(p)){
			if(p->from == fd && p->to == td){
				if(p->xmpp_verify == XMPP_STATE_VERIFY_OK)
			//if(p->from == td && p->to == fd && p->xmpp_verify == XMPP_STATE_VERIFY_OK){
					return p;
				else
					if(p->ltime > tm - 10)
						wait = 1;
			}
		}

		if(wait)
			return 0;

		XmppConnect(fd, td);
		return 0;
	}

	virtual void CallMsg(XDataXmlEl *el){
		if(events)
			events->CallMsg(el);
	}

	virtual void CallRoomOpen(XDataXmlEl *el){
		if(events)
			events->CallRoomOpen(el);
	}

	virtual void CallRoomClose(XDataXmlEl *el){
		if(events)
			events->CallRoomClose(el);
	}

	static int XmppSendMsg(VString from, VString to, VString msg){
		listen_xml2s *p = XmppCanSend(from, to);
		SString s;

		if(!p)
			return 0;
		
		s.Format("<message from='%s' to='%s' type='groupchat'><body>%s</body></message>", from, to, msg);
		p->sock.SendData(s);
		//print(" *** > ", s, "\r\n");
		print(LString()+" ***>> (" + p->xmpp_state + ") " + s + "\r\n\r\n");

		return 1;
	}

	static int XmppSendMsgPriv(VString from, VString to, VString msg){
		listen_xml2s *p = XmppCanSend(from, to);
		SString s;

		if(!p)
			return 0;
		
		s.Format("<message from='%s' to='%s' type='chat'><body>%s</body><x xmlns='http://jabber.org/protocol/muc#user'/></message>", from, to, msg);
		p->sock.SendData(s);
		print(LString()+" ***>> (" + p->xmpp_state + ") " + s + "\r\n\r\n");

		return 1;
	}

	static int XmppOpenRoom(VString from, VString to){
		listen_xml2s *p = XmppCanSend(from, to);
		SString s;

		if(!p)
			return 0;
		
		s.Format("<presence from='%s' to='%s'><priority>0</priority><x xmlns='http://jabber.org/protocol/muc' /><c xmlns='http://jabber.org/protocol/caps' /><status>%s</status><x xmlns='http://jabber.org/protocol/muc#user'></x></presence>"
					, from, to, "This is port from Telegram Bot to xmpp.");
		p->sock.SendData(s);
		//print(" *** > ", s, "\r\n");
		print(LString()+" ***>> (" + p->xmpp_state + ") " + s + "\r\n\r\n");

		return 1;
	}

	static int XmppCloseRoom(VString from, VString to){
		listen_xml2s *p = XmppCanSend(from, to);
		SString s;

		if(!p)
			return 0;
		
		s.Format("<presence from='%s' to='%s' type='unavailable'></presence>"
					, from, to, "test");
		p->sock.SendData(s);
		print(" *** > ", s, "\r\n");

		return 1;
	}

	STORM_DEFINE_NEW_DELETE(listen_xml2s);
};

STORM_DEFINE_NEW_DELETE_OK(listen_xml2s);

listen_xml2s* listen_xml2s::events = 0;

// type: 0 - clave, 1 - master
int listen_xml2s::XmppConnect(VString from, VString to, listen_xml2s *con, VString verify_code){
	ConIp ip(GetIP(to), 5269);
	SOCKET sock=ip.Connect();
	if(sock){
		UGLOCK(storm_items_list);
		listen_xml2s *xmpp = static_storm_new();
		
		xmpp->xmpp_state = 1;
		xmpp->from = from;
		xmpp->to = to;
		xmpp->xmpp_verify = 0;
		xmpp->xmpp_verify_code = verify_code;
		//xmpp->xmpp_verify_con = 0;

		if(con){
			con->xmpp_verify_con = xmpp;
			xmpp->xmpp_verify_con = con;
		}

		//xmpp->xmpp_state = 1 + ((type&2) ? 0 : 2 );
		//xmpp->xmpp_type = type;
		//xmpp->xmpp_state = 1;
//		xmpp->con = con;
//		if(con)
//			con->con = xmpp;
		
		//if(type == 1)
		//	XmppVerified.AddConnect(mydomain, domain, xmpp);
		//else
		//	XmppVerified.AddConnect(domain, mydomain, con);

		SString it;
		it.Format("<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:server' xmlns:db='jabber:server:dialback' from='%s' to='%s' version='1.0'>", from, to);
		send(sock, it, it, 0);
		print(LString()+" >>>>> (" + 1 + ") " + it + "\r\n\r\n");

		return MyStormCore.StormAddSock(xmpp, sock, 0);
	}

	return 0;
}
