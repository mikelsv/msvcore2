ConfLineOptions::ConfLineOptions(){
	options = 0;
}

ConfLineOptions::ConfLineOptions(VString file){
	options = 0;
	LoadFile(file);
}

ConfLineOptions::~ConfLineOptions(){
	confdata.Clean();
	options = 0;
}

// Load
int ConfLineOptions::LoadFile(VString file, int included){
	if(!cfile || file[0] == '/' || file[0] == '\\' || file[1] == ':')
		cfile = file;
	else{
		unsigned char *p = cfile.endu() - 1;
		while(p >= cfile.data){
			if(*p == '/' || *p == '\\'){
				p ++;
				break;
			}
			p --;
		}
			
		if(p < cfile.data)
			cfile = file;
		else{
			int s = p - cfile.data;
			VString d(confdata.addnfv(s + file.sz), s + file.sz);
			memcpy(d.data, cfile, s);
			memcpy(d.data + s, file, file);
			cfile = d;
		}
	}
		
	return Load(::LoadFile(cfile), included);
}

int ConfLineOptions::Load(VString line, int included){
	if(!included){
		confdata.Clean();
		options = 0;
		err.Clean();
	}

	int lnum = 0;
	Read(line, 0, lnum);
		
	return !err;
}

// Pointer
ConfLineOption* ConfLineOptions::GetFirst(){
	return options;
}

ConfLineOption* ConfLineOptions::GetEnd(){
	ConfLineOption *opt = options;

	if(!opt)
		return 0;

	while(opt->next)
		opt = opt->next;

	return opt;
}

ConfLineOption* ConfLineOptions::Next(ConfLineOption *opt){
	if(!opt)
		return options;
	return opt->next;
}

// Find
ConfLineOption* ConfLineOptions::FindOption(VString name, ConfLineOption *opt, ConfLineOption *parent){
	if(opt)			
		opt = opt->next;
	else
		if(parent)
			opt = parent->up;
		else
			opt = options;

	for(opt; opt; opt = opt->next){
		if(opt->key.compareu(name))
			return opt;
	}

	return 0;
}

ConfLineOption* ConfLineOptions::FindOption2(VString name, VString name2, ConfLineOption *opt, ConfLineOption *parent){
	if(opt)			
		opt = opt->next;
	else
		if(parent)
			opt = parent->up;
		else
			opt = options;

	for(opt; opt; opt = opt->next){
		if(opt->key == name || opt->key == name2)
			return opt;
	}

	return 0;
}

VString ConfLineOptions::GetOption(VString name, ConfLineOption *opt, ConfLineOption *parent){
	ConfLineOption *r = FindOption(name, opt, parent);

	if(r)
		return r->val;
	return VString();
}

// 
VString ConfLineOptions::DelSpace(VString line){
	// first
	while(line[0] == ' ' || line[0] == '\t'){
		line.data++; line.sz--;
	}

	// end
	while(line.endo() == ' ' || line.endo() == '\t' || line.endo() == '\r'){
		line.sz--;
	}

	return line;
}

VString ConfLineOptions::GetKeyVal(VString line, VString &val){
	line = DelSpace(line);
		
	for(unsigned int i = 0; i < line.sz; i++){
		if(line[i] == ' ' || line[i] == '\t'){
			val = DelSpace(line.str(i));
			return line.str(0, i);
		}
	}

	val.Clean();
	return line;
}

VString ConfLineOptions::GetError(){
	return err;
}

ConfLineOption* ConfLineOptions::NewOption(){
	return (ConfLineOption*)confdata.Add(0, sizeof(ConfLineOption), 1);
}

VString ConfLineOptions::NewOptionVal(VString line){
	return confdata.addnf(line);
}

void ConfLineOptions::Read(VString &data, ConfLineOption *parent, int &lnum, int level){
	ConfLineOption *lopt = !parent ? GetEnd() : 0;
		

	while(data){
		VString line, key, val;
			
			line = PartLine(data, data, "\n");
			key = GetKeyVal(line, val);
			lnum++;

			int up = 0, down = 0;

			// up
			if(val && val.endo() == '{'){
				val.sz--; up = 1;
			}else if(!val && key.endo() == '{'){
				key.sz--; up = 1;
			}

			// down
			if(key[0] == '}'){
				if(level <= 0){
					Error("found } prevous {", lnum);
				}
				return ;
			}

			if(!key){
				if(up)
					Read(data, lopt, lnum, level + 1);
				continue;
			}

			if(lopt)
				lopt = lopt->next = NewOption();
			else
				if(parent)
					parent->up = lopt = NewOption();
				else
					options = lopt = NewOption();

			lopt->up = 0;
			lopt->key = NewOptionVal(key);
			lopt->val = NewOptionVal(val);

			if(up)
			Read(data, lopt, lnum, level + 1);

			if(key == "include-conf"){
				if(level){
					Error(LString() + "included level must be 0!", lnum);
					return ;
				}

				if(!IsFile(val))
					Error(LString() + "included file '" + val + "' not found.", lnum);
				else{
				//Load(::LoadFile(val), 1);
				LoadFile(val, 1);
				lopt = GetEnd();
				}
			}

			// continue;
	}

	if(!data && level){
		Error("not found }", lnum);
	}
}

void ConfLineOptions::Error(VString line, int lnum){
	err.set(LString() + "Error in config file: '" + line + "' on line " + (lnum + 1) + ".\r\n");
	return ;
}
