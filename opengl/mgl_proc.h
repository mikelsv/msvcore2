VString GetFragVer(){
	return "\r\n#version 450\r\n";
}

VString GetFragHead(){
	return "\r\n\
precision highp float;	\r\n\
precision highp int;	\r\n\
precision mediump sampler3D;	\r\n\
						\r\n\
uniform ivec2 mouse;	\r\n\
						\r\n\
uniform int iFrame;		\r\n\
uniform float iTime;	\r\n\
uniform float iTimeDelta; \r\n\
uniform vec4 iMouse;	\r\n\
uniform vec3 iResolution;\r\n\
						\r\n\
void mainImage(inout highp vec4 fragColor, in highp vec2 fragCoord); \r\n\
						\r\n\
uniform sampler2D iChannel0;	\r\n\
uniform sampler2D iChannel1;	\r\n\
uniform sampler2D iChannel2;	\r\n\
uniform sampler2D iChannel3;	\r\n\
uniform sampler2D iChannel4;	\r\n\
uniform sampler2D iChannel5;	\r\n\
uniform sampler2D iChannel6;	\r\n\
uniform sampler2D iChannel7;	\r\n\
uniform sampler2D iChannel8;	\r\n\
uniform sampler2D iChannel9;	\r\n\
uniform vec2 iChannelResolution[10];	\r\n\
						\r\n\
out highp vec4 fragColor;		\r\n\
void main(){			\r\n\
	fragColor = vec4(0., 0., 0., 1.);	\r\n\
    mainImage(fragColor, gl_FragCoord.xy);\r\n\
}\r\n\
";
}


#define MG_PROC_FREE(v, f) if(v){ f(v); v = 0; }
#define MG_PROC_FREE2(v, f) if(v){ f(1, &v); v = 0; }

class MgProcEl;

class MgProcChildEl{
public:
	MgProcEl* child;
	int chan, start_frame, this_frame;
	double start_time, len_time;
};

class MgProcPel{
public:
	GLuint prog, frag, tex, fbo;
	MaticalsGlTexture link[MGLSL_CHANNELS_MAX];
	OList<MgProcChildEl> childs;

	MgProcPel(){
		prog = 0;
		frag = 0;
		tex = 0;
		fbo = 0;

		memset(link, 0, sizeof(link));
	}

	MgProcChildEl* GetNextChild(MgProcChildEl* cel, double time){
		UGLOCK(childs);

		while(cel = childs.Next(cel)){
			if(cel->start_time <= time && time < cel->start_time + cel->len_time)
				return cel;
		}

		return 0;
	}

	void AddChild(XDataEl *el, MgProcEl *child){
		UGLOCK(childs);

		MgProcChildEl *cel = childs.NewE();
		cel->child = child;
		cel->chan = 0;
		cel->this_frame = 9999999;
		cel->start_time = 0;
		cel->len_time = 9999;		

		el = el->a();

		while(el){
			if(el->k() == "channel")
				cel->chan = el->v().toi();
			else if(el->k() == "start_time")
				cel->start_time = el->v().tod();
			else if(el->k() == "len_time")
				cel->len_time = el->v().tod();
			else
				print("AddChild() key is unknown: '", el->k(), "'.\r\n");

			el = el->n();
		}

		if(cel->chan >= MGLSL_CHANNELS_MAX)
			cel->chan = MGLSL_CHANNELS_MAX - 1;

		return ;
	}

	void Clean(){
		if(prog){
			//glDetachShader(prog, vert);
			glDetachShader(prog, frag);
			MG_PROC_FREE(prog, glDeleteProgram);
			prog = 0;
		}

		MG_PROC_FREE(frag, glDeleteShader);

		MG_PROC_FREE2(fbo, glDeleteFramebuffers);
		MG_PROC_FREE2(tex, glDeleteTextures);

		childs.Clean();
	}

	~MgProcPel(){
		Clean();
	}

};



class MgProcEl{
public:
	MString file;
	MaticalsGlslImages images;
	MaticalsGlslVideos videos;
	MgProcPel pel[MGLSL_BUFFERS_ALL];
	GLuint vert;

	int opt_start_frame;
	int opt_printf;

	MgProcEl(){
		opt_printf = 0;

		vert = 0;
	}

	void CleanProc(){
		for(int i = 0; i < MGLSL_BUFFERS_ALL; i ++){
			pel[i].Clean();
		}

		MG_PROC_FREE(vert, glDeleteShader);
	}

	void Clean(){
		CleanProc();
	}

	~MgProcEl(){
		Clean();
	}

};

class MgProc : public MgGenChan{
	OList<MgProcEl> procs;
	MgProcEl *this_proc, *child_proc;

public:
	MgProc(){
		this_proc = 0;
		child_proc = 0;
	}

	MgProcEl* GetThisProc(){
		return this_proc;
	}

	MgProcEl* GetChildProc(){
		return child_proc;
	}

	bool Load(VString name, int use = 0){
		UGLOCK(procs);

		MgProcEl *el = Get(name);
		if(el)
			el->CleanProc();
		else{
			el = procs.NewE();
			el->file = name;
		}

		if(use)
			this_proc = el;

		if(RealLoad(el)){			
			return 1;
		}

		return 0;
	}

private:
	bool RealLoad(MgProcEl *mpel){
		XDataCont ct;
		MString data, common;
		GLuint vert;
		
		// Configure
		SString file;
		file.Add(mpel->file, "/configure.json");

		//
		if(!IsFile(file)){
			print("Load() '", file, "' FAIL.\r\n");
			return 0;
		}

		// Load
		data = LoadFile(file);

		// Json
		ct.ReadJson(data);
		XDataPEl el(ct);

		// Vert
		vert = CreateShaderFile(LString() + mpel->file + "/" + el("vertex.file")->v(), GL_VERTEX_SHADER);

		// Common
		if(el("common.file"))
			common = LoadFile(LString() + mpel->file + "/" + el("common.file")->v());


		// Options
		mpel->opt_start_frame = el("start_frame")->v().toi();			

		MgProcPel *proc = mpel->pel;
		mpel->vert = vert;

		// Compile
		if(!MaticalsOpenGl.flag_error)
		for(int i = 0; i < MGLSL_BUFFERS_ALL; i ++){
			if(i < MGLSL_BUFFERS_MAX){
				VString file = el(GenBuffName(i))->GetVal("file");
				if(file)
					proc[i].frag = CreateShader(LString() + GetFragVer() + GetFragHead() + common + "\r\n" + LoadFile(LString() + mpel->file + "/" + file), GL_FRAGMENT_SHADER, mpel->opt_printf);
			} else{
				if(!IsFile(LString() + mpel->file + "/" + el("fragment.file")->v())){
					print(LString() + "MaticalsGlsl.Load() error: fragment.file is no file: " + mpel->file + "/" + el("fragment.file")->v() + ".\r\n");
					//UseErrorProgram(VString(), "MaticalsGlsl.Load() error: fragment.file is no file");
				}
				proc[i].frag = CreateShader(LString() + GetFragVer() + GetFragHead() + common + "\r\n" + LoadFile(LString() + mpel->file + "/" + el("fragment.file")->v()), GL_FRAGMENT_SHADER, mpel->opt_printf);
			}

			if(proc[i].frag){
				proc[i].prog = glCreateProgram();
				//;

				// Attach
				glAttachShader(proc[i].prog, mpel->vert);
				glAttachShader(proc[i].prog, proc[i].frag);
				glLinkProgram(proc[i].prog);

				if(MaticalsOpenGl.flag_error)
					return 1;

				if(i < MGLSL_BUFFERS_MAX)
					CreateFrameBuffer(proc[i].tex, proc[i].fbo);

				//glUseProgram(proc[i].prog);

				// Channel data
				XDataEl *bel = el(i < MGLSL_BUFFERS_MAX ? GenBuffName(i) : "fragment");

				for(int j = 0; j < MGLSL_CHANNELS_MAX; j ++){
					XDataEl *pel = bel->Get(GenChanName(j));
					proc[i].link[j].Clean();

					if(!pel)
						continue;

					VString name = pel->a() ? pel->GetVal("file") : pel->v();

					// Set channel data
					for(int k = 0; k < MGLSL_BUFFERS_MAX; k ++){
						if(name == GenBuffName(k))
							proc[i].link[j].Set(proc[k].tex, MaticalsOpenGl.width, MaticalsOpenGl.height);
					}
#ifdef MGLSL_USE_LOAD_IMAGE
					if(IsImage(LString() + mpel->file + name))
						proc[i].link[j] = mpel->images.Load(LString() + mpel->file + name);
#endif
#ifdef MGLSL_USE_LOAD_VIDEO
					if(IsVideo(LString() + mpel->file + name))
						proc[i].link[j] = mpel->videos.Load(LString() + mpel->file + name, pel->a());
#endif

					if(proc[i].link[j] == -1){
						print(LString() + "MaticalsGlsl.Load() error: set texture data: " + bel->v() + "." + GenChanName(j) +  " = '" + name + "'." + "\r\n");
					}
				}

				// Childs
				XDataEl *cel = bel->Get("childs.:");
				while(cel){
					int res = Load(cel->k());

					if(res)
						proc[i].AddChild(cel, Get(cel->k()));
					else
						print("LoadChild(): '", cel->k(), "' FAIL!\r\n");

					cel = cel->n();
				}



				// OK.
			}

		}

		return 1;
	}

public:
	MgProcEl* Active(VString name){
		UGLOCK(procs);
		return this_proc = Get(name);
	}

	MgProcEl* ActiveChild(VString name){
		UGLOCK(procs);
		return child_proc = Get(name);
	}

	MgProcEl* Get(VString name){
		UGLOCK(procs);
		MgProcEl *el = 0;

		while(el = procs.Next(el)){
			if(el->file == name){
				return el;
			}
		}

		return 0;
	}

	~MgProc(){
	}

};
