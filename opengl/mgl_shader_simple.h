class MglSimpleGlsl{
protected:
	GLint prog_id;

public:
	MglSimpleGlsl(){
		prog_id = 0;
	}

	// Load
	int LoadFiles(VString vert_file, VString frag_file){
		return Load(LoadFile(vert_file), LoadFile(frag_file));
	}

	int Load(VString vert_code, VString frag_code){
		GLint vert_id, frag_id;

		Clean();

		if(!(vert_id = CreateShader(vert_code, GL_VERTEX_SHADER)))
			return 0;

		if(!(frag_id = CreateShader(frag_code, GL_FRAGMENT_SHADER))){
			glDeleteShader(vert_id);
			return 0;
		}

		if(!(prog_id = glCreateProgram())){
			glDeleteShader(vert_id);
			glDeleteShader(frag_id);
			return 0;
		}

		glAttachShader(prog_id, vert_id);
        glAttachShader(prog_id, frag_id);

		glLinkProgram(prog_id);
		checkCompileErrors(prog_id, 0);

		glDeleteShader(vert_id);
        glDeleteShader(frag_id);

		return 1;
	}

	// Get
	GLint GetAttribLocation(const char *name){
		return glGetAttribLocation(prog_id, name);
	}

	GLint GetUniformLocation(const char *name){
		return glGetUniformLocation(prog_id, name);
	}

	// Programm
	GLint GetProgramId(){
		return prog_id;
	}

	void UseProgram(){
		if(!prog_id)
			return ;
	     
        glUseProgram(prog_id);    
	}

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, int type){
        GLint success;
        GLchar infoLog[1024];

        if(type != 0){
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success){
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                print("ERROR::SHADER_COMPILATION_ERROR of type: ", type ? "???" : "PROGRAM", "\n", infoLog, "\n -- --------------------------------------------------- -- ");
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success){
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                print("ERROR::PROGRAM_LINKING_ERROR of type: ", type ? "???" : "PROGRAM", "\n", infoLog, "\n -- --------------------------------------------------- -- ");
            }
        }
    }

public:
	void checkOpenGLerror(){
	  GLenum errCode;
	  if((errCode = glGetError()) != GL_NO_ERROR)
		print("OpenGl error! - ", glErrorString(errCode));
	}

	void Clean(){
		if(prog_id)
			glDeleteProgram(prog_id);

		prog_id = 0;
	}

	~MglSimpleGlsl(){
		Clean();
	}
};


class MglSimpleGlslTriangle : public MglSimpleGlsl{
	GLuint vbo, vertex_id, color_id;

struct vertex{
 GLfloat x;
 GLfloat y;
};

public:
	int Init(){
		const char* vsSource = 
"#version 330 core\n"
"in vec2 coord;\n"
"void main() {\n"
"  gl_Position = vec4(coord, 0.0, 1.0);\n"
"}\n";
	const char* fsSource = 
"#version 330 core\n"
"uniform vec4 color;\n"
"void main() {\n"
"  gl_FragColor = color;\n"
"}\n";
	
	if(!Load(vsSource, fsSource))
			return 0;

		vertex_id = GetAttribLocation("coord");
		color_id = GetUniformLocation("color");	

		checkOpenGLerror();

		InitVbo();

		return 1;
	}

	int InitVbo(){
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		// ! Вершины нашего треугольника
		vertex triangle[] = {
			{-1.0f,-1.0f},
			{ 0.0f, 1.0f},
			{ 1.0f,-1.0f}
		};

		//! Передаем вершины в буфер
		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangle), triangle);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

		checkOpenGLerror();

		return 1;
	}

	void Render(){
		//! Отрисовка
		glClear(GL_COLOR_BUFFER_BIT);
		// ! Устанавливаем шейдерную программу текущей
		UseProgram();
		//glUseProgram(GetId()); 
  
		static float red[4] = {1.0f, 0.0f, 0.0f, 1.0f};
		// ! Передаем юниформ в шейдер
		glUniform4fv(color_id, 1, red);

		// ! Включаем массив атрибутов
		glEnableVertexAttribArray(vertex_id);
		// ! Подключаем VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		// ! Указывая pointer 0 при подключенном буфере, мы указываем, что данные представлены в VBO
		glVertexAttribPointer(vertex_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// ! Отключаем VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// ! Передаем данные на видеокарту (рисуем)
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vertex));

		// ! Отключаем массив атрибутов
		glDisableVertexAttribArray(vertex_id);

		// ! Отключаем шейдерную программу
		glUseProgram(0); 

		checkOpenGLerror();
	}
	
};

class MglGlslTexture : public MglSimpleGlsl{
	GLuint vbo, vao, vertex_id, color_id;

	// Render texture.
	// vec4 from, vec4 to;

struct vertex{
 GLfloat x;
 GLfloat y;
};

public:
	int Init(){
		const char* vsSource = 
"#version 330 core\n"
"in vec4 coord;\n"
"out vec2 TexCoords;\n"
"void main() {\n"
//"  gl_Position = vec4(coord, 0.0, 1.0);\n"
"  gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
"  TexCoords = coord.zw;\n"
//"  TexCoords = gl_Position.xy;\n"
"}\n";
	const char* fsSource = 
"#version 330 core\n"
"uniform vec3 textColor;\n"
"uniform sampler2D iChannel0;\n"
"in vec2 TexCoords;\n"
"out vec4 result;\n"
"void main() {\n"
//"vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);"
//"vec4 sampled = vec4(1.0, 1.0, 1.0, texture(iChannel0, TexCoords).r);\n"
//"result = vec4(textColor.xyz, 1.0) * sampled;\n"
//"  result = texture(iChannel0, TexCoords.xy);\n"
//"result = vec4(textColor.xyz, 1.0) * sampled;\n"
//"  gl_FragColor = color;\n"
"result = vec4(texture(iChannel0, TexCoords).rgb, 1.);\n"
//"result = vec4(1., 0., 1., 1.);\n"
"}\n";

		if(!Load(vsSource, fsSource))
			return 0;

		color_id = GetUniformLocation("textColor");	

		InitVbo();

		return 1;
	}

	int InitVbo(){
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		return 1;
	}

	void Render(GLint tex_id){
	   // activate corresponding render state	
		UseProgram();
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(vao);

		float xpos = -1, ypos = -1;
		float w = 2, h = 2;
		
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, tex_id);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}	
};


class MglGlslBuffer{
	GLuint ubo, bind, prog_id;

public:
	MglGlslBuffer(){
		ubo = 0;
	}

	void Init(GLuint prog, GLuint b){
		glGenBuffers(1, &ubo);
		
		prog_id = prog;
		bind = b;
	}

	void SetData(void *data, int size){
		glUseProgram(prog_id);

		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, bind, ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glUseProgram(0);
	}

	void SetBufferData(void *data, int size){
		glUseProgram(prog_id);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ubo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind, ubo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glUseProgram(0);
	}


	~MglGlslBuffer(){
		if(ubo){
			glDeleteBuffers(1, &ubo);
			ubo = 0;
		}
	}
	
};