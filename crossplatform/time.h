// MyTime

struct timebm{
	unsigned int time;
	unsigned short millitm;
	short timezone;
	short dstflag; 

	void operator=(const timeb &tm){
		time = (unsigned int)tm.time;
		millitm = tm.millitm;
		timezone = tm.timezone;
		dstflag = tm.dstflag;

		return ;
	}
};

class MTime{
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

	MTime();
	int timemit(timebm&t);
	int64 timemitx(timebm&t);
	int timemits(timebm&t);
	int mtime(int tm, bool gmt=0);
	int time(int cl=0);
	int mktime(bool gmt=0);

	int weekday();
	int monthdays(int month=0);

	int gmttotime(VString line);

	MString date(const VString line, unsigned int tm=-1, bool gmt=0, bool eng=0, BYTE c=0);
	unsigned int rpmf_date(unsigned char *ret, unsigned int rsz, const VString line, unsigned int tm, bool gmt, bool eng, BYTE c);
	unsigned int mktime(int y, int m=0, int d=0, int h=0, int mi=0, int s=0, bool gmt=0);
};

// Timer
class MTimer{
	int64 freq;
public:
	MTimer();
	MTimer(int noinit);

	bool Init();
	double dtime();
	int64 time();
};