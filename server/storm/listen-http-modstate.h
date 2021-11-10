template<int statksize>
class listen_http_msring{
	int64 ring[statksize];
	int test;
	int ltm, pos, sz, size;

public:
	listen_http_msring(){
		memset(ring, 0, sizeof(ring));
		ltm = 0;
		pos = 0;
		sz = 0;
		size = sizeof(ring) / sizeof(int64);
		test = 0xDEADF00D;
	}

	void Add(int tm = time(), int count = 1){
		int tltm = ltm;
		int tpos = pos;

		if(tm != ltm){
			pos ++;
			int s = tm - ltm, m;
			//s --;

			if(s <= 0)
				s = 1;

			m = minel(s, size - pos);
			
			if(s >= size){
				pos = 0;
				sz = 1;
				ring[0] = 0;
				//memset(ring, 0, size * sizeof(int));
			}
			//else if(!s){
			//	pos++;
			//}
			else{
				memset(ring + pos, 0, m * sizeof(int));
				pos += s - 1;
				sz += s;

				if(sz > size)
					sz = size;

				if(pos >= size){
					s -= m;
					memset(ring, 0, s * sizeof(int));
					pos -= size;
				}

	/*			if(s + pos >= size){
					s -= m;
					memset(ring, 0, s * sizeof(int));
					pos = s;
				} else
					pos += s - 1;

				sz += s;*/


			}

			//if(pos >= size)
			//	pos -= size;

			ltm = tm;
		}

		ring[pos] += count;

		if(size != 60)
			int errr = 1;

		if(test != 0xDEADF00D)
			int errr = 1;

	}

	void Get(LString &ls, int step = 1){
		int tm = ltm - sz;

		//ls + "([";
		//ls + tm + ":";

		int p = (pos + size - sz + 1) % size, s = sz, f = 1;

		if(sz < 2){
			ls + "[" + ((tm - 1) * step) + "000," + 0 + "]";
			f = 0;
		}

		while(s --){
			if(p >= size)
				p = 0;

			if(!f)				
				ls + ", ";
			else
				f = 0;

			ls + "[" + (tm * step) + "000," + ring[p] + "]";
			
			p ++; tm ++;
		}

		//ls + "]);";

		return ;
	}

	//int Get(){
	//	int cnt = 0;

	//	// ltm - size -> ltm
	//	if(tm < ltm && tm + s > ltm - size){
	//		if(tm <= ltm)
	//			s -= ltm - tm;
	//		else{
	//			s -= tm - ltm;
	//		}


	//	}

	//	return cnt;		
	//}

};

#define MSRC_NULL		0xc0de	// msrc_null_code
#define MSRC_ACCEPT		1		// sock, client ip:port, this ip:port.
#define MSRC_CLOSE		2		// sock
#define MSRC_SOCKS		3		// accepted, connected, closed
#define MSRC_HTTP_REQ	4		// sock, * GET
#define MSRC_STATE		5		// state_http_req, state_recv_all, state_send_all

unsigned int msrc_null_code = 0xc0de;
uint64 msrc_null_test = 0xc04eda1a1e311e31; // 0xc04e da1a 1e31 1e31 // code: core data test test

class modstate_ring_buffer : public TLock{
	MString data;
	TLock lock;
	unsigned int pos;

public:

	modstate_ring_buffer(){
		data.Reserve(S1M);
		pos = 0;
	}

	int Write(VString line){
		if(!line || line > S1M - 16)
			return 0;

		ALOCK(this);
		int again = 0;

		if(pos == 0)
			again = 1;

		int f = data.sz - pos;
		int s = minel(f, line.sz);

		memcpy(data.data + pos, line, s);
		pos += s;
		
		if(pos >= data.sz)
			pos = 0;
		
		if(line.sz != s){
			return Write(line.str(s));
		}

		if(again){
			again = 0;
			Write(VString((char*)&msrc_null_code, 4));
			Write(VString((char*)&msrc_null_test, 8));
		}

		return 1;
	}

	int Read(unsigned int from, VString line){
		if(from >= data.sz)
			from = from % data.sz;

		ALOCK(this);

		int s = (pos >= from) ? pos - from : data.sz - from;
		int r = minel(s, line.sz);
		
		memcpy(line, data.data + from, r);

		if(pos < from && r < line.sz)
			return Read(from + r, line.str(r)) + r;

		return r;
	}
};

class _listen_http_modstate : public TLock{
	// Activate State
	int state_on;
	
	// Connect, Close
	int64 state_con, state_close;
	int64 state_http_req;
	int64 state_recv_all, state_send_all;

	// http request ring
	listen_http_msring<120> r_httpreq_hour, r_httpreq_min, r_httpreq_sec;
	listen_http_msring<120> r_recv_hour, r_recv_min, r_recv_sec
	, r_send_hour, r_send_min, r_send_sec;

	// Core
	int64 sock_accept, sock_conn, sock_close;

	// Ring
	modstate_ring_buffer ring;
	unsigned int ring_time;

	//struct hr_ring{ };
	//int hrring[60];
	//int hrring_tm, hrring_pos;


#define LISTEN_HTTP_MODSTATE_ON// if(!state_on) return ;

public:
	
	_listen_http_modstate(){
		state_on = 0;

		state_con = 0;
		state_close = 0;
		state_http_req = 0;

		state_recv_all = 0;
		state_send_all = 0;

//		hrring_tm = 0;
//		hrring_pos = 0;
//		memset(hrring, 0, sizeof(hrring));

		sock_accept = 0;
		sock_conn = 0;
		sock_close = 0;

		//WriteRing0();
	}

	int OnOff(){
		return state_on;
	}

	void OnOff(int v){
		state_on = v;
	}

	// Ring
	struct modstate_ring_struct{
		unsigned int code;
		unsigned char data[S1K];
		unsigned int sz;
	};

	void WriteRing(unsigned int code, VString data){
		modstate_ring_struct rs;

		// Set
		rs.code = code;
		rs.sz = data.sz;

		if(rs.sz > S1K)
			rs.sz = S1K;
		
		memcpy(rs.data, data, data.sz);
		rs.data[rs.sz] = 0;

		// Write
		ring.Write(VString((char*)&rs, rs.sz + 4));

		// Again
		//if(ring.IsAgain()){
		//	ring.NoAgain();
		//	WriteRing0();
		//}

		if(ring_time + 60 < time())
			OnWriteTime();
	}

	//void WriteRing0(){
		//VString ring0 = "RING NULL CODE ~!@#$%^&*()_+";

		//SString ss;
		//ss.Add(VString((char*)&ring0.sz, 4), ring0);

		
	//}

	void OnAccept(SOCKET sock){
		ALOCK(this);
		sock_accept ++;
		sock_conn ++;

//		listen_http_sendstatus status;
		unsigned int cip, tip;
		unsigned short cport, tport;
		SString ss;

		gettip(sock, cip, cport);
		getcip(sock, tip, tport);

		ss.Add(VString((char*)&sock, 4), VString((char*)&cip, 4), VString((char*)&cport, 2),VString((char*)&tip, 4), VString((char*)&tport, 2));

		WriteRing(MSRC_ACCEPT, ss);
	}

	void OnDisconnect(SOCKET sock){
		ALOCK(this);
		sock_close ++;
		sock_conn --;

		VString line = VString((char*)&sock, 4);

		WriteRing(MSRC_CLOSE, line);
	}

	void OnWriteTime(){
		ALOCK(this);
		ring_time = time();

		SString ss;

		ss.Add(VString((char*)&sock_accept, 8), VString((char*)&sock_conn, 8), VString((char*)&sock_close, 8));
		WriteRing(MSRC_SOCKS, ss);

		// #define MSRC_STATE		5	// state_http_req, state_recv_all, state_send_all
		ss.Add(VString((char*)&state_http_req, 8), VString((char*)&state_recv_all, 8), VString((char*)&state_send_all, 8));
		WriteRing(MSRC_STATE, ss);
	}

	void OnHttpRequest(SOCKET sock, VString site, VString line){
		SString ss;

		if(site.sz > 500)
			site.sz = 500;
		if(line.sz > 500)
			line.sz = 500;

		ss.Add(VString((char*)&sock, 4), VString((char*)&site.sz, 4), site, VString((char*)&line.sz, 4), line);
		WriteRing(MSRC_HTTP_REQ, ss);
	}

	unsigned int OnRead(unsigned int pos, VString line){
		return ring.Read(pos, line);
	}

	//
	void OnConnect(){//SOCKET sock, unsigned int ip){
		LISTEN_HTTP_MODSTATE_ON;

		UGLOCK(this);
		state_con ++;
	}

	void OnClose(){
		LISTEN_HTTP_MODSTATE_ON;

		UGLOCK(this);
		state_close ++;
	}

	void OnRecv(int sz){
		LISTEN_HTTP_MODSTATE_ON;

		if(sz <= 0)
			return ;
		
		UGLOCK(this);

		state_recv_all += sz;

		int tm = time();
		r_recv_hour.Add(tm / 3600, sz);
		r_recv_min.Add(tm / 60, sz);
		r_recv_sec.Add(tm, sz);

		return ;
	}

	void OnSend(int sz){
		LISTEN_HTTP_MODSTATE_ON;

		if(sz <= 0)
			return ;
		
		UGLOCK(this);

		state_send_all += sz;

		int tm = time();
		r_send_hour.Add(tm / 3600, sz);
		r_send_min.Add(tm / 60, sz);
		r_send_sec.Add(tm, sz);

		return ;
	}

	void OnHttpRequest(){
		LISTEN_HTTP_MODSTATE_ON;

		UGLOCK(this);
		state_http_req ++;

		int tm = time();
		r_httpreq_hour.Add(tm / 3600);
		r_httpreq_min.Add(tm / 60);
		r_httpreq_sec.Add(tm);

		//int tm = time();
		//if(hrring_tm == tm)
		//	hrring[hrring_pos] ++;
		//else{
		//	if(tm - hrring_tm < (sizeof(hrring) / sizeof(int))){
		//		int m = minel(tm - hrring_tm, sizeof(hrring) / sizeof(int) - hrring_pos);
		//		memset(hrring + hrring_pos, 0, m * sizeof(int));
		//		
		//		if(m != tm - hrring_tm){
		//			m = tm - hrring_tm - m;
		//			memset(hrring, 0, m * sizeof(int));
		//			hrring_pos = m;
		//		}
		//		
		//		if(hrring_pos == sizeof(hrring) / sizeof(int))
		//			hrring_pos = 0;
		//	}
		//	else
		//		memset(hrring, 0, sizeof(hrring));

		//	hrring_tm = tm;
		//	hrring[hrring_pos] ++;

		//	//hrring_pos = ++hrring_pos % (sizeof(hrring) / sizeof(int));
		//}
	}

#define GetStateDataHttpRequestQ(key, val) if(seestate == #key) r_ ## key .Get(ls, val);

	MString GetStateDataHttpRequest(VString seestate, VString callback){
		VString key, val;
		key = PartLine(callback, val, "=");

		UGLOCK(this);

		LString ls;
		ls + key + "([";

		if(seestate == "hr_hour")
			r_httpreq_hour.Get(ls, 3600);
		else if(seestate == "hr_min")
			r_httpreq_min.Get(ls, 60);
		else if(seestate == "hr_sec")
			r_httpreq_sec.Get(ls);

		GetStateDataHttpRequestQ(recv_hour, 3600);
		GetStateDataHttpRequestQ(recv_min, 60);
		GetStateDataHttpRequestQ(recv_sec, 1);

		GetStateDataHttpRequestQ(send_hour, 3600);
		GetStateDataHttpRequestQ(send_min, 60);
		GetStateDataHttpRequestQ(send_sec, 1);

		ls + "]);";

		return ls;
		//return ret.Add(key, "(/* AAPL historical OHLC data from the Google Finance API */ [ [1215388800000,25.02], [1215475200000,25.65], [1215561600000,24.89], [1215648000000,25.23], [1215734400000,24.65], [1215993600000,24.84], [1216080000000,24.23], [1216166400000,24.69], [1216252800000,24.54], [1216339200000,23.59], [1216598400000,23.76], [1216684800000,23.15], [1216771200000,23.75], [1216857600000,22.72], [1216944000000,23.16], [1217203200000,22.06], [1217289600000,22.44], [1435708800000,126.60], [1435795200000,126.44] ]);");
	}

	MString GetState(VString url, SString &ithead){
		LString ls;
		SString it;
		ILink il;
		
		il.Link(url);

		if(VString file = il.GetArg("file")){
			ithead = "Content-type: application/x-javascript\r\nPragma: no-cache\r\n";

			if(file == "highstock.js")
				return LoadFile("mods/state/highstock.js");
			if(file == "exporting.js")
				return LoadFile("mods/state/exporting.js");
			
			//if(file == "httprequestdata")
			//	return LoadFile("mods/state/httprequestdata");

				if(file == "httprequestdata")
					return GetStateDataHttpRequest(il.GetArg("seestate"), il.GetArg("callback"));

			return VString();
		}

		ithead = "Content-type: text/html\r\nPragma: no-cache\r\n";

		ls + "<title>Storm Http | Mod State</title>";
		ls + "[ModState " + (state_on ? "On" : "Off") + "]<p>";
		
		int tm = time(), tmu =  tm - msvcorestate.procupped;
		it.Reserve(prmf_uptime(0, 0, tmu));
		prmf_uptime(it, it, tmu);
		ls + "Uptime: " + it + "<br>";
		
		if(!state_on)
			return ls;

		ls + "Connections: " + state_con + " / " + (state_con - state_close) + " / " + state_close + " (con/this/close).<br>";
		ls + "HTTP Requests: " + state_http_req + " See state: <a href='?seestate=hr_hour'>Hour<a/> | <a href='?seestate=hr_min'>Minute<a/> | <a href='?seestate=hr_sec'>Sec<a/><br>";
		ls + "Traffic recv: " + state_recv_all + " See state: <a href='?seestate=recv_hour'>Hour<a/> | <a href='?seestate=recv_min'>Minute<a/> | <a href='?seestate=recv_sec'>Sec<a/><br>";
		ls + "Traffic send: " + state_send_all + " See state: <a href='?seestate=send_hour'>Hour<a/> | <a href='?seestate=send_min'>Minute<a/> | <a href='?seestate=send_sec'>Sec<a/><br>";

		VString seestate = il.GetArg("seestate");
		int ok = seestate;

		for(unsigned int i = 0; i < seestate; i ++){
			if(seestate[i] != '_' && seestate[i] < 'a' || seestate[i] > 'z')
				ok = 0;
		}

		if(ok){
			MString d = LoadFile("mods/state/http_request.html");
			ls + Replace(d, "$SEESTATE", seestate);
		}

	//	UGLOCK(this);
		//ls + "<br>Http Request detail:<br>";

		//ls + 
		
//		ls + "<script type='text/javascript' src='http://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js'></script>"
//		"<script type='text/javascript'>"
//		"$(function () { $.getJSON('?file=httprequestdata', function (data) {"
//        // Create the chart
//        "$('#container').highcharts('StockChart', {"
//		"rangeSelector : { selected : 1 },"
//		"title : { text : 'AAPL Stock Price' },"
//
//        "series : [{ name : 'AAPL', data : data, tooltip: { valueDecimals: 2 } }]"
//        "}); }); });"
//
//		"</script>"
//"<script src='?file=highstock.js'></script>"
//"<script src='?file=exporting.js'></script>"
//"<div id='container' style='height: 400px; min-width: 310px'></div>";


//		ls + "<script src='http://code.highcharts.com/highcharts.js'></script>"
//"<script src='http://code.highcharts.com/modules/exporting.js'></script>"
//"<div id='container' style='min-width: 310px; height: 400px; margin: 0 auto'></div>";
//
//		ls + "<script>"
//		"$(function () { $('#container').highcharts({ title: { text: 'Monthly Average Temperature', x: -20 },"
//        "subtitle: { text: 'Source: WorldClimate.com', x: -20 },"
//        "xAxis: { categories: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'] },"
//        "yAxis: { title: { text: 'Temperature (°C)' }, plotLines: [{ value: 0, width: 1, color: '#808080' }] },"
//        "tooltip: { valueSuffix: '°C' },"
//        "legend: { layout: 'vertical', align: 'right', verticalAlign: 'middle', borderWidth: 0 },"
//        "series: [{"
//		"name: 'Tokyo', data: [7.0, 6.9, 9.5, 14.5, 18.2, 21.5, 25.2, 26.5, 23.3, 18.3, 13.9, 9.6] },"
//		"{ name: 'New York', data: [-0.2, 0.8, 5.7, 11.3, 17.0, 22.0, 24.8, 24.1, 20.1, 14.1, 8.6, 2.5] }, {"
//         " name: 'Berlin', data: [-0.9, 0.6, 3.5, 8.4, 13.5, 17.0, 18.6, 17.9, 14.3, 9.0, 3.9, 1.0] }, {"
//         " name: 'London', data: [3.9, 4.2, 5.7, 8.5, 11.9, 15.2, 17.0, 16.6, 14.2, 10.3, 6.6, 4.8] }]"
//		 "}); });"
//		 "</script>";

		//ls + "State. ";
		//hrring.Get(ls);
		//ls + "<p>";

		//int cnt = hrring.Get(tm - 60);	

		//int count = 0, mcount = sizeof(hrring) / sizeof(int) - (tm - hrring_tm), pos = hrring_pos + 1;
		//
		//while(mcount > 0){
		//	pos %= sizeof(hrring) / sizeof(int);
		//	count += hrring[pos];
		//	mcount --;
		//}

		//ls + "Last 60 sec: " + cnt + "<br>";

		return ls;
	}

};

#ifndef USEMSV_MSVCORE
	extern
#endif
	_listen_http_modstate listen_http_modstate;