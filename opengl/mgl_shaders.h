GLuint CreateShader(VString source, GLenum type, int flag_print = 0);

GLuint CreateShaderFile(VString path, GLenum type){		
	int res = CreateShader(LoadFile(path), type);

	if(!res)
		print("MaticalsGlsl.CreateShaderFile() error: ", path, ".\r\n");

	return res;
}

GLuint CreateShader(VString source, GLenum type, int flag_print){	
	int success, length;

	if (!source)
		return 0;

	// Create shader
	GLuint shader = glCreateShader(type);
	GLchar const* string[] = { source };

	// Source
#ifdef GLSL_ENABLE_PRINTF
	if(flag_print)
		glShaderSourcePrint(shader, 1, string, (GLint*)&source.sz);
	else
		flag_print = 0;
#endif

	if(!flag_print)
		glShaderSource(shader, 1, string, (GLint*)&source.sz);

	// Compile
	glCompileShader(shader);

	// Get status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		TString log;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		log.Reserve(length);		

		glGetShaderInfoLog(shader, length, &length, (GLchar *)log);

		print("MaticalsGlsl.CreateShader() error:\r\n", log, "\r\n");
		SaveFile("elog_create_shader.txt", source);

		if(!MaticalsOpenGl.flag_error)
			MaticalsGlsl.UseErrorProgram(source, log);

		return 0;
	}
	return shader;
}

GLuint CreateFrameBuffer(GLuint &texture, GLuint &fbo){
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//glActiveTexture(GL_TEXTURE0);

	// Configure
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set size
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MaticalsOpenGl.width, MaticalsOpenGl.height, 0, GL_RGBA, GL_FLOAT, 0);

	// Create framebuffer
	glCreateFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Bind texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	//gl.framebufferTexture2D(
	//        gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT1, gl.TEXTURE_2D, texture, 0);                

	//gl.viewport(0, 0, this.canvas.width, this.canvas.height);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	//console.log(glEnumToString(gl, attachmentPoint), glEnumToString(gl, status));
        
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		print("CreateFrameBuffer(). Return FAIL!\r\n");
		return 0;
	}

	return 1;
}