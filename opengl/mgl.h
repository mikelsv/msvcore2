#pragma once

// Libraries
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"glu32.lib")

// Ffmpeg
#ifdef USEMSV_FFMPEG_LIBRARY

// https://habr.com/ru/post/449198/

#ifndef H264_DECODER_H
#define H264_DECODER_H
#endif

extern "C"
{
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavformat/avio.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavutil/motion_vector.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")

#endif

// Maticals OpenGL Core //

// Ki
#include "ki_base.h"

bool MaticalsOpenGlRenderOpen(VString file, KiInt3 window, int fps);

class MaticalsOpenGl{
public:
	// Window size
	KiInt3 window;
	int width, height;
	bool opt_update_title;

	// Project
	MString project_file;

	// Time
	bool opt_time_go;
	float time_now, time_delta;

	// Frames
	int frame_id;
	
	// Mouse
	KiInt2 mouse;
	bool mouse_down;

	// Render
	MString opt_render_file, opt_render_time;
	KiInt2 opt_time;
	bool opt_render_enable, opt_disable_resize;	
	int opt_fps;

	// Error
	int flag_error;

	// Init
	MaticalsOpenGl(){
		opt_update_title = 1;
		opt_time_go = 0;
		frame_id = 0;
		mouse_down = 0;
		opt_render_enable = 0;
		opt_disable_resize = 0;
		opt_fps = 30;
		flag_error = 0;
	}

	// Window
	void SetWindowSize(KiInt3 w){
		window = w;
	}

	void UpdateSize(int w, int h){
		width = w;
		height = h;
	}

	bool IsUpdateTitle(){
		return opt_update_title;
	}

	char* GetWindowTitle(){
		SString ss;
		///static char title[1024];

		if(opt_update_title){
			opt_update_title = 0;
			ss.Format("Maticals Render %s %ds %df", !opt_time_go ? "[||]" : ">>>", time_now, frame_id);
		}

		return ss;
	}

	// Project
	void SetProject(VString file){
		project_file = file;
	}

	// Time
	void OnStartStop(){
		opt_update_title = 1;
		opt_time_go = !opt_time_go;
	}

	void UpdateTime(float d){
		if(opt_time_go && !opt_render_enable){
			time_now += d;
			frame_id ++;
			opt_update_title = 1;
			time_delta = d;
		}
	}

	void OnTimeHome(){
		time_now = 0;
		opt_update_title = 1;
	}

	void OnTimeLeft(){
		if(time_now > 0)
			time_now = max(time_now - 1, 0);

		opt_update_title = 1;
	}

	void OnTimeRight(){
		time_now ++;

		opt_update_title = 1;
	}

	// Mouse
	KiInt2 GetMouse(){
		return mouse;
	}

	void SetMouse(KiInt2 m){
		mouse = m;
	}
	
	// Render
	bool IsRender(){
		return opt_render_enable;
	}

	bool SetRender(bool v){
		opt_render_enable = v;
	}

	void SetFps(int f){
		opt_fps = f;

		if(!opt_fps)
			opt_fps = 30;
	}

	int GetFps(){
		return opt_fps;
	}

	void EnableRender(VString out, int fps, VString time){
		opt_render_file = out;
		opt_render_time = time;

		SetFps(fps);

		//opt_render_enable = 1;
		opt_time = time;

		opt_render_enable = MaticalsOpenGlRenderOpen(opt_render_file, window, fps);		
		if(!opt_render_enable)
			print("Render start failed!\r\n");
	}

	void SetDisableResize(){
		opt_disable_resize = 1;
	}

	bool IsDisableResize(){
		if(!opt_disable_resize && opt_render_enable){
			opt_disable_resize = 1;
			return 0;
		}

		return opt_disable_resize;
	}

} MaticalsOpenGl;

class MaticalsGlsl{
public:

	void UseErrorProgram(VString source, VString log){}

} MaticalsGlsl;



// User callback
#ifndef MATICALS_GLSL_CALLBAK
#define MATICALS_GLSL_CALLBAK
class MaticalsGlslCallback{
} MaticalsGlslCallback;
#endif

#define GL_BGR_EXT                        0x80E0

void glColor3m(MRGB rgb){
	glColor3ub(rgb.red, rgb.green, rgb.blue);
	//glColor3f(rgb.red/float(255), rgb.green/float(255), rgb.blue/float(255));
}

#include "mgl_error.h"
#include "mgl_common.cpp"
#include "mgl_shaders.h"
#include "mgl_load.h"
#include "mgl_proc.h"
#include "mgl_shader_simple.h"
#include "mgl_shader_gui.h"

#ifdef USEMSV_FFMPEG_LIBRARY
#include "mgl_render.h"
#include "mgl_avideo.h"


class MaticalsOpenGlRender{
	// Grab
	MaticalsVideoWrite grab;	

public:
	bool Open(VString file, KiInt3 window, int fps){
		return grab.Open(file, window.x, window.y, fps);
	}

	bool Write(int frame, int frames){
		glReadPixels(0, 0, MaticalsOpenGl.width, MaticalsOpenGl.height, GL_RGB, GL_UNSIGNED_BYTE, grab.ipe.GetData());
		return grab.WriteFrame(grab.ipe.GetData(), grab.ipe.GetAllSize(), frame, frames);
	}

	bool WriteAudio(MaticalsAudioLink &al){
		unsigned char *d = al.data, *t = al.data.endu();

		for(int i = 0; i < 200; i ++)
			grab.WriteTestAudioFrame();

		return 1;

		while(d < t){
			int len = min(t - d, 1024); 
			grab.WriteAudioFrame(d, len, 0);

			d += len;
		}
		
		return 1;
	}

	void Close(){
		return grab.Close();
	}

} MaticalsOpenGlRender;

bool MaticalsOpenGlRenderOpen(VString file, KiInt3 window, int fps){
	return MaticalsOpenGlRender.Open(file, window, fps);
}

#endif