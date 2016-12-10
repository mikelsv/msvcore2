class msl_extension_file_d{
public:
	int cid;
	Readdir dir;
};

int msl_extension_file_find(msl_extension_file_d &d, int &cid){
	return d.cid == cid;
}

class msl_extension_file{
	OList<msl_extension_file_d> dirs;
	int cids;

public:
	
	msl_extension_file(){
		cids = 100 + (rand() % 100000);
	}

	msl_extension_file_d* GetById(int cid){
		return dirs.Search(msl_extension_file_find, cid);
	}

	int opendir(VString path){
		msl_extension_file_d * con = dirs.NewE();
		con->cid = cids;
		cids += 1 + (rand() % 3);

		if(con->dir.OpenDir(path))
			return con->cid;

		dirs.Free(con);
		return 0;
	}

	MString readdir(int cid){
		msl_extension_file_d *con = GetById(cid);
		if(!con)
			return "";

		SString ss;

		return con->dir.ReadOne(ss);
	}

	int closedir(int cid){
		msl_extension_file_d *con = GetById(cid);
		if(!con)
			return 0;

		dirs.Free(con);
		return 1;
	}

	int is_dir(VString path){
		return IsDir(path);
	}

	int is_file(VString path){
		sstat64 stt = GetFileInfo(path);

		return stt.st_atime && (stt.st_mode & S_IFDIR) == 0;
	}

	MString file_get_contents(VString path){
		return LoadFile(path);
	}

	int file_put_contents(VString path, VString data){
		return SaveFile(path, data);
	}

	int pathinfo(VString path, msl_value &val){
		ILink link(path);
		val.Set("dirname", link.path);
		val.Set("basename", link.file);
		val.Set("extension", link.ext());
		val.Set("filename", link.name());
		return 1;
	}

	int mkdir(VString path){
		return MkDir(path);
	}

	MString exec(VString cmd, int &rc){
		PipeLine ppl;

		LString hls, ehls;
		//int rc;

		int ret = ppl.Run(cmd, rc, hls, ehls, 0);

		return (VString)hls;
	}
	
	virtual int DoCodeFunctionExec(VString name, msl_fl_fargs &args, msl_value &val){
		if(name == "opendir" && args.Sz() == 1){
			val.val = itos(opendir(args[0].val.val));
		}

		else if(name == "readdir" && args.Sz() == 1){
			val.val = readdir(args[0].val.val.toi());
		}

		else if(name == "closedir" && args.Sz() == 1){
			val.val = closedir(args[0].val.val.toi()) ? "1" : "0";
		}

		else if(name == "is_dir" && args.Sz() == 1){
			val.val = is_dir(args[0].val.val) ? "1" : "0";
		}

		else if(name == "is_file" && args.Sz() == 1){
			val.val = is_file(args[0].val.val) ? "1" : "0";
		}

		else if(name == "file_get_contents" && args.Sz() == 1){
			val.val = file_get_contents(args[0].val.val);
		}

		else if(name == "file_put_contents" && args.Sz() == 2){
			val.val = itos(file_put_contents(args[0].val.val, args[1].val.val));
		}

		else if(name == "copy" && args.Sz() == 2){
			val.val = itos(CopyFile(args[0].val.val, args[1].val.val));
		}

		else if(name == "pathinfo" && args.Sz() == 1){
			pathinfo(args[0].val.val, val);
		}

		else if(name == "mkdir" && args.Sz() == 1){
			val.val = itos(mkdir(args[0].val.val));
		}

		else if(name == "exec" && args.Sz() == 2){
			int rc;
			val.val = exec(args[0].val.val, rc);
			if(args[1].pval)
				args[1].pval->val = itos(rc);
		}

		else
			return 0;

		return 1;
	}

	~msl_extension_file(){
		// auto clean
	}

};