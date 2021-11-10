// OpenGL
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"glu32.lib")
#include <GL/gl.h>   // подключаем OpenGL-заголовки
#include <GL/glu.h>

#ifndef PI
#define PI          3.1415926535897932  /* pi */
#endif

#ifndef WIN32
//#include <GL/mesa_wgl.h>
#include <GL/glx.h>
//typedef void* HGLRC;
typedef	GLXContext		HGLRC;

// emulate
HDC wglGetCurrentDC(){ return 0; }

#endif

HGLRC hGLRC; int firstopengl=3;

/*
//#include "../../Center/MWnd/Timer.h"
#ifndef OPENGL_MAX_FREQ
#define OPENGL_MAX_FREQ       30   // максимальная частота обновления экрана
#endif

#ifndef OPENGL_MAX_TIME
#define OPENGL_MAX_TIME	500
#endif

int opengl_frame=0, opengl_frames=0;
*/

void OpenGLError(GLenum err);
#define MWNDGLERROR	err=glGetError(); if(err){ OpenGLError(err); return 0; }
int MglSetWindowPixelFormat(HDC hDC);
int MglSetProjectionMatrix(int w, int h);
void MglCreateGLFont(char *fontName, int fontSize, int listBase);
void display(double frametime);

//=========================
// тут будут настройки OpenGL
int MglInit(HDC hDC, int width, int height){
	GLenum err;

	if(!MglSetWindowPixelFormat(hDC))
		return 0;

	hGLRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hGLRC);

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);

	MglSetProjectionMatrix(width, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	MWNDGLERROR

/*
    MglCreateGLFont("Tahoma", 12, 1000);

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);

	if(!MglSetProjectionMatrix(width, height))
		return 0;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(5,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	MWNDGLERROR
	*/

	return 1;
}

int MglSetProjectionMatrix(int w, int h){
	GLenum err;
	GLfloat fAspRatio = (GLfloat)h/w;
	glMatrixMode(GL_PROJECTION);
MWNDGLERROR
	glLoadIdentity();
MWNDGLERROR
	glOrtho(0, w, 0, h, 1000, -1000); // , 1000, -1000);
MWNDGLERROR
//	glViewport(0, 0, w, h);
//MWNDGLERROR
//	glMatrixMode(GL_MODELVIEW);
//MWNDGLERROR

	return 1;
}

//==========================


//===========================
#ifdef WIN32
int MglSetWindowPixelFormat(HDC hDC){
	int iGLPixelIndex;
	PIXELFORMATDESCRIPTOR pfd;

	pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion    = 1;

	pfd.dwFlags   = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	pfd.iPixelType     = PFD_TYPE_RGBA;
	pfd.cColorBits     = 32;
	pfd.cRedBits       = 0;
	pfd.cRedShift      = 0;
	pfd.cGreenBits     = 0;
	pfd.cGreenShift    = 0;
	pfd.cBlueBits      = 0;
	pfd.cBlueShift     = 0;
	pfd.cAlphaBits     = 0;
	pfd.cAlphaShift    = 0;
	pfd.cAccumBits     = 0;    
	pfd.cAccumRedBits  = 0;
	pfd.cAccumGreenBits   = 0;
	pfd.cAccumBlueBits    = 0;
	pfd.cAccumAlphaBits   = 0;
	pfd.cDepthBits        = 24;
	pfd.cStencilBits      = 0;
	pfd.cAuxBuffers       = 0;
	pfd.iLayerType        = PFD_MAIN_PLANE;
	pfd.bReserved         = 0;
	pfd.dwLayerMask       = 0;
	pfd.dwVisibleMask     = 0;
	pfd.dwDamageMask      = 0;

	iGLPixelIndex = ChoosePixelFormat( hDC, &pfd);
	if(iGLPixelIndex == 0) {
		iGLPixelIndex = 1;    
		if(DescribePixelFormat(hDC,iGLPixelIndex,sizeof(PIXELFORMATDESCRIPTOR),&pfd)==0)
			return 0;
	}
	if (SetPixelFormat( hDC, iGLPixelIndex, &pfd)==FALSE)
		return 0;
	return 1;
}
#else

static int OGLattrListDbl[] = { GLX_RGBA, GLX_DOUBLEBUFFER, 
    GLX_RED_SIZE, 4, 
    GLX_GREEN_SIZE, 4, 
    GLX_BLUE_SIZE, 4, 
    GLX_DEPTH_SIZE, 16,
    None };

int SetWindowPixelFormat(HDC hDC){
	static int attrs[] = {
	GLX_RGBA,
	GLX_DOUBLEBUFFER, // пробовал закомментировать, результата ноль
	GLX_RED_SIZE, 8,
	GLX_GREEN_SIZE, 8,
	GLX_BLUE_SIZE, 8,
	GLX_DEPTH_SIZE, 24,
	0
	};

	//XVisualInfo *vinfo = glXChooseVisual(mwnddisp, DefaultScreen(mwnddisp), attrs);	
	return 1; //vinfo!=0;
}
#endif

void OpenGLError(GLenum err){
	VString gle((char*)gluErrorString(err));
	printf("glError: %s\n", gle);
}

//=================
// создание логического фонта
#ifdef WIN32
HFONT MakeFont(HDC hDC, char* name, int size, 
               bool bold, bool italic, bool underline, bool strikeout,
               float angle){
  LOGFONT f;
  memset(&f, 0, sizeof(f));

  f.lfHeight = -MulDiv(size, GetDeviceCaps(hDC, LOGPIXELSY), 72);

  f.lfWeight =  bold ? FW_BOLD : FW_NORMAL;
  f.lfItalic  = italic;
  f.lfUnderline = underline;
  f.lfStrikeOut = strikeout;

  f.lfOutPrecision = OUT_TT_PRECIS;
  f.lfQuality = ANTIALIASED_QUALITY;
  f.lfPitchAndFamily = VARIABLE_PITCH;

  f.lfEscapement = (LONG)(angle*10.0);

  if(name)
	memcpy(f.lfFaceName, name, strlen(name) < 32 ? strlen(name) + 1 : 32);
  else
	  f.lfFaceName[0] = 0;
 
  return CreateFontIndirect(&f);
}
#else
HFONT MakeFont(HDC hDC, char* name, int size, bool bold, bool italic, bool underline, bool strikeout, float angle){
	return 0;
}
#endif

//=========
// создание OpenGL-фонта			   
void MglCreateGLFont(char *fontName, int fontSize, int listBase){
  HDC hDC;
  HFONT old, hf;

  hDC = wglGetCurrentDC();

  hf = MakeFont(hDC, fontName, fontSize, false, false, false, false, 0);

  old = (HFONT)SelectObject(hDC, hf);
#ifdef WIN32
  bool b = wglUseFontBitmaps(hDC, 0, 256, listBase);
#endif

/*
  SIZE sz;
  GetTextExtentPoint(hDC, K_STR, length(K_STR), sz);
  text_width := sz.cx; text_height := sz.cy;
*/

  SelectObject(hDC, old);
  DeleteObject(hf);
}

void DrawGLText(char *str, int x,int y, int listBase){
	glRasterPos2d(x,y);
	glListBase(listBase);
	glCallLists((GLsizei)strlen(str),GL_UNSIGNED_BYTE,str);
}

void DrawGLTextSz(char *str, int sz, int x,int y, int listBase){
	glRasterPos2d(x,y);
	glListBase(listBase);
	glCallLists(sz,GL_UNSIGNED_BYTE,str);
}



void OpenGLDrawCircle(GLint x, GLint y, GLint radius, MRGB rgb, int a=1, int lines=0){
	float angle;
	if(!lines) lines=radius+10;
	glColor3f(rgb.red/float(255), rgb.green/float(255), rgb.blue/float(255));

	glBegin(!a ? GL_LINE_LOOP : GL_POLYGON);
	for(int i = 0; i < lines; i++){
		angle = i * 2 * PI / lines;
		glVertex2f(x + (cos(angle) * radius), y + (sin(angle) * radius));
	}

	glEnd();
	return ;
}


void OpenGLDrawCircle3d(GLint x, GLint y, GLint z, GLint radius, MRGB rgb, int a=1, int lines=0){
	float angle;
	if(!lines) lines=radius+10;
	glColor3f(rgb.red/float(255), rgb.green/float(255), rgb.blue/float(255));

	glBegin(!a ? GL_LINE_LOOP : GL_POLYGON);
	for(int i = 0; i < lines; i++){
		angle = i * 2 * PI / lines;
		glVertex3f(x + (cos(angle) * radius), y + (sin(angle) * radius), z);
	}

	glEnd();
	return ;
}


void glColor3m(MRGB rgb){
	glColor3ub(rgb.red, rgb.green, rgb.blue);
	//glColor3f(rgb.red/float(255), rgb.green/float(255), rgb.blue/float(255));
}


// OpenGl Rotate
void MglRotate(float g, float f1, float f2, float &t1, float &t2){
	float angle = g * PI / 180;
	float i = -(f2 * sin(angle) - f1 * cos(angle));
	float j = (f2 * cos(angle) + f1 * sin(angle));
	t1 = i;
	t2 = j;
}

void MglRotateSphere(float rx, float ry, float sx, float sy, float sz, float &nx, float &ny, float &nz){
	float ax = rx * PI / 180, ay = ry * PI / 180;

	//nx = 0; sin(ay) * cos(ax);
	//ny = sy * sin(ax);
	//nz = sz * cos(ay);

	nx = sx * 2 * sin(ay) * cos(ax);
	ny = - sy * sin(ax);
	nz = sz * cos(ax) * cos(ay);

	//nx = sx * cos(ax) * cos(ay);
	//ny = sy * cos(ax) * sin(ay);
	//nz = sz * sin(ax);
}

float box_sin(float angle){
	if(angle < 0)
		angle = -1 * ( -360 - angle);
	angle -= (int)angle / 360 * 360;

	if(angle <= 45)
		return angle / 45;
	if(angle <= 90 + 45)
		return 1;
	if(angle <= 180 + 45)
		return 1 - (angle - 90 - 45) / 45;
	if(angle <= 180 + 90 + 45)
		return -1;
	if(angle <= 360)
		return -1 + (angle - 180 - 90 - 45) / 45;

	return 0;
}

float box_cos(float angle){
	if(angle < 0)
		angle = -1 * ( -360 - angle);
	angle -= (int)angle / 360 * 360;

	if(angle <= 45)
		return 1;
	if(angle <= 90 + 45)
		return 1 - (angle - 45) / 45;
	if(angle <= 180 + 45)
		return -1;
	if(angle <= 180 + 90 + 45)
		return -1 + (angle - 180 - 45) / 45;
	if(angle <= 360)
		return 1;

	return 0;
}

void MglRotateBox(float rx, float ry, float sx, float sy, float sz, float &nx, float &ny, float &nz){
	float ax = rx, ay = ry; // * PI / 180, ay = ry * PI / 180;

	if(box_sin(ax) < -1 || box_cos(ax) > 1)
		int z = 234;

	if(box_sin(ay) < -1 || box_cos(ay) > 1)
		int z = 234;

	nx = sx * 2 * box_sin(ay) * box_cos(ax);
	ny = - sy * box_sin(ax);
	nz = sz * box_cos(ax) * box_cos(ay);
}

//void MglRotateX(float g, float fy, float fz, float &ty, float &tz){
//	float angle = g * PI / 180;
//	float y = -(fz * sin(angle) - fy * cos(angle));
//	float z = (fz * cos(angle) + fy * sin(angle));
//	ty = y;
//	tz = z;
//}
//
//void MglRotateY(float g, float fx, float fz, float &tx, float &tz){
//	float angle = g * PI / 180;
//	float x = -(fz * sin(angle) - fx * cos(angle));
//	float z = (fz * cos(angle) + fx * sin(angle));
//	tx = x;
//	tz = z;
//}
//
//void MglRotateZ(float g, float fx, float fy, float &tx, float &ty){
//	float angle = g * PI / 180;
//	float x = -(fy * sin(angle) - fx * cos(angle));
//	float y = (fy * cos(angle) + fx * sin(angle));
//	tx = x;
//	ty = y;
//}

#define MGL_ROTATE_TEST(ID, AXIS, angle, x, y, rtx, rty) { \
	float nx, ny; \
	MglRotate(angle, x, y, nx, ny); \
	if(int(nx) != int(rtx) || int(ny) != int(rty)){ \
		print("MGL test Fail! For ", itos(ID), ". ", itos(nx)); \
		print(" != ", itos(rtx), " || ", itos(ny), " != ", itos(rty),  ".\r\n"); \
	} \
}

#define MGL_ROTATE(rx, ry, rz, x, y, z) \
	if(rx) MglRotate(rx, y, z, y, z); \
	if(ry) MglRotate(ry, x, z, x, z); \
	if(rz) MglRotate(rz, x, y, x, y);