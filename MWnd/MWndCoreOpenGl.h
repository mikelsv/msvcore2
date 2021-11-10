// MWnd Core OpenGl
// 0.0.0.1 25.02.2017 17:41

class MWndGl;
LRESULT WINAPI MWndGlProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);



class MWndGlInit{
	// Register Class
	ATOM MWndClassRegister(HINSTANCE hInstance){
	#ifndef WINCE
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.hIconSm		= 0;//LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
		wcex.lpszClassName	= MODUNICODET("MWNDGL");	
	#else
		WNDCLASS wcex;
		wcex.lpszClassName	= MODUNICODE("MWNDGL");
	#endif
		wcex.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

		wcex.lpfnWndProc	= (WNDPROC)MWndGlProc;


		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= 0; //LoadIcon(hInstance, (LPCTSTR)IDI_PRINTX);
		wcex.hCursor		= 0; //LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= 0; //(HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= 0;//(LPCSTR)IDC_PRINTX;

	#ifndef WINCE
		return RegisterClassEx(&wcex);
	#else
		return RegisterClass(&wcex);
	#endif	
	}

public:
	MWndGlInit(){
		MWndClassRegister(GetModuleHandle(0));
	}

} MWndGlInit;

// MWndGl
class MWndGl{
	// Window handle
	HWND hwnd;
	DWORD style;

	// Rect & Client Rect
	SRect rect, clrect;


public:
	MWndGl(){
		hwnd = 0;
		style = 0;
	}

	~MWndGl(){
		
	}

	// Create & Destroy
	virtual HWND Create(MString name, DWORD stl, DWORD exstyle, MExRect rc, MWnd *prnt=0, HINSTANCE hinstance=0, unsigned int uid=0, LPVOID lpParam=0, HWND hwparent=0){
		//UGLOCK(this);

		HWND p = 0;
		rect = rc;
		//parent = prnt;
		style = stl;

		//if(parent){
		//	p=parent->GetHWND();
		//	style|=WS_CHILD; //mstyle=parent->mstyle; /*parent->SetupElement()->elw=this;*/
		//}
		//if(hwparent){
		//	p=hwparent;
		//	style|=WS_CHILD;
		//}
		//	RecheckRect();

		hwnd = MWndCreateWindow(exstyle, MODUNICODE("MWND"), MODUNICODE(name), style, rect, p, 0, thishinstance /*hinst*/, 0 /*visual lpParam*/);
		if(!hwnd)
			return 0;

		MRect mr;
		::GetClientRect(hwnd, mr);
		clrect = mr;

		//SetCursor(mousecursor);

		HDC hdc = GetDC(hwnd);

		bool ret = MglInit(hdc, clrect.GetW(), clrect.GetH());

		//SendMessage(hwnd, WM_CREATE, 0, 0);

		return hwnd;
	}

	// Destroy Window
	virtual int DestroyWindow(){
		//UGLOCK(this);
		if(hwnd)
			::MWndDestroyWindow(hwnd);
		
		hwnd = 0;

		return 0;
	}

	// Do
	int Do(){
		MSG Message = {0}; MTimer timer;
		double old_time = timer.dtime(), new_time=0, max_time=0, frame_time=timer.dtime();// oldFPSTime = old_time;

		while(Message.message!=WM_QUIT){
			if(PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE)){
				if(GetMessage(&Message, NULL, 0, 0)) {
					TranslateMessage(&Message);
					DispatchMessage(&Message);      
				} 
			}else{
				new_time = timer.dtime();
				if(new_time - frame_time > 1000.0){
					opengl_frames=opengl_frame; opengl_frame=0;
					if(new_time - frame_time < 10000.0) frame_time+=1000.0; else frame_time=new_time;
			//	fframesCnt = framesCnt; framesCnt = 0; //oldFPSTime = new_time;
				}
				if((new_time - old_time > 1000.0 / OPENGL_MAX_FREQ) && (new_time - old_time > max_time)){
					//++opengl_frame; //++framesCnt;
					max_time=OnImpulseTime(new_time - old_time);
					if(max_time<0) max_time=OPENGL_MAX_TIME;
					//display(new_time - old_time);
					old_time = new_time;

					//g_pBump->RenderMap(rand()%200, rand()%200);
					//PostMessage(g_hMainWindow, WM_PAINT, NULL, 0);
					//OnDo();
				}
				else 
					Sleep(1);
			}
		}
		return Message.wParam;
	}

	// On
	virtual LRESULT DefWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){ return 0; }
	virtual int OnImpulseTime(int ms){
		// Draw
		//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glBegin(GL_QUADS);
		glColor3f(rand()%255/255., rand()%255/255., rand()%255/255.);
		glVertex2d(10, 10);
		glVertex2d(210, 20);
		glVertex2d(190+rand()%8-1, 130+rand()%8-1);
		glVertex2d(200, 10);
		glEnd();

		//glListBase(1000); 
		//glCallLists(24, GL_UNSIGNED_BYTE, "Hello Windows OpenGL World");

// display a string:  
// indicate start of glyph display lists  

		glListBase (1000);

		glRasterPos2d(-1.0, 0);
		glColor3f(1,0,0);
		
		glCallLists (24, GL_UNSIGNED_BYTE, "Hello Windows OpenGL World");

		SwapBuffers(wglGetCurrentDC());

		return 500;
	}

};


// MWndProc
LRESULT CALLBACK MWndGlProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
	return DefWindowProc(hwnd, message, wparam, lparam);
}



