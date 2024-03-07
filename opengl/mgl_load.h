class MaticalsGlTexture{
public:
	GLint texture, width, height;
	
	MaticalsGlTexture(){
		texture = -1;
		width = 0;
		height = 0;
	}

	MaticalsGlTexture(GLint t, GLint w = 0, GLint h = 0){
		texture = t;
		width = w;
		height = h;
	}

	void Set(GLint t, GLint w, GLint h){
		texture = t;
		width = w;
		height = h;
	}

	operator GLint(){
		return texture;
	}

	void Clean(){
		texture = -1;
		width = 0;
		height = 0;
	}

};

#ifdef MGLSL_USE_LOAD_IMAGE

class MaticalsGlslImagesEl{
	friend class MaticalsGlslImages;

	MString file;
	GLuint texture;
	int64 ltime;

	int width, height;

public:
	MaticalsGlTexture GetTexture(){
		return MaticalsGlTexture(texture, width, height);
	}

};

class MaticalsGlslImages{
	OList<MaticalsGlslImagesEl> images;

public:
	MaticalsGlslImages(){}

	MaticalsGlTexture Load(VString file){
		UGLOCK(images);

		if(IsFile(file)){
			MaticalsGlslImagesEl *el = Find(file);
			if(el){
				sstat64 st = GetFileInfo(file);
				if(st.st_mtime != el->ltime){
					if(Load(el, file))
						return MaticalsGlTexture(el->texture, el->width, el->height);

					return -1;
				}

				return MaticalsGlTexture(el->texture, el->width, el->height);
			}

			el = images.NewE();
			el->texture = -1;
			if(Load(el, file))
				return MaticalsGlTexture(el->texture, el->width, el->height);
		}

		return -1;
	}

	int Load(MaticalsGlslImagesEl *el, VString file){
		MWndIPE ipe;

		if(el->texture != -1)
			glDeleteTextures(1, &el->texture);

		if(ipe.Load(file)){
			GLuint t = 0;
			//glEnable(GL_TEXTURE_2D);
			glGenTextures( 1, &t );
			glBindTexture(GL_TEXTURE_2D, t);

			// Set parameters to determine how the texture is resized
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			// Set parameters to determine how the texture wraps at edges
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T, GL_REPEAT );
			// Read the texture data from file and upload it to the GPU
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ipe.GetWidth(), ipe.GetHeight(), 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, ipe.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			el->texture = t;
			el->file = file;

			sstat64 st = GetFileInfo(file);
			el->ltime = st.st_mtime;

			el->width = ipe.GetWidth();
			el->height = ipe.GetHeight();

			return 1;
		}

		images.Free(el);

		return 0;
	}

	MaticalsGlslImagesEl* Set(VString name, MWndIPE *ipe = 0){
		UGLOCK(images);

		MaticalsGlslImagesEl *el = Find(name);
		if(!el)
			el = Create(name, ipe);

		if(!ipe)
			return el;

		glBindTexture(GL_TEXTURE_2D, el->texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ipe->GetWidth(), ipe->GetHeight(), 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, ipe->GetData());
		el->width = ipe->GetWidth();
		el->height = ipe->GetHeight();

		return el;
	}

	MaticalsGlslImagesEl* Set(VString name, KiVec2 size){
		UGLOCK(images);

		MaticalsGlslImagesEl *el = Find(name);
		if(!el)
			el = Create(name, 0);

		glBindTexture(GL_TEXTURE_2D, el->texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, 0);
		el->width = size.x;
		el->height = size.y;

		return el;
	}

	MaticalsGlslImagesEl* Create(VString name, MWndIPE *ipe = 0){
		UGLOCK(images);

		MaticalsGlslImagesEl *el = images.NewE();
		el->file = name;

		GLuint t = 0;
		//glEnable(GL_TEXTURE_2D);
		glGenTextures( 1, &t );
		glBindTexture(GL_TEXTURE_2D, t);

		// Set parameters to determine how the texture is resized
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		// Set parameters to determine how the texture wraps at edges
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		// Read the texture data from file and upload it to the GPU
		
		if(ipe){
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ipe->GetWidth(), ipe->GetHeight(), 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, ipe->GetData());
			el->width = ipe->GetWidth();
			el->height = ipe->GetHeight();
		}
		
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		el->texture = t;
		el->file = name;
		el->ltime = 0;

		return el;
	}

	void SetFilter(MaticalsGlslImagesEl *el, GLuint fmin, GLuint fmax){
		if(!el || !el->texture)
			return ;

		glBindTexture(GL_TEXTURE_2D, el->texture);
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER, fmin );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER, fmax );
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	MaticalsGlslImagesEl* Find(VString file){
		UGLOCK(images);
		MaticalsGlslImagesEl *el = 0;

		while(el = images.Next(el)){
			if(el->file == file)
				return el;
		}

		return 0;
	}

	void Clean(MaticalsGlslImagesEl *el){
		if(el->texture != -1){
			glDeleteTextures(1, &el->texture);
			el->texture = -1;
		}
	}

	void Clean(){
		UGLOCK(images);
		MaticalsGlslImagesEl *el = 0;

		while(el = images.Next(el)){
			Clean(el);
		}

		images.Clean();
	}

	~MaticalsGlslImages(){
		Clean();
	}

};

#endif

#ifdef MGLSL_USE_LOAD_VIDEO
class MaticalsGlslVideosEl{
	friend class MaticalsGlslVideos;

	MString file;
	GLuint texture;
	int flag_update;
	int64 ltime;

	AVFormatContext *formatx;
	int nstream;
	const AVCodec *codec;
    AVStream *stream;
	AVCodecContext *codecx;
    AVFrame *rawframe;
    AVFrame *rgbframe;
	SwsContext *swsx;

	int opt_start_frame;
	int opt_repeat, opt_repeat_mirror;

	MString data_mirror;
	bool flag_mirror_ok;
	//int *opt_mirror;
};

#define MGLV_REPEAT_NO	0
#define MGLV_REPEAT_YES	1	
#define MGLV_REPEAT_MIRROR	2

class MaticalsGlslVideos{
	OList<MaticalsGlslVideosEl> video;

public:
	MaticalsGlslVideos(){}

	MaticalsGlTexture Load(VString file, XDataEl *pel){
		UGLOCK(video);

		// File
		if(IsFile(file)){
			MaticalsGlslVideosEl *el = Find(file);
			if(el){
				sstat64 st = GetFileInfo(file);
				if(st.st_mtime != el->ltime){
					if(Load(el, file, pel))
						return MaticalsGlTexture(el->texture, el->codecx->width, el->codecx->height);

					return -1;
				}

				Seek(el, 0);
				el->flag_update = 1;
				return MaticalsGlTexture(el->texture, el->codecx->width, el->codecx->height);
			}

			// Init
			el = video.NewE();
			el->texture = -1;
			el->formatx = 0;
			el->codecx = 0;
			el->rawframe = 0;
			el->rgbframe = 0;
			el->swsx = 0;

			// Load
			if(Load(el, file, pel))
				return MaticalsGlTexture(el->texture, el->codecx->width, el->codecx->height);
		}

		return -1;
	}

	int Load(MaticalsGlslVideosEl *el, VString file, XDataEl *pel){
		Clean(el);

		avdevice_register_all();

		int ret = avformat_open_input(&el->formatx, el->file = file, NULL, NULL);
		if(ret < 0){ Clean(el); video.Free(el); return 0; }

		MaticalsGlslVideosEl &d = *el;

		avformat_find_stream_info(el->formatx, NULL);
		el->nstream = av_find_best_stream(el->formatx, AVMEDIA_TYPE_VIDEO, -1, -1, &el->codec, 0);
		d.stream = d.formatx->streams[d.nstream];
		d.codecx = avcodec_alloc_context3(d.codec);
		avcodec_parameters_to_context(d.codecx, d.stream->codecpar);
		avcodec_open2(d.codecx, NULL, NULL);
		d.rawframe = av_frame_alloc();
		d.rgbframe = av_frame_alloc();
		d.rgbframe->format = AV_PIX_FMT_RGB24;
		d.rgbframe->width = d.codecx->width;
		d.rgbframe->height = d.codecx->height;
		av_frame_get_buffer(d.rgbframe, 1);
		d.swsx = sws_getContext(d.codecx->width, d.codecx->height, d.codecx->pix_fmt,
								d.codecx->width, d.codecx->height, AV_PIX_FMT_RGB24,
								SWS_POINT, NULL, NULL, NULL);



		//av_dump_format(d.formatx, 0, infile, 0);
//		dump_timebase("in stream", d.stream);
//		dump_timebase("in stream:codec", d.stream->codec); // note: deprecated
//		dump_timebase("in codec", d.codecx);

		// OpenGL
		GLuint t = 0;
		//glEnable(GL_TEXTURE_2D);
		glGenTextures( 1, &t );
		glBindTexture(GL_TEXTURE_2D, t);

		// Set parameters to determine how the texture is resized
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		// Set parameters to determine how the texture wraps at edges
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T, GL_REPEAT );

		el->texture = t;
		el->file = file;

		sstat64 st = GetFileInfo(file);
		el->ltime = st.st_mtime;
		el->flag_update = 1;

		// Options
		el->opt_start_frame = 0;
		el->opt_repeat = MGLV_REPEAT_YES;
		el->opt_repeat_mirror = 0;
		el->flag_mirror_ok = 0;

		if(pel){
			while(pel){
				VString k = pel->k(), v = pel->v();

				if(k == "file"){}
				else if(k == "start_frame"){
					el->opt_start_frame = v.toi();
				}
				else if(k == "repeat"){
					if(v == "no")
						el->opt_repeat = MGLV_REPEAT_NO;
					else if(v == "yes")
						el->opt_repeat = MGLV_REPEAT_YES;
					else if(v == "mirror")
						el->opt_repeat = MGLV_REPEAT_MIRROR;
					else
						print("MaticalsGlslVideos.Load() error: unknown options: '", k, "' = '", v, "'.\r\n");
				} else
					print("MaticalsGlslVideos.Load() error: unknown options: '", k, "' = '", v, "'.\r\n");

				pel = pel->n();
			}
		}

		if(el->opt_repeat == MGLV_REPEAT_MIRROR){
			//el->data_mirror.Reserve(el->stream->nb_frames * sizeof(int)); 
#ifdef MS_RENDER_BIGSIZE
			el->data_mirror.Reserve(el->stream->nb_frames * sizeof(char*), 0); 
#else
			el->data_mirror.Reserve(el->stream->nb_frames * 3 * d.codecx->width * d.codecx->height); 
#endif

			//if(!el->data_mirror.data){
			//	el->data_mirror.data = (unsigned char*)malloc(sz+1);
			//}

			//el->opt_mirror = (int*)el->data_mirror.data;
		}

		return 1;
	}

	int Update(int frame){
		UGLOCK(video);
		MaticalsGlslVideosEl *el = 0;
		glActiveTexture(GL_TEXTURE31);

		while(el = video.Next(el)){
			if(el->flag_update && el->opt_start_frame <= frame && ReadFrame(el)){
				glBindTexture(GL_TEXTURE_2D, el->texture);
				//glPixelStorei(GL_UNPACK_FLIP_Y_WEBGL, false);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, el->codecx->width, el->codecx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, el->rgbframe->data[0]);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		return 1;
	}

	void CleanUpdateFlag(){
		UGLOCK(video);
		MaticalsGlslVideosEl *el = 0;

		while(el = video.Next(el)){
			el->flag_update = 0;
		}

		return ;
	}

	void MemCpy(MaticalsGlslVideosEl *el, AVFrame *frame, int id){
#ifndef MS_RENDER_BIGSIZE
		memcpy(frame->data[0], el->data_mirror.data + id * 3 * frame->width * frame->height, 3 * frame->width * frame->height);
#else
		char **p = (char**)(el->data_mirror.data);
		memcpy(frame->data[0], p[id], 3 * frame->width * frame->height);				
#endif
	}

	int decode_packet(MaticalsGlslVideosEl *el, const AVPacket *pkt, int &haveframe){
		int ret = avcodec_send_packet(el->codecx, pkt);
		if(ret < 0){
			char e[AV_ERROR_MAX_STRING_SIZE];			
			print("Error while sending a packet to the decoder: ", av_make_error_string(e, AV_ERROR_MAX_STRING_SIZE, ret), ".\r\n");
			return ret;
		}

		while(ret >= 0){
			ret = avcodec_receive_frame(el->codecx, el->rawframe);
			if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
				break;
			} else if(ret < 0){
				char e[AV_ERROR_MAX_STRING_SIZE];			
				print("Error while receiving a frame from the decoder: ", av_make_error_string(e, AV_ERROR_MAX_STRING_SIZE, ret), ".\r\n");
				return ret;
			} else if(ret >= 0){
				haveframe = 1;
				return 1;
			}
		}

		return 0;
	}

	bool ReadFrame(MaticalsGlslVideosEl *el){
		AVPacket packet = {0};
		int err = 0, haveframe = 0;
		MaticalsGlslVideosEl &d = *el;

		if(el->flag_mirror_ok){
			if(!el->opt_repeat_mirror){
				if(d.codecx->frame_number < el->stream->nb_frames - 2){
					MemCpy(el, d.rgbframe, d.codecx->frame_number);
					//memcpy(d.rgbframe->data[0], el->data_mirror.data + d.codecx->frame_number * 3 * d.rgbframe->width * d.rgbframe->height, 3 * d.rgbframe->width * d.rgbframe->height);
					d.codecx->frame_number ++;
				} else{
					el->opt_repeat_mirror = d.codecx->frame_number - 1;
				}
			}

			if(el->opt_repeat_mirror){
				MemCpy(el, d.rgbframe, el->opt_repeat_mirror);
				//memcpy(d.rgbframe->data[0], el->data_mirror.data + el->opt_repeat_mirror * 3 * d.rgbframe->width * d.rgbframe->height, 3 * d.rgbframe->width * d.rgbframe->height);
				el->opt_repeat_mirror --;
				d.codecx->frame_number = el->opt_repeat_mirror;
			} 

			return 1;
		}

		if(el->opt_repeat_mirror){
			//av_seek_frame(el->formatx, el->nstream, el->opt_mirror[el->opt_repeat_mirror], AVSEEK_FLAG_BACKWARD);
			//Seek(el, el->opt_repeat_mirror - 1);
			//d.codecx->frame_number = el->opt_repeat_mirror;

			memcpy(d.rgbframe->data[0], el->data_mirror.data + el->opt_repeat_mirror * 3 * d.rgbframe->width * d.rgbframe->height, 3 * d.rgbframe->width * d.rgbframe->height);
			el->opt_repeat_mirror --;
			
			if(el->opt_repeat_mirror == 0){
				//d.codecx->frame_number = 0;
				Seek(el, 0);
				ReadFrame(el);
			}
			
			return 1;
		}

		while(!haveframe && err >= 0 && ((err = av_read_frame(d.formatx, &packet)) >= 0)){
			if(packet.stream_index == d.nstream)
				err = decode_packet(el, &packet, haveframe);
			
			av_packet_unref(&packet);
		}

		//// read
		//while (!haveframe && err >= 0 && ((err = av_read_frame(d.formatx, &packet)) >= 0)){
		//   if(packet.stream_index == d.nstream){
		//	   //err = avcodec_receive_frame(d.codecx, d.rawframe);
		//	   err = avcodec_decode_video2(d.codecx, d.rawframe, &haveframe, &packet);
		//	  
		//	   if(err == AVERROR(EAGAIN) || err == AVERROR_EOF){
		//		   break;
		//	   }
		//	   
		//	   if(err >= 0)
		//		   haveframe = 1;
		//   }

		//   av_packet_unref(&packet);
		//}

		//Seek(el, d.codecx->frame_number + 1);
		//av_init_packet(&packet);

		//int ret = avcodec_send_packet(d.codecx, &packet);
		//if (ret < 0) {
		//	print("Error sending a packet for decoding\n");
		//	//exit(1);
		//	return 0;
		//}

		//while (ret >= 0) {
		//	ret = avcodec_receive_frame(d.codecx, d.rawframe);
		//	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
		//		haveframe = 0;
		//		break;
		//	}
		//	else if (ret < 0) {
		//		print("Error during decoding\n");
		//		return 0;
		//	}

		//	haveframe = 1;
		//	break;
		//}
	
        /* the picture is allocated by the decoder. no need to
           free it * /
        snprintf(buf, sizeof(buf), filename, dec_ctx->frame_number);
        pgm_save(frame->data[0], frame->linesize[0],
                 frame->width, frame->height, buf);
    }

		// read
		while (!haveframe && err >= 0 && ((err = av_read_frame(d.formatx, &packet)) >= 0)){
		   if(packet.stream_index == d.nstream){
			   err = avcodec_receive_frame(d.codecx, d.rawframe);
			   //err = avcodec_decode_video2(d.codecx, d.rawframe, &haveframe, &packet);
		   }

		   av_packet_unref(&packet);
		}*/

		// error output
		if(!haveframe && err != AVERROR_EOF){
			char buf[500];
			av_strerror(err, buf, sizeof(buf) - 1);
			buf[499] = 0;
			printf("read_frame: %s\n", buf);
		}

		// convert to rgb
		if (haveframe){
			//d.rawframe->data[1] = d.rawframe->data[1] + d.rawframe->linesize[1] * (d.rawframe->height / 2 - 1);
			//d.rawframe->data[2] = d.rawframe->data[2] + d.rawframe->linesize[2] * (d.rawframe->height / 2 - 1);

			d.rawframe->data[0] = d.rawframe->data[0] + d.rawframe->linesize[0] * (d.codecx->height - 1);
			  d.rawframe->data[1] = d.rawframe->data[1] + d.rawframe->linesize[0] * d.codecx->height / 4 - 1;
			  d.rawframe->data[2] = d.rawframe->data[2] + d.rawframe->linesize[0] * d.rawframe->height / 4 - 1;
			  d.rawframe->linesize[0] *= -1;
			  d.rawframe->linesize[1] *= -1;
			  d.rawframe->linesize[2] *= -1;

			sws_scale(d.swsx, d.rawframe->data, d.rawframe->linesize, 0, d.rawframe->height,
					  d.rgbframe->data, d.rgbframe->linesize);

			if(el->opt_repeat == MGLV_REPEAT_MIRROR && el->opt_repeat_mirror == 0 && d.codecx->frame_number < el->stream->nb_frames){
#ifndef MS_RENDER_BIGSIZE
				memcpy(el->data_mirror.data + (d.codecx->frame_number - 1) * 3 * d.rgbframe->width * d.rgbframe->height,
					d.rgbframe->data[0], 3 * d.rgbframe->width * d.rgbframe->height);
#else
				char **p = (char**)(el->data_mirror.data);
				p[d.codecx->frame_number - 1] = (char*)malloc(3 * d.rgbframe->width * d.rgbframe->height);
				memcpy(p[d.codecx->frame_number - 1], d.rgbframe->data[0], 3 * d.rgbframe->width * d.rgbframe->height);
#endif
			}

		} else{
			if(el->opt_repeat == MGLV_REPEAT_YES)
				Seek(el, 0);
			if(el->opt_repeat == MGLV_REPEAT_MIRROR){
				//Seek(el, 0);
				el->opt_repeat_mirror = min(d.codecx->frame_number - 1, el->stream->nb_frames - 1);
				el->flag_mirror_ok = 1;
				//Seek(el, d.codecx->frame_number);
			}
		}

		return haveframe != 0;
	}

	bool Seek(MaticalsGlslVideosEl *el, int frameIndex){
		if(!el->formatx)
			return false;
		
		//timeBase = (int64_t(pCodecCtx->time_base.num) * AV_TIME_BASE) / int64_t(pCodecCtx->time_base.den);
		int timBase = (int64_t(el->stream->time_base.num) * AV_TIME_BASE) / int64_t(el->stream->time_base.den);
		int64_t seekTarget = int64_t(frameIndex) * timBase;

		int64_t seek_target;
		AVRational ar;
		ar.num = 1;
		ar.den = AV_TIME_BASE;
		seek_target = av_rescale_q(frameIndex * AV_TIME_BASE / 30, ar, el->stream->time_base);

		int flag = el->opt_repeat_mirror ? AVSEEK_FLAG_BACKWARD : AVSEEK_FLAG_ANY;

		//if(av_seek_frame(el->formatx, -1, seek_target, 0) < 0){
		if(av_seek_frame(el->formatx, -1, seekTarget, AVSEEK_FLAG_ANY) < 0){
			print("MaticalsGlslVideos.Seek() failed.\r\n");
			return 0;
		}

		return 1;
	}

	MaticalsGlslVideosEl* Find(VString file){
		UGLOCK(video);
		MaticalsGlslVideosEl *el = 0;

		while(el = video.Next(el)){
			if(el->file == file)
				return el;
		}

		return 0;
	}

template <typename T>
	static void dump_timebase (const char *what, const T *o) {
		if (o)
			printf("%s timebase: %d/%d\n", what, o->time_base.num, o->time_base.den);
		else
			printf("%s timebase: null object\n", what);
	}

	void Clean(MaticalsGlslVideosEl *el){
		if(el->texture != -1){
			glDeleteTextures(1, &el->texture);
			el->texture = -1;
		}

		if(el->formatx)
			avformat_close_input(&el->formatx);

		if(el->codecx){
			avcodec_close(el->codecx);
			av_free(el->codecx);
		}

		av_frame_free(&el->rawframe);
		av_frame_free(&el->rgbframe);

		if(el->swsx){
			sws_freeContext(el->swsx);
			el->swsx = 0;
		}

		return ;
	}

	void Clean(){
		UGLOCK(video);
		MaticalsGlslVideosEl *el = 0;

		while(el = video.Next(el)){
			Clean(el);
		}

		video.Clean();
	}

	~MaticalsGlslVideos(){
		Clean();
	}

};

#else
class MaticalsGlslVideosEl{

};

class MaticalsGlslVideos{
public:
	void Update(float){}
};

#endif


#ifdef MGLSL_USE_LOAD_AUDIO
// https://gist.github.com/jimjibone/6569303

class MaticalsAudioLoad{
	MString file;
	AVFormatContext *formatContext;
	AVCodec			*codec;
	AVStream		*stream;
	AVFrame			*frame;
	int streamIndex;
	int needsResample;

	uint8_t **srcData, **dstData; // Only used for resampling.

	struct SwrContext *swrContext;

	struct AudioInfo {
		// Audio resampling info.
		int64_t src_ch_layout, dst_ch_layout;
		int src_rate, dst_rate;
		int src_nb_channels, dst_nb_channels;
		int src_linesize, dst_linesize;
		int src_nb_samples, dst_nb_samples, max_dst_nb_samples;
		enum AVSampleFormat src_sample_fmt, dst_sample_fmt;
		int dst_bufsize;
	} info;

public:

	MaticalsAudioLoad(){
		formatContext = 0;
		needsResample = 0;
		codec = 0;
		frame = 0;
	}

	int GetLay(){
		if(!formatContext || !stream || !stream->codec)
			return 0;

		return stream->codec->channel_layout;		
	}

	int GetRate(){
		if(!formatContext || !stream || !stream->codec)
			return 0;

		return stream->codec->sample_rate;		
	}

	AVSampleFormat GetFmt(){
		if(!formatContext || !stream || !stream->codec)
			return AV_SAMPLE_FMT_NONE;

		return stream->codec->sample_fmt;		
	}

	int GetNbSam(){
		if(!formatContext || !stream || !stream->codec)
			return 0;

		return 1024 * 4;
	}

	int Open(VString in_file){
		file = in_file;

		int result = avformat_open_input(&formatContext, file, NULL, NULL);
		if(result < 0){
			return 0;
		}

		result = avformat_find_stream_info(formatContext, NULL);
		if(result < 0){
			return 0;
		}

		streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
		if (streamIndex < 0) {
			return 0;
		}

		stream = formatContext->streams[streamIndex];
		int duration = stream->time_base.num * (int)stream->duration / stream->time_base.den;

		// Open
		result = avcodec_open2(stream->codec, codec, NULL);
		if (result < 0){
			return 0;
		}

		if (stream->codec->channels <= 0) {
			return 0;
		}

		return 1;
	}

	// This function does the job of `av_samples_alloc()` because it seems to fail on my computer...
int jr_av_samples_alloc_array_and_samples(uint8_t ***audio_data, int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
{
	int ret, nb_planes = av_sample_fmt_is_planar(sample_fmt) ? nb_channels : 1;

	*audio_data = (uint8_t **)av_calloc(nb_planes, sizeof(**audio_data));
	if (!*audio_data)
		return AVERROR(ENOMEM);
	ret = av_samples_alloc(*audio_data, linesize, nb_channels, nb_samples, sample_fmt, align);
	if (ret < 0)
		av_freep(audio_data);
		return ret;
}


	int EnableResample(int lay = AV_CH_LAYOUT_STEREO, int rate = 44100){
		swrContext = swr_alloc();
		if (!swrContext) {
			//result = AVERROR(ENOMEM);
			//*error = CRERROR_DECODER_RESMPL;
			//throw std::runtime_error("Decoder - Could not allocate resampler context.");
			return 0;
		}
		
		// Get some information about the audio.
		//info.src_ch_layout = stream->codec->channel_layout;
		info.src_ch_layout = AV_CH_LAYOUT_MONO;

		info.dst_ch_layout = lay;
		info.src_rate = stream->codec->sample_rate;
		info.dst_rate = rate;
		info.src_nb_channels = av_get_channel_layout_nb_channels(info.src_ch_layout);
		info.dst_nb_channels = av_get_channel_layout_nb_channels(info.dst_ch_layout);
		//info.src_linesize = //done during decode/resample
		//info.dst_linesize = //done during decode/resample
		info.src_nb_samples = 1024;
		info.dst_nb_samples = 0;
		info.max_dst_nb_samples = 0;
		info.src_sample_fmt = stream->codec->sample_fmt;
		info.dst_sample_fmt = AV_SAMPLE_FMT_FLTP; //AV_SAMPLE_FMT_U8P;

		// Set the resample options.
		av_opt_set_int(swrContext,			"in_channel_layout",	info.src_ch_layout, 0);
		av_opt_set_int(swrContext,			"in_sample_rate",		info.src_rate, 0);
		av_opt_set_sample_fmt(swrContext,	"in_sample_fmt",		info.src_sample_fmt, 0);
		
		av_opt_set_int(swrContext,			"out_channel_layout",	info.dst_ch_layout, 0);
		av_opt_set_int(swrContext,			"out_sample_rate",		info.dst_rate, 0);
		av_opt_set_sample_fmt(swrContext,	"out_sample_fmt",		info.dst_sample_fmt, 0);
		
		// Do we need to resample?
		if (info.src_ch_layout != info.dst_ch_layout ||
			info.src_rate != info.dst_rate ||
			info.src_sample_fmt != info.dst_sample_fmt
			) {
			needsResample = true;
		} else
			needsResample = 0;
		
		// Initialise the resample context.
		int result = swr_init(swrContext);
		if (result < 0) {
			//*error = CRERROR_DECODER_RESMPL;
			//throw std::runtime_error("Decoder - Could not initialise resampler context.");
			return 0;
		}
		
		// Allocate the resample destination buffer.
		if (needsResample) {
			/* compute the number of converted samples: buffering is avoided
			 * ensuring that the output buffer will contain at least all the
			 * converted input samples */
			info.max_dst_nb_samples = info.dst_nb_samples = (int)av_rescale_rnd(info.src_nb_samples, info.dst_rate, info.src_rate, AV_ROUND_UP);
			
			result = jr_av_samples_alloc_array_and_samples(&dstData, &info.dst_linesize, info.dst_nb_channels, info.dst_nb_samples, info.dst_sample_fmt, 0);
			//result = av_samples_alloc(dstData, &info.dst_linesize, info.dst_nb_channels, info.dst_nb_samples, info.dst_sample_fmt, 0);
			if (result < 0) {
				//*error = CRERROR_DECODER_RESMPL;
				//throw std::runtime_error("Decoder - Could not allocate resample destination samples.");
				return 0;
			}
		}




	}

	int Read(){
		AVPacket pkt;
        av_init_packet(&pkt);
		int result;

		if(!frame)
			frame = av_frame_alloc();

		if (av_read_frame(formatContext, &pkt) < 0) {
				return 0;
		}

		if(pkt.stream_index != streamIndex)
			return 1;

		int gotFrame = 0;
		int consumed = avcodec_decode_audio4(stream->codec, frame, &gotFrame, &pkt);
		if (consumed < 0) {
			return 0;
		}

		if(gotFrame){
			srcData = frame->data;

			// Resample if necessary.
					if (needsResample) {
						
						info.dst_nb_samples = (int)av_rescale_rnd(swr_get_delay(swrContext, info.src_rate) + info.src_nb_samples, info.src_rate, info.dst_rate, AV_ROUND_UP);
						if (info.dst_nb_samples > info.max_dst_nb_samples) {
							av_free(dstData[0]);
							result = jr_av_samples_alloc_array_and_samples(&dstData, &info.dst_linesize, info.dst_nb_channels, info.dst_nb_samples, info.dst_sample_fmt, 0);
							//result = av_samples_alloc(dstData, &info.dst_linesize, info.dst_nb_channels, info.dst_nb_samples, info.dst_sample_fmt, 1);
							if (result < 0)
								return 0;
							info.max_dst_nb_samples = info.dst_nb_samples;
						}
						
						// Convert to destination format.
						#if cratesanalyser_CAST_SRC_DATA
						int resampled = swr_convert(swrContext, dstData, info.dst_nb_samples, (const uint8_t**)srcData, info.src_nb_samples);
						#else
						int resampled = swr_convert(swrContext, dstData, info.dst_nb_samples, (const uint8_t**)srcData, info.src_nb_samples);
						#endif
						if (resampled < 0) {
							swr_free(&swrContext);
							//*error = CRERROR_DECODER_CONVRT;
							//throw std::runtime_error("Error while converting\n");
							return 0;
						}
						info.dst_bufsize = av_samples_get_buffer_size(&info.dst_linesize, info.dst_nb_channels, resampled, info.dst_sample_fmt, 1);
						
						// Add the resampled samples to the store.
						//length = MIN(remaining, frame->nb_samples * info.src_nb_channels);
						srcData = dstData;
						
						//int oldCount = audio.getSampleCount();
						
						//audio.addToSampleCount(info.dst_nb_samples);
						
						//for (int i = 0; i < info.dst_nb_samples; i++) {
						//	audio.setSample(oldCount+i, (float)dstData[0][i]);
						//}
						
					} // End resample.
					else {
						//length = MIN(remaining, frame->nb_samples * info.src_nb_channels);
						//int oldCount = audio.getSampleCount();
						
						//audio.addToSampleCount(info.src_nb_samples);
						
						//for (int i = 0; i < length; i++) {
						//	audio.setSample(oldCount+i, (float)srcData[0][i]);
						//}						

					} // End no resample needed.

		}// end if gotFrame.

		av_free_packet(&pkt);

		return 1;
	}

	/*
	int GetDataSize(){
		if(!frame)
			return 0;

		return 0;

		return info.dst_nb_samples;
		return frame->nb_samples;
	}*/

	uint8_t ** GetData(){
		if(!frame)
			return 0;

		return srcData;
	}

	void Close(){
		if(frame){
			av_frame_free(&frame);
			frame = 0;
		}

		if(formatContext){
			avformat_close_input(&formatContext);
			formatContext = 0;
		}
	}

	~MaticalsAudioLoad(){
		Close();
	}

};

#else
class MaticalsAudioLoad{

};
#endif

MString ShadertoyReplace(VString code, LString &ls){
	LString ret;
	VString l;

	ls.Clean();

	while(code){
		l = PartLine(code, code, " mainImage(");
		if(code){
			ls + l + " mainImage(";
			l = PartLine(code, code, "out");
			ls + l + "inout";
		} else
			ls + l;
	}

	return ls;
}

bool ShadertoyImport(VString url, TString &save_path){
	GetHttp gp;
	LString ls;
	XDataCont ct;

	VString name = PartLineOT(url, "view/");
	if(!name)
		return 0;

	ls + "s=%7B%20%22shaders%22%20%3A%20%5B%22" + name + "%22%5D%20%7D&nt=0&nl=0&np=0";

	print("\r\nShadertoyImport(", url, ").\r\n");
	gp.SetPost(ls);
	
	// Next
	gp.SetHeader("\
Origin: https://www.shadertoy.com\r\n\
Referer: https://www.shadertoy.com/embed/ololo?gui=true&t=10&paused=true&muted=false\r\n\
Sec-Fetch-Dest: empty\r\n\
Sec-Fetch-Mode: cors\r\n\
Sec-Fetch-Site: same-origin\r\n\
TE: trailers\r\n\
");
	int res = gp.Request("https://www.shadertoy.com/shadertoy");
	VString data = gp.GetData();
	
	// Json
	ct.ReadJson(data.str(1, -2));
	XDataPEl el(ct.GetData());

	// Folder
	MString path;
	path.Add("shadertoy/", name, "/");
	MkDir(path);

	// Configure
	JsonEncode json;
	MgGenChan gc;

	json.Up("info");
	json("id", el("info.id"));
	json("url", url);
	json("name", el("info.name"));
	json("username", el("info.username"));
	json("description", el("info.description"));
	json.Down();

	json.Up("vertex");
	json("file", "vertex.glsl");
	json.Down();

	// Render
	XDataEl *e = el("renderpass.:"), *ei;
	while(e){
		VString type = e->GetVal("type");
		VString code = e->GetVal("code");
		VString key, val;
		TString tmp;

		if(type == "image"){
			key = "fragment";
			val = "fragment.glsl";
		}else if(type == "common"){
			key = "common";
			val = "common.glsl";
		} else if(type == "buffer"){
			int id = e->GetVal("name")[7] - 'A';
			key = gc.GenBuffName(id);
			val = tmp.Add(key, ".glsl");
		}

		json.Up(key);
		json("file", val);

		print("---[", val, "]---\r\n");

		if(ei = e->Get("inputs.:")){
			while(ei){
				VString id = ei->GetVal("id");
				VString type = ei->GetVal("type");
				VString chan = ei->GetVal("channel");

				if(type == "texture" || type == "video"){
					VString file = ei->GetVal("filepath");			

					print("(", chan, ") ", file, "\r\n");

					GetHttp gp;
					ILink il(Replace(file, "\\", ""));

					gp.SetHeader("\
Origin: https://www.shadertoy.com\r\n\
Referer: https://www.shadertoy.com/embed/ololo?gui=true&t=10&paused=true&muted=false\r\n\
Sec-Fetch-Dest: empty\r\n\
Sec-Fetch-Mode: cors\r\n\
Sec-Fetch-Site: same-origin\r\n\
TE: trailers\r\n\
");
					gp.Request(LString() + "https://www.shadertoy.com/" + il.GetPathFile());
					if(gp.GetHead("Location")){
						gp.Request(gp.GetHead("Location"));
					}

					TString dpath;
					dpath.Add(path, type == "texture" ? "/images/" : "/video/");

					MkDir(LString() + dpath);
					SaveFile(LString() + dpath + il.file, gp.GetData());

					if(type == "texture")
						json(gc.GenChanName(chan.toi()), LString() + "/images/" + il.file);
					else 
						json(gc.GenChanName(chan.toi()), LString() + "/video/" + il.file);
				} else if(type == "buffer"){
					XDataEl *er = el("renderpass.:");
					while(er){
						if(er->GetVal("outputs.:.id") == id){
							break;
						}

						er = er->n();
					}

					// Result
					if(er && er->GetVal("name").str(0, -1) == "Buffer "){
						int id = er->GetVal("name")[7] - 'A';

						print("(", chan, ") ", gc.GenBuffName(id), "\r\n");
						json(gc.GenChanName(chan.toi()), gc.GenBuffName(id));
					} else{
						print("!!!(", chan, ") NOT FOUND '", id, "'.\r\n");
					}
				} else{
					print("!!!(", chan, ") Unknown type: ", type ," '", id, "'.\r\n");
				}

				ei = ei->n();
			}
		}
		
		// Save
		SaveFile(LString() + path + val, ShadertoyReplace(prmf_json_encode(code), ls));

		json.Down();
		e = e->n();
	}

	SaveFile(LString() + path + "vertex.glsl", LoadFile("shadertoy/vertex.glsl"));

	// Save project
	json.Save(LString() + path + "configure.json");

	// Result path
	save_path = VString(LString() + "shadertoy/" + name);

	print("ShadertoyImport(). Save to: ", save_path, ".\r\n\r\n");

	return 1;
}
