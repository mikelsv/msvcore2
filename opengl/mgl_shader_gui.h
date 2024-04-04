/* Menu shader
Input:
	iTime - time
	iMouse - mouse position
	iResolution - screen resolution
*/

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
		KiVec2 menu_pos; 

		bool active;

		// Uniform
		GLint uni_iTime, uni_iRes, uni_iMouse, uni_iMouseMenu, uni_iFont;
		GLint uni_iOpt;
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
		uni_iFont = GetUniformLocation("iFont");
		//unu_iOptions = GetUniformLocation("MenuOptions");
		//uni_iOpt = glGetUniformBlockIndex(prog_id, "MenuOptionsId");
		//GLuint buf_head_id = glGetProgramResourceIndex(prog_id, GL_SHADER_STORAGE_BLOCK, "MenuHead");
		//GLuint buf_text_id = glGetProgramResourceIndex(prog_id, GL_SHADER_STORAGE_BLOCK, "MenuTextBuffer");

		buf_opt.Init(prog_id, 2);
		buf_head.Init(prog_id, 3);
		buf_text.Init(prog_id, 4);

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

	void UpdateMouse(KiVec2 pos){
		UseProgram();
		glUniform2f(uni_iMouse, pos.x, pos.y);
		glUseProgram(0);
	}

	void UpdateMouseMenu(KiVec2 pos){
		menu_pos = pos;

		UseProgram();
		glUniform2f(uni_iMouseMenu, pos.x, pos.y);
		glUseProgram(0);
	}

	// In
	bool InRect(KiVec4 rect, KiVec2 coord){
		if(coord.x > rect.x && coord.x < rect.z && coord.y > rect.y && coord.y < rect.w)
			return true;
		return false;
	}

	int OnClick(KiVec2 mouse){
		MglMenulEl *el = 0;
		float menu_height = menu.Size() * opt.fontSize * 0;
		int count = 0;

		while(el = menu.Next(el)){
			KiVec4 rect(menu_pos.x, menu_pos.y - menu_height - opt.fontSize * (count + 1), menu_pos.x + opt.maxWidth, menu_pos.y - menu_height - opt.fontSize * count);

			if(InRect(rect, mouse)){
				//el->etime = this_time + opt.hideTime;
				return el->id;
			}

			count ++;
		}

		return 0;
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
		buf_opt.SetUniformData(&opt, sizeof(opt));

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

// Pop up
class MglPopUpEl{
public:
	MString text;
	float stime, etime;
	KiVec4 rect;
};

#pragma pack(push, 1) 
struct MglPopUpOptions{
public:
    // Size
	float fontSize, letterWidth;    
    float borderSize, freeSize;
	float spaceHeight;
	float showTime, hideTime;
	float unk[1];

    // Colors
    KiVec4 textColor, bgColor, borderColor;
};
#pragma pack(pop)

#pragma pack(push, 1) 
struct MglPopUpHead{
	int id, text_pos, text_size, _unk;
	float stime, etime, _unk2[2];
	KiVec4 rect;
};
#pragma pack(pop)

class MglPopUp : public MglSimpleGlsl{
	// Data
	OList<MglPopUpEl> els;
	MglPopUpOptions opt;
	float this_time;
	KiVec2 resolution;
	bool active, update;

	// Uniform
	GLint uni_iTime, uni_iRes, uni_iMouse, uni_iFont, unu_iOptions, uni_popupSize;
	GLuint font_id;
	GLuint vbo, vao;

	// Buffer
	MglGlslBuffer buf_opt, buf_head, buf_text;
public:

	MglPopUp(){
		active = 0;
	}		

	bool Init(){
		// Init Options
		opt.fontSize = 40;
		opt.letterWidth = 20.;
		opt.borderSize = 2;
		opt.freeSize = 5;
		opt.spaceHeight = 10;
		opt.showTime = 1;
		opt.hideTime = 1;

		opt.textColor = KiVec4(1.0, 0.83, 0.22, 1);		
		opt.bgColor = KiVec4(.94, .94, .94, 1);
		opt.borderColor = KiVec4(.06, .06, .06, 1);

		// Init Glsl
		if(!Load(LoadFile("glsl/popup.vs"), LoadFile("glsl/popup.fs")))
			return 0;

		InitVbo();

		// Init uniforms
		uni_iTime = GetUniformLocation("iTime");
		uni_iRes = GetUniformLocation("iResolution");
		uni_iMouse = GetUniformLocation("iMouse");
		//uni_iMouseMenu = GetUniformLocation("iMouseMenu");
		uni_iFont = GetUniformLocation("iFont");
		uni_popupSize = GetUniformLocation("popupSize");
		//unu_iOptions = GetUniformLocation("MenuOptions");

		buf_opt.Init(prog_id, 5);
		buf_head.Init(prog_id, 6);
		buf_text.Init(prog_id, 7);
		
		//buf_opt.SetData(&opt, sizeof(opt));

		//UpdateMouse(KiInt2());

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

	void Insert(VString text, int life_time = 30){
		MglPopUpEl *el = els.NewE();
		el->text = text;
		el->stime = this_time;
		el->etime = this_time + life_time;

		update = 1;
		//Draw();
		return ;
	}

	// Update
	void SetFont(GLuint f){
		font_id = f;
	}

	void UpdateRes(KiVec2 res){
		resolution = res;

		UseProgram();
		glUniform2f(uni_iRes, res.x, res.y);
		glUseProgram(0);
	}

	void UpdateHeadSize(int size){
		UseProgram();
		glUniform1i(uni_popupSize, size);
		glUseProgram(0);
	}

	void UpdateTime(float time){
		this_time = time;
	}

	bool InRect(KiVec4 rect, KiVec2 coord){
		if(coord.x > rect.x && coord.x < rect.z && coord.y > rect.y && coord.y < rect.w)
			return true;
		return false;
	}

	void OnClick(KiVec2 mouse){
		MglPopUpEl *el = 0;
		//int update = 0;

		while(el = els.Next(el)){			
			if(InRect(el->rect, mouse) && el->etime > this_time + opt.hideTime){
				el->etime = this_time + opt.hideTime;
				update = 1;
			}
		}
	}

	void Draw(){
		update = 0;
		ClearExpire();

		// Count
		MglPopUpEl *el = 0;
		int count = 0, text_count = 0;

		while(el = els.Next(el)){			
			text_count += el->text.size();
			count ++;
		}

		// Memory
		MString head_data, text_data;
		int text_pos = 0;

		// Text
		text_data.Reserve(text_count * sizeof(int));
		int *idata = (int*)text_data.data;

		//Data
		head_data.Reserve(sizeof(MglPopUpHead) * count);
		MglPopUpHead *head = (MglPopUpHead*) head_data.data;

		// Rect
		KiVec2 pos = KiVec2(resolution.x - 10, 40);
		float border = opt.borderSize * 2 + opt.freeSize * 2;
		float cell_height = opt.fontSize + border;		

		// Write
		while(el = els.Next(el)){
			//memcpy(data.data + text_pos, el->text, el->text.size());
			for(int i = 0; i < el->text.size(); i ++){
				*idata ++ = el->text[i];
			}

			head->id = 0;
			head->rect = el->rect = KiVec4(pos.x - el->text.size() * opt.letterWidth - border, pos.y, pos.x, pos.y + cell_height);
			head->stime = el->stime;
			head->etime = el->etime;
			head->text_pos = text_pos;
			head->text_size = el->text.size();
			head ++;

			pos.y += cell_height + opt.spaceHeight;
			text_pos += el->text.size();
		}

		// Set options
		buf_opt.SetUniformData(&opt, sizeof(opt));
		buf_head.SetBufferData(head_data, head_data);
		buf_text.SetBufferData(text_data, text_data);

		UpdateHeadSize(count);
	}

	void ClearExpire(){
		MglPopUpEl *el = 0, *pel = 0;		

		while(el = els.Next(el)){
			if(el->etime < this_time){
				els.Free(el);
				el = pel;
			} else
				pel = el;			
		}
	}

	void Render(float iTime){
		this_time = iTime;

		if(update)
			Draw();

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

	~MglPopUp(){
		//Clear();
	}
};

enum MglGuiTypesEnum{
	MGLGUI_TYPES_UNKNOWN,
	MGLGUI_TYPES_MENU,
	MGLGUI_TYPES_TREE,
	MGLGUI_TYPES_TOOLBAR,

	MGLGUI_PLUSMINUS,
	MGLGUI_CHECKBOX,
	MGLGUI_COLOR,
	MGLGUI_TEXT
};

class MglGuiElements{
public:
	MglGuiTypesEnum type;
	MString text;
	int id;
	KiVec4 rect;
};

#pragma pack(push, 1)
class MglGuiOptions{
public:
	float fontSize, letterWidth;
	float borderSize, freeSize;
	//float unk[3];

	KiVec4 textColor, bgColor, borderColor;

	MglGuiOptions(){
		fontSize = 40;
		letterWidth = fontSize / 2;
	}
};
#pragma pack(pop)

#pragma pack(push, 1) 
struct MglGuUpHead{
	int id, text_pos, text_size, _unk;
	KiVec4 rect;
};
#pragma pack(pop)

class MglGui : public MglSimpleGlsl{
	// Data
	OList<MglGuiElements> els;
	MglGuiOptions opt;
	KiVec2 screen;
	bool update;

	// Uniform
	GLint uni_iTime, uni_iRes, uni_iMouse, uni_iFont, unu_iOptions, uni_guiSize;
	GLuint font_id;
	GLuint vbo, vao;

	// Buffer
	MglGlslBuffer buf_opt, buf_head, buf_text;
public:

	bool Init(){
		// Init Options
		opt.fontSize = 40;
		opt.letterWidth = opt.fontSize / 2;
		opt.borderSize = 2;
		opt.freeSize = 5;

		opt.textColor = KiVec4(.06, .94, .94, 1);		
		opt.bgColor = KiVec4(.94, .94, .94, 1);
		opt.borderColor = KiVec4(.06, .06, .06, 1);

		// Init Glsl
		if(!Load(LoadFile("glsl/gui.vs"), LoadFile("glsl/gui.fs")))
			return 0;

		InitVbo();

		// Init uniforms
		uni_iTime = GetUniformLocation("iTime");
		uni_iRes = GetUniformLocation("iResolution");
		uni_iMouse = GetUniformLocation("iMouse");
		uni_iFont = GetUniformLocation("iFont");
		uni_guiSize = GetUniformLocation("guiSize");

		buf_opt.Init(prog_id, 8);
		buf_head.Init(prog_id, 9);
		buf_text.Init(prog_id, 10);

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

	void InsertMenu(int id, VString text){
		MglGuiElements *el = els.NewE();
		el->type = MGLGUI_TYPES_MENU;
		el->id = id;
		el->text = text;

		update = 1;
	}

	void InsertToolbar(int id, VString text){
		
	}

	// Update
	void SetFont(GLuint f){
		font_id = f;
	}

	void UpdateRes(KiVec2 res){
		screen = res;

		UseProgram();
		glUniform2f(uni_iRes, res.x, res.y);
		glUseProgram(0);
	}

	void UpdateHeadSize(int size){
		UseProgram();
		glUniform1i(uni_guiSize, size);
		glUseProgram(0);
	}	

	// In
	bool InRect(KiVec4 rect, KiVec2 coord){
		if(coord.x > rect.x && coord.x < rect.z && coord.y > rect.y && coord.y < rect.w)
			return true;
		return false;
	}

	int OnClick(KiVec2 mouse){
		MglGuiElements *el = 0;
		//int update = 0;

		while(el = els.Next(el)){			
			if(InRect(el->rect, mouse)){
				//el->etime = this_time + opt.hideTime;
				return el->id;
			}
		}

		return 0;
	}

	void Draw(){
		update = 0;

		// Update elements
		DrawMenu();

		// Count
		MglGuiElements *el = 0;
		int count = 0, text_count = 0;

		while(el = els.Next(el)){			
			text_count += el->text.size();
			count ++;
		}

				// Memory
		MString head_data, text_data;
		int text_pos = 0;

		// Text
		text_data.Reserve(text_count * sizeof(int));
		int *idata = (int*)text_data.data;

		//Data
		head_data.Reserve(sizeof(MglGuUpHead) * count);
		MglGuUpHead *head = (MglGuUpHead*) head_data.data;

				// Write
		while(el = els.Next(el)){
			//memcpy(data.data + text_pos, el->text, el->text.size());
			for(int i = 0; i < el->text.size(); i ++){
				*idata ++ = el->text[i];
			}

			head->id = 0;
			head->rect = el->rect;// = KiVec4(pos.x - el->text.size() * opt.letterWidth - border, pos.y, pos.x, pos.y + cell_height);
			head->text_pos = text_pos;
			head->text_size = el->text.size();
			head ++;

			//pos.y += cell_height + opt.spaceHeight;
			text_pos += el->text.size();
		}

		// Set options
		buf_opt.SetUniformData(&opt, sizeof(opt));
		buf_head.SetBufferData(head_data, head_data);
		buf_text.SetBufferData(text_data, text_data);

		UpdateHeadSize(count);
	}

	void DrawMenu(){
		MglGuiElements *el = 0;
		KiVec2 pos = KiVec2(0, screen.y);
		float border = opt.borderSize * 2 + opt.freeSize * 2;

		// Update menu
		while(el = els.Next(el)){
			float size = el->text.size() * opt.letterWidth + border;
			el->rect = KiVec4(pos.x, pos.y -  opt.fontSize - border, pos.x + size, pos.y);
			pos.x += size;
		}
	}	

	void Render(float iTime){
		//this_time = iTime;

		if(update)
			Draw();

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

	~MglGui(){}

};