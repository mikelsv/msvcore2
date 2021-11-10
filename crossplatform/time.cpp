// MSV Time Functions (MSVLibrary) MSV TML
//1.12.04 (build 13)
//4.12.05  1.1.0
//26.12.07  1.2.0

//#include <sys/types.h>
/*
#include <time.h>
#ifndef WINCE
#include <sys/timeb.h>
#endif
*/
//int time(){ return time(0);}
//int64 time64(){ __timeb64 tb; _ftime64(&tb); return tb.time; }

/*
unsigned int time(){ timeb tb; ftime(&tb); return (int)tb.time; }
timeb alltime(){ timeb tb; ftime(&tb); return tb; }
int timemi(timeb&o, timeb&t){ return (int)(o.time-t.time)*1000+o.millitm-t.millitm; };
int timemit(timeb&t){ timeb o; ftime(&o); return (int)(o.time-t.time)*1000+o.millitm-t.millitm; };
int64 sectime(){ timeb ft; ftime(&ft); return int64(ft.time)*1000+ft.millitm; }
*/

//*// > IString.h
//short dmonths[]={0,1,32,60,91,121,152,182,213,244,274,305,335};
//short dmonthsv[]={0,1,32,61,92,122,153,183,214,245,275,306,336};
short dmonths[]={0,0,31,59,90,120,151,181,212,243,273,304,334};
short dmonthsv[]={0,0,31,60,91,121,152,182,213,244,274,305,335};

#define MTIME_MTDAYS_USE mtdays
#define MTIME_MTMONTH_USE mtmonths
#define MTIME_MTPDAYS_USE mtpdays_ru

VString mtmonths[]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
VString mtdays[]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

VString mtmonths_ru[]={"������", "�������", "����", "������", "���", "����", "����", "������", "��������", "�������", "������", "�������"};
VString mtdays_ru[]={"�����������", "�����������", "�������", "�����", "�������", "�������", "�������"};
VString mtpdays_ru[]={"���������", "�����", "�������", "������", "�����������"};
//*/


double timeg(){
	timeb tb; ftime(&tb);
	return tb.time+(double)tb.millitm/1000;
}


/*
// MSV Time Library
//unsigned int time(){ timeb tb; ftime(&tb); return tb.time; }
unsigned int time();

class MTime64{
public:
int year;
int month;
int day;
int doy;
int hour;
int minute;
int sec;
unsigned short millitm;
short timezone;
short dstflag;

int ttime;
MTime64();f
int mtime(int tm, bool gmt=0);
int time(int cl=0);
int mktime(bool gmt=0);
MString date(const VString line, unsigned int tm=-1, bool gmt=0, bool eng=0);
unsigned int mktime(int y, int m=0, int d=0, int h=0, int mi=0, int s=0, bool gmt=0);
};*/

MTime::MTime(){
	memset(this, 0, sizeof(MTime)); timeb tb; ftime(&tb);
	ttime=(int)tb.time; millitm=tb.millitm; timezone=tb.timezone; dstflag=tb.dstflag;
}

int MTime::time(int cl){ timeb tb; ftime(&tb);
	ttime=(int)tb.time; millitm=tb.millitm; timezone=tb.timezone; dstflag=tb.dstflag;
	return ttime;}

//int MTime64::settime(){ timeb tb;
//	tb.time=ttime; tb.millitm=millitm; tb.timezone=timezone; tb.dstflag=dstflag;
	//stime(&tb);
//return ttime;}

int MTime::timemit(timebm&t){ return (ttime-t.time)*1000+millitm-t.millitm; };
int64 MTime::timemitx(timebm&t){ return int64((ttime-t.time))*1000+millitm-t.millitm; };
int MTime::timemits(timebm&t){ return (ttime-t.time); };

int MTime::mtime(int tm, bool gmt){
if(tm) ttime=tm; else tm=ttime;
if(!gmt){ tm-=timezone*60; tm+=dstflag*3600; }
day=tm/86400;
year=(day-(day/365+1)/4)/365;
sec=tm-(year*365+(year+1)/4)*86400;

int ds=0;

//if(month==3){
//unsigned int t=year*365+(year+1)/4+62+((year+2)%4==0 ? 29 : 28)+4;
//if(tm>=(t-4 - (t%7 ? t%7 : 7) )*86400+3600*2){ ds=1;
//
////unsigned int t=year*365+(year+1)/4+276+((year+2)%4==0 ? 29 : 28)+3;
//t=year*365+(year+1)/4+276+((year+2)%4==0 ? 29 : 28)+4;
////	if(tm>=(t-(t)%7-4)*86400+3600*2) ds=0;
//	if(tm>=(t-4 - (t%7 ? t%7 : 7) )*86400+3600*2) ds=0;
//if(ds) sec+=3600;
//}

doy=sec/86400; day=doy;

month=1;
while(1){
	if(day<31) {break;} day-=31; month++;
	minute=28+((year+2)%4==0);// ? 29 : 28;
	if(day<minute) {break;} day-=minute; month++;
	if(day<31) {break;} day-=31; month++;
	if(day<30) {break;} day-=30; month++;
	if(day<31) {break;} day-=31; month++;
	if(day<30) {break;} day-=30; month++;
	if(day<31) {break;} day-=31; month++;
	if(day<31) {break;} day-=31; month++;
	if(day<30) {break;} day-=30; month++;
	if(day<31) {break;} day-=31; month++;
	if(day<30) {break;} day-=30; month++;
	if(day<31) {break;} day-=31; month++;
	year++; month=1; doy=day;
} day++;

year+=1970;

sec-=doy*86400;
minute=sec/60%60;
hour=sec/60/60; sec%=60;
return 1;
}

int MTime::mktime(bool gmt){ return mktime(year, month, day, hour, minute, sec, gmt); }

unsigned int MTime::mktime(int y, int m, int d, int h, int mi, int s, bool gmt){
	year=y; month=m; day=d; hour=h; minute=mi; sec=s;
	unsigned tm=year-1970; if(!year) tm=0;
	tm=(tm*365+(tm+1)/4);
	tm+=((year-1970+2)%4==0 ? dmonthsv[month] : dmonths[month] )+d-1; // if(month>0) tm+=31; //tm+=day-1;
	tm*=24*60*60; tm+=hour*3600+minute*60+sec; //tm*=60; tm+=minute; //tm*=60; tm+=sec;
	int ds=0;

//if(month==3 && !gmt){
//unsigned int t=((year-1970)*365+(year-1970+1)/4)+62+((year-1970+2)%4==0 ? 29 : 28)+4;
//	if(tm>=(t-4 - (t%7 ? t%7 : 7) )*86400+3600*2) ds=1;
//} else if(month>3) ds=1;
//
//if(ds){
//	if(month==10 && !gmt){
//unsigned int t=((year-1970)*365+(year-1970+1)/4)+276+((year-1970+2)%4==0 ? 29 : 28)+4;
//	if(tm>=(t-4 - (t%7 ? t%7 : 7) )*86400+3600*3) ds=0;
//} else if(month>10) ds=0;}
//if(ds) tm-=3600;
	
	if(!gmt){ tm+=timezone*60; tm-=dstflag*3600;}
return ttime=tm;
}

int MTime::weekday(){
	return ((year-1970)*365+(year-1970+1)/4+doy+3)%7;
}

int MTime::monthdays(int month){ if(month>12) month-=12;
	if(!month) month=this->month; if(!month) return 0;
	switch(month){
		case 2: return 28+((year+2)%4==0);
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			return 31;
		case 4: case 6: case 9: case 11:
			return 30; 
	}
	return 0;
}

unsigned int MTime::gmttotime(VString tline){
	// Friday, 31-Dec-99 23:59:59 GMT
	// Thu, 01 Jan 1970 00:00:01 GMT
	// Fri, 12 Jan 2018 12:22:50 +0300

	VString line = PartLineOT(tline, ","), v;
	dspacev(line, 7);

	// dmy
	v = PartLineTwo(line, line, " ", "-");
	day = v.toi();

	v = PartLineTwo(line, line, " ", "-");

	for(int i = 0; i < 12; i ++)
		if(VString(mtmonths[i].data, 3) == v){ //cmp(mtmonths[i].data, v, 3)){
			month = i + 1;
			break;
		}

	v = PartLineTwo(line, line, " ", "-");
	if(v.size() == 2){
		year = v.toi();
		year += year < 70 ? 2000 : 1900;
	} else if(v.size() == 4){
		year = v.toi();
	} else
		return 0;

	v = PartLineTwo(line, line, " ", ":");
	hour = v.toi();

	v = PartLineTwo(line, line, " ", ":");
	minute = v.toi();

	v = PartLineTwo(line, line, " ", ":");
	sec = v.toi();

	return mktime(year, month, day, hour, minute, sec, 1);
}

MString MTime::date(const VString line, unsigned int tm, bool gmt, bool eng, BYTE c){
	if(tm!=-1) mtime(tm, gmt); else mtime(time(), gmt); if(!year) mtime(time(), gmt);
	MString ret; ret.Reserve(rpmf_date(0, 0, line, tm, gmt, eng, c)); rpmf_date(ret, ret, line, tm, gmt, eng, c); return ret;
}


unsigned int MTime::rpmf_date(unsigned char *ret, unsigned int rsz, const VString line, unsigned int tm, bool gmt, bool eng, BYTE c){
	unsigned char *aret=ret, *rt=ret ? ret+rsz : 0; // define
	unsigned char *ln=line, *to=line.endu();

	//LString ret; Itos it;

	//for(unsigned int i=0; i<line.size(); i++){
	for(ln; ln<to; ln++){
		if(c){
			if(*ln!=c){ if(ret<rt) *ret++=*ln; continue; }
			ln++;
			continue;
		}

		switch(*ln){
		case 'a': if(ret+2<rt){ *ret++= hour<12 ? 'a' : 'p'; *ret++='m'; } else ret+=2; break;
		case 'A': if(ret+2<rt){ *ret++= hour<12 ? 'a' : 'p'; *ret++='M'; } else ret+=2; break; //ret+=hour<12 ? "AM" : "PM"; break;
		// b
		case 'd': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, day, 10, 2); break;
		case 'D': if(!eng){ if(ret+3<rt){ memcpy(ret, MTIME_MTDAYS_USE[((year-1970)*365+(year-1970+1)/4+doy+4)%7], 3); } ret+=3; } //  ret+=VString(MTIME_MTDAYS_USE[((year-1970)*365+(year-1970+1)/4+doy+4)%7]).Left(3); 
			else { if(ret+3<rt){ memcpy(ret, mtdays[((year-1970)*365+(year-1970+1)/4+doy+4)%7], 3); } ret+=3; }
			break;
				//ret+=VString(mtdays[((year-1970)*365+(year-1970+1)/4+doy+4)%7]).Left(3); break; // day name
/*		case 'F': if(!eng) ret+=MTIME_MTMONTH_USE[month-1]; else ret+=mtmonths[month-1]; break; // month name
*/		case 'g': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, hour-(hour>11 ? 12 : 0)); break;
		case 'h': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, hour-(hour>11 ? 12 : 0), 10, 2); break;
		case 'G': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, hour); break;
		case 'H': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, hour, 10, 2); break;
		case 'i': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, minute, 10, 2); break;
		//i
/*		case 'l': if(!eng) ret+=MTIME_MTDAYS_USE[((year-1970)*365+(year-1970+1)/4+doy+4)%7];
			else ret+=mtdays[((year-1970)*365+(year-1970+1)/4+doy+4)%7]; break; // day name
		case 'J': ret+=it.itos(day); break;
		case 'L': ret+=(year-2%4)? "0" : "1"; break;
*/		case 'm': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, month, 10, 2); break;
		case 'M': if(!eng){ if(ret+3<rt){ memcpy(ret, MTIME_MTMONTH_USE[month-1], 3); } ret+=3; }
				  else{ if(ret+3<rt){ memcpy(ret, mtmonths[month-1], 3); } ret+=3; }
				  break;
/*		//M
		case 'p':{ int d=(::time()-::time()%86400-tm+86400)/86400;
			if(!eng) if(d<3) ret+=MTIME_MTPDAYS_USE[2-d];
				   else ret+=MTIME_MTDAYS_USE[((year-1970)*365+(year-1970+1)/4+doy+4)%7];
			else if(d<3) ret+=mtdays[2-d];
				   else ret+=mtdays[((year-1970)*365+(year-1970+1)/4+doy+4)%7]; break; // day name
			break;}
*/	//	case 'n': ret+=it.itos(month-1); break;
		case 'O':
			if(timezone<=0){ if(ret<rt){ *ret='+'; } ret++; }
			ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, -timezone/60, 10, 2);
			ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, dstflag, 10, 2);
			break;
		case 'r': ret+=rpmf_date(ret, ret<rt ? rt-ret : 0, "D, d M Y H:i:s O", tm, gmt, eng, c); break;
		case 's': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, sec, 10, 2); break;
		case 't': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, day); break;
		case 'Y': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, year); break;
		case 'y': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, year%100); break;
		case 'w': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, ((year-1970)*365+(year-1970+1)/4+doy+4)%7); break;
		case 'z': ret+=prmf_itos(ret, ret<rt ? rt-ret : 0, doy); break;

/*
� ������ ������� ������������ ��������� �������: 
B - ����� Swatch Internet 

I (��������� i) - "1", ���� Daylight Savings Time, "0" - � ��������� ������.
//l ('L' � ������ ��������) - ���� ������, ���������, long; ��������, "Friday" 

//M - �����, ���������, 3 �����; ��������, "Jan" 
O - ������� � �������� �� ��������, � �����; ��������, "+0200" 
r - RFC 822 ������ ����; ��������, "Thu, 21 Dec 2000 16:01:07 +0200" (����� � PHP 4.0.4) 

S - ������� ���������� ������� ��� ��� (�����) ������, 2 �������; �.�. "st", "nd", "rd" ��� "th"

T - ��������� Timezone/������� ���� �� ������ ������; ��������, "EST" ��� "MDT" 
U - ������� ����� Unix Epoch (������� � January 1 1970 00:00:00 GMT) 
w - ���� ������, ��������, �.�. �� "0" (Sunday) �� "6" (Saturday) 
W - ISO-8601 ����� ������ � ����, ������ ���������� � ������������/Monday (������� � PHP 4.1.0) 
Z - �������� �������� �����, � �������� (�.�. �� "-43200" �� "43200"). �������� ������� ������ � ������ �� UTC ������ �������������, � ��� ������ � ������� �� UTC - ������ �������������.
*/
		default:
			if(*ln=='\\' && ln+1<to){ if(ret<rt) *ret=*(ln+1); ln++; ret++; }
			else{ if(ret<rt) *ret=*ln; ret++; }
		}
	}

	return ret-aret;
}

//void genuptime(itos it


//class MTimer{
//	int64 freq;
//public:
// ������������� �������
	MTimer::MTimer(){ Init(); }
	MTimer::MTimer(int noinit){ freq = 0; }

	bool MTimer::Init(){
#ifdef WIN32
		if(!QueryPerformanceFrequency((LARGE_INTEGER*)&freq)) return 0;
#else
		//clock_gettime(CLOCK_REALTIME, &freq);
#endif
		return 1;
	}

    // ��������� ������� ������� �������
	double MTimer::dtime(){
#ifdef WIN32
		int64 li;
		if(!QueryPerformanceCounter((LARGE_INTEGER*)&li)) return 0;
		return (li * 1000.0) / freq;
#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);	
		return ts.tv_sec*1000+(double)ts.tv_nsec/1000000;
#endif
	}

	int64 MTimer::time(){
#ifdef WIN32
		int64 li;
		if(!QueryPerformanceCounter((LARGE_INTEGER*)&li)) return 0;
		return (int64)(li * 1000.0) / freq;
#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);	
		return ts.tv_sec*1000+ts.tv_nsec/1000000;
#endif
	}
//};