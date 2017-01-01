void storm_work_el::send(VString line){
	if(!line)
		return ;

	int sd = item->storm_socket_send(data->sock, line, line, 0);
	listen_http_modstate.OnSend(sd);

	if(sd > 0){
		line.data += sd;
		line.sz -= sd;
	}

	if(line)
		data->send(line);

	data->wr = 0;

	return ;
}

void storm_work_el::send(const char *line, unsigned int sz){
	return send(VString(line, sz));
}

void storm_work_el::gettip(unsigned int &ip, unsigned short &port){
	sockaddr_in from; int fromlen = sizeof(from);

	getsockname(data->sock, (struct sockaddr*)&from, (socklen_t*)&fromlen);

	ip = ntohl(from.sin_addr.s_addr);
	port = htons(from.sin_port);
}

void storm_work_el::getcip(unsigned int &ip, unsigned short &port){
	sockaddr_in from; int fromlen = sizeof(from);

	getpeername(data->sock, (struct sockaddr*)&from, (socklen_t*)&fromlen);

	ip = ntohl(from.sin_addr.s_addr);
	port = htons(from.sin_port);
}

SOCKET storm_work_el::getsock(){
	return data ? data->sock : 0;
}