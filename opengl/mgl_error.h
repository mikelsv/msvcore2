class MaticalsGlslErrorImage{
	MWndIPE ipe;
	LString data;
	int width, height, lines;
	int ferr;

public:
	int Error(VString code, VString error, int w, int h){
		data.Clean();

		int ly = 0;
		VString err;
		int ex, ey;

		err = ReadError(error, ex, ey);
		ferr = -1;

		width = w;
		height = 0;	
		lines = 0;

		VString line = code, l;
		while(1){
			l = PartLine(line, line, "\n");
			if(!l.data){
				break;
			}

			AddLine(l);

			if(ey == lines + 1 && err){
				AddLine(err, 1);
				err = ReadError(error, ex, ey);
				
				if(ferr < 0)
					ferr = height;
			}

			lines ++;
		}

		DoImage();

		return 1;
	}

	void AddLine(VString line, int err = 0){
		unsigned char c[3];
		int x = 0;

		// Number
		char b[10];
		int ls = lines + 1, lz = 0;

		while(ls > 0){
			b[lz ++] = ls % 10;
			ls /= 10;
		}

		if(!err){
			for(int i = 0; i < lz; i ++){
				c[0] = 48 + b[lz - i - 1];
				c[1] = err;
				c[2] = 0;

				data.Add(c, 3);
				x ++;
			}

			c[0] = ':';
			data.Add(c, 3);
			x ++;

			c[0] = ' ';
			data.Add(c, 3);
			x ++;
		}

		// Line
		for(unsigned int i = 0; i < line.size(); i ++){
			c[0] = line[i];
			c[1] = err;
			c[2] = err;

			if(c[0] == '\r' || c[0] == '\n')
				c[0] = ' ';

			if(c[0] == '\t'){				
				int s = 4 - ((x + 1) % 4);
				c[0] = ' ';

				while(s){
					data.Add(c, 3);
					x ++;
					s --;
				}
			}

			data.Add(c, 3);
			x ++;
		}

		if(x >= width){
			height ++;
			x -= width;
		}

		for(x; x < width; x ++){
			c[0] = ' ';
			c[1] = 0;
			c[2] = 0;

			data.Add(c, 3);
		}

		height ++;
		return ;
	}

	VString ReadError(VString &err, int &x, int &y){
		VString line = PartLine(err, err, "\n"), l;
		VString lx = PartLine(line, l, "(");
		VString ly = PartLine(l, l, ")");

		x = lx.toi();
		y = ly.toi();

		return line;
	}

	void DoImage(){
		ipe.New(width, height);
		if(!ipe.Is())
			return ;

		unsigned char *line = data.OneLine();
		//if(data.size() != ipe.GetAllSize())
		//	return ;

		// Set first error
		if(data.size() > 3)
			line[0 + 1] = ferr >> 8;
			line[0 + 2] = ferr;

		unsigned char *img = ipe.GetData(), *to = img + ipe.GetAllSize();
		for(int y = 0; y < height; y ++){
			memcpy(img + ipe.GetEffWidth() * y, line + y * width * 3, width * 3);
		}

		//memcpy(img, line, ipe.GetAllSize());

		return ;
	}

	MWndIPE* GetIPE(){
		return &ipe;
	}

	int GetWidth(){
		return ipe.GetWidth();
	}

	int GetHeight(){
		return ipe.GetHeight();
	}

	unsigned char* GetImage(){
		return ipe.GetData();
	}

};


char const* glErrorString(GLenum const err){
  switch (err)
  {
    // opengl 2 errors (8)
    case GL_NO_ERROR:
      return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";

    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";

    //case GL_TABLE_TOO_LARGE:
    //  return "GL_TABLE_TOO_LARGE";

    // opengl 3 errors (1)
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    // gles 2, 3 and gl 4 error are handled by the switch above
    default:
    //  assert(!"unknown error");
      return nullptr;
  }
}


void APIENTRY MsvGlDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, void*){
	unsigned char log[2048];
	int count = 0, len;

#define MsvGlDebugCallbackAdd(text){len = strlen(text); if(count + len < (sizeof(log) - 1)){ memcpy(log + count, text, len); count += len; log[count] = 0; } }
#define MsvGlDebugCallbackAdd2(text, len){ if(count + len < (sizeof(log) - 1)){ memcpy(log + count, text, len); count += len; log[count] = 0; } }

	//auto output = std::string("GLDEBUG: OpenGL ");
	//LString output;
	MsvGlDebugCallbackAdd("\r\nGLDEBUG: OpenGL ");

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:					MsvGlDebugCallbackAdd("error");							break;
	case GL_DEBUG_TYPE_PORTABILITY:				MsvGlDebugCallbackAdd("portability issue");				break;
	case GL_DEBUG_TYPE_PERFORMANCE:				MsvGlDebugCallbackAdd("performance issue");				break;
	case GL_DEBUG_TYPE_OTHER:					MsvGlDebugCallbackAdd("issue");							break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:		MsvGlDebugCallbackAdd("undefined behavior");			break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:		MsvGlDebugCallbackAdd("deprecated behavior");			break;
	default:									MsvGlDebugCallbackAdd("unknown issue");					break;
	}

	switch (source) {
	case GL_DEBUG_SOURCE_API:																break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:			MsvGlDebugCallbackAdd(" in the window system");			break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:		MsvGlDebugCallbackAdd(" in the shader compiler");		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:			MsvGlDebugCallbackAdd(" in third party code");			break;
	case GL_DEBUG_SOURCE_APPLICATION:			MsvGlDebugCallbackAdd(" in this program");				break;
	case GL_DEBUG_SOURCE_OTHER:					MsvGlDebugCallbackAdd(" in an undefined source");		break;
	default:									MsvGlDebugCallbackAdd(" nowhere(?)");					break;
	}

	MsvGlDebugCallbackAdd(", id ");
	MsvGlDebugCallbackAdd(itos(id));
	MsvGlDebugCallbackAdd(":\n");

	//output + ", id " + id + ":\n";
	
	MsvGlDebugCallbackAdd2(message, length);
	//output + VString(message, length);

	//printf("%s\n", output.c_str());
	print(log);

	return ;
}

void MsvGlDebug(int enable, int ext = 0){
	if(!enable){
		glDisable(GL_DEBUG_OUTPUT);
		glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		return ;
	}

	// enable the debug output
	glEnable(GL_DEBUG_OUTPUT);
	// debug output from main thread: step-by-step debug from the correct stack frame when breaking
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glDebugMessageCallback((GLDEBUGPROC)MsvGlDebugCallback, 0);

	// query everything about errors, deprecated and undefined things
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, 0, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, 0, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DONT_CARE, 0, 0, GL_TRUE);

	// disable misc info. might want to check these from time to time!
	//glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, 0, GL_FALSE);
	//glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, 0, GL_FALSE);

	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, 0, !ext ? GL_FALSE : GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, 0, !ext ? GL_FALSE : GL_TRUE);

	return ;
}