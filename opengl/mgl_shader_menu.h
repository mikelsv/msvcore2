/* Menu shader
Input:
	iTime - time
	iMouse - mouse position
	iResolution - screen resolution
*/

class MglGlslVec3{
public:
	float z, y, x;

	MglGlslVec3(){
		x = y = z;
	}

	MglGlslVec3(const KiVec3 &v){
		x = v.x;
		y = v.y;
		z = v.z;
	}

};


#pragma pack(push, 1) 
struct MglMenuOptions{
public:
    // Size
	float fontSize, letterWidth, maxWidth;    
    float borderSize, freeSize, unk[3];

    // Colors
    KiVec4 textColor, bgColor, borderColor, activeColor;
};
#pragma pack(pop)

#pragma pack(push, 1) 
struct MglMenuHead{
	int id, pos, size;
};
#pragma pack(pop)

class MglMenulEl{

public:
	int id;
	MString text;
};

class MglMenu : public MglSimpleGlsl{
		OList<MglMenulEl> menu;
		MglMenuOptions opt;
		MString head_data, text_data;

		bool active;

		// Uniform
		GLint uni_iTime, uni_iRes, uni_iMouse, uni_iMouseMenu, unu_iFont, unu_iOptions;
		GLuint font_id;
		GLuint vbo, vao;

		// Buffer
		MglGlslBuffer buf_opt, buf_head, buf_text;
public:

	MglMenu(){}

	bool Init(){
		// Init Options
		opt.fontSize = 40;
		opt.letterWidth = 20.;
		opt.borderSize = 2;

		opt.textColor = KiVec4(.06, .94, .94, 1);		
		opt.bgColor = KiVec4(.94, .94, .94, 1);
		opt.borderColor = KiVec4(.06, .06, .06, 1);
		opt.activeColor = KiVec4(1.0, 0.83, 0.22, 1);

		// Test
		//opt.textColor = KiVec4(1., 1., 1., );
		//opt.bgColor = KiVec4(0., 0., 1.);
		//opt.borderColor = KiVec4(0., 0., 0.);
		//opt.activeColor = KiVec4(1.0, 0., 0.);
		

		// Test
		//opt.maxWidth = 300;
		//opt.borderSize = 5;
		//opt.freeSize = 10;

		// Init Glsl
		if(!Load(LoadFile("glsl/menu.vs"), LoadFile("glsl/menu.fs")))
			return 0;

		InitVbo();

		// Init uniforms
		uni_iTime = GetUniformLocation("iTime");
		uni_iRes = GetUniformLocation("iResolution");
		uni_iMouse = GetUniformLocation("iMouse");
		uni_iMouseMenu = GetUniformLocation("iMouseMenu");
		unu_iFont = GetUniformLocation("iFont");
		//unu_iOptions = GetUniformLocation("MenuOptions");

		buf_opt.Init(prog_id, 2);
		buf_head.Init(prog_id, 3);
		buf_text.Init(prog_id, 4);
		
		//buf_opt.SetData(&opt, sizeof(opt));

		UpdateMouse(KiInt2());

		return 1;
	}

	// Is
	bool IsActive(){
		return active;
	}

	// Insert
	bool InsertItem(int id, VString text){
		MglMenulEl *el = menu.NewE();
		el->id = id;
		el->text = text;

		return 1;
	}

	// Update
	void SetFont(GLuint f){
		font_id = f;
	}

	void UpdateRes(KiVec2 res){
		UseProgram();
		glUniform2f(uni_iRes, res.x, res.y);
		glUseProgram(0);
	}

	void UpdateMouse(KiVec2 res){
		UseProgram();
		glUniform2f(uni_iMouse, res.x, res.y);
		glUseProgram(0);
	}

	void UpdateMouseMenu(KiVec2 res){
		UseProgram();
		glUniform2f(uni_iMouseMenu, res.x, res.y);
		glUseProgram(0);
	}
		
	bool DrawMenu(){
		MglMenulEl *el = 0;
		int max_width = 0, count = 0, text_count = 0;
		int text_pos = 0;

		// Count
		while(el = menu.Next(el)){
			max_width = max(max_width, el->text.size());
			text_count += el->text.size();
			count ++;
		}

		MString data, data_head;
		data.Reserve(text_count * sizeof(int));
		int *idata = (int*)data.data;

		data_head.Reserve(sizeof(MglMenuHead) * count);
		MglMenuHead *head = (MglMenuHead*) data_head.data;
		count = 0;

		while(el = menu.Next(el)){
			//memcpy(data.data + text_pos, el->text, el->text.size());
			for(int i = 0; i < el->text.size(); i ++){
				*idata ++ = el->text[i];
			}

			head->id = el->id;
			head->pos = text_pos;
			head->size = el->text.size();
			head ++;
			
			text_pos += el->text.size();
			count ++;
		}

		// Set options
		opt.maxWidth = max_width * opt.letterWidth;
		buf_opt.SetData(&opt, sizeof(opt));

		buf_head.SetBufferData(data_head, data_head.size());
		buf_text.SetBufferData(data, data.size());

		if(data_head.size())
			active = 1;
		else
			active = 0;

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

	void Render(float iTime){
		//GLint tex_id = 0;

	   // activate corresponding render state	
		UseProgram();
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(vao);

		glUniform1fv(uni_iTime, 1, &iTime);

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
		glBindTexture(GL_TEXTURE_2D, font_id);
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

	void CleanMenu(){
		menu.Clean();
	}

	~MglMenu(){
		//Clear();
	}

};