class MaticalsAudioTranscode{
	struct SwrContext *swrContext;
	uint8_t **srcData;
	uint8_t **dstData;
	bool needsResample;
	int result;

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
	MaticalsAudioTranscode(){
		swrContext = 0;
		dstData = 0;
	}

	int Init(int in_lay, int in_rate, AVSampleFormat in_fmt, int src_nb_samples, int out_lay, int out_rate, AVSampleFormat out_fmt){
		Close();

		swrContext = swr_alloc();
		if(!swrContext)
			return 0;

		if(!in_lay)
			in_lay = AV_CH_FRONT_CENTER;

		// Get some information about the audio.
		info.src_ch_layout = in_lay;
		info.dst_ch_layout = out_lay;
		info.src_rate = in_rate;
		info.dst_rate = out_rate;
		info.src_nb_channels = av_get_channel_layout_nb_channels(info.src_ch_layout);
		info.dst_nb_channels = av_get_channel_layout_nb_channels(info.dst_ch_layout);
		//info.src_linesize = //done during decode/resample
		//info.dst_linesize = //done during decode/resample
		info.src_nb_samples = src_nb_samples;
		info.dst_nb_samples = 0;
		info.max_dst_nb_samples = 0;
		info.src_sample_fmt = in_fmt;
		info.dst_sample_fmt = out_fmt;

		// Set the resample options.
		av_opt_set_int(swrContext,			"in_channel_layout",	info.src_ch_layout, 0);
		av_opt_set_int(swrContext,			"in_sample_rate",		info.src_rate, 0);
		av_opt_set_sample_fmt(swrContext,	"in_sample_fmt",		info.src_sample_fmt, 0);
		
		av_opt_set_int(swrContext,			"out_channel_layout",	info.dst_ch_layout, 0);
		av_opt_set_int(swrContext,			"out_sample_rate",		info.dst_rate, 0);
		av_opt_set_sample_fmt(swrContext,	"out_sample_fmt",		info.dst_sample_fmt, 0);
		
		// Do we need to resample?
		if (info.src_ch_layout != info.dst_ch_layout || info.src_rate != info.dst_rate || info.src_sample_fmt != info.dst_sample_fmt)
			needsResample = true;
		else
			needsResample = false;
		
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

		/*

		// Set the resample options.
		av_opt_set_int(swrContext,			"in_channel_layout",	in_lay, 0);
		av_opt_set_int(swrContext,			"in_sample_rate",		in_rate, 0);
		av_opt_set_sample_fmt(swrContext,	"in_sample_fmt",		in_fmt, 0);
		
		av_opt_set_int(swrContext,			"out_channel_layout",	out_lay, 0);
		av_opt_set_int(swrContext,			"out_sample_rate",		out_rate, 0);
		av_opt_set_sample_fmt(swrContext,	"out_sample_fmt",		out_fmt, 0);

		// Do we need to resample?
		if (in_lay != out_lay || in_rate != out_rate || in_fmt != out_fmt)
			needsResample = true;
		else
			needsResample = false;

		// Initialise the resample context.
		int result = swr_init(swrContext);
		if(result < 0)
			return 0;

		// Allocate the resample destination buffer.
		if(needsResample){
			int dst_nb_channels = av_get_channel_layout_nb_channels(out_lay);

			/* compute the number of converted samples: buffering is avoided
			 * ensuring that the output buffer will contain at least all the
			 * converted input samples * /
			max_dst_nb_samples = dst_nb_samples = (int)av_rescale_rnd(src_nb_samples, out_rate, in_rate, AV_ROUND_UP);
			
			result = jr_av_samples_alloc_array_and_samples(&dstData, &dst_linesize, dst_nb_channels, dst_nb_samples, out_fmt, 0);
			//result = av_samples_alloc(dstData, &info.dst_linesize, info.dst_nb_channels, info.dst_nb_samples, info.dst_sample_fmt, 0);
			if (result < 0) {
				//*error = CRERROR_DECODER_RESMPL;
				//throw std::runtime_error("Decoder - Could not allocate resample destination samples.");
				return 0;
			}
		}*/

		return 1;
	}

	int Transcode(uint8_t **data){
		if(!swrContext)
			return 0;

		srcData = data;

		if(!needsResample)
			return 1;

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

		ls + VString(dstData[0], resampled);

		info.dst_bufsize = av_samples_get_buffer_size(&info.dst_linesize, info.dst_nb_channels, resampled, info.dst_sample_fmt, 1);

		return 1;
	}

	LString ls;

	int GetDataSize(){
		if(!swrContext)
			return 0;

		if(!needsResample)
			return 0;

		return info.dst_nb_samples;


	//	if(!frame)
	//		return 0;

	//	return info.dst_nb_samples;
	//	return frame->nb_samples;
	}

	uint8_t ** GetData(){
		if(!swrContext)
			return 0;

		if(!needsResample)
			return srcData;
		else
			return dstData;
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

	void Close(){
		if(swrContext){	
			swr_free(&swrContext);
			swrContext = 0;
		}
	}

	~MaticalsAudioTranscode(){
		Close();
	}

};

class MaticalsAudioSave{
	MString file;
	AVCodec *aud_codec;
	AVCodecContext *aud_codec_context;
	AVFormatContext *outctx;
	AVFrame *aud_frame;
	int aud_frame_counter;
	AVStream *video_st, *audio_st;

public:

	MaticalsAudioSave(){
		aud_codec = 0;
		aud_codec_context = 0;
	}

	int GetLay(){
		if(!aud_codec_context)
			return 0;

		return aud_codec_context->channel_layout;		
	}

	int GetRate(){
		if(!aud_codec_context)
			return 0;

		return aud_codec_context->sample_rate;
		//return aud_codec_context->bit_rate;
	}

	AVSampleFormat GetFmt(){
		if(!aud_codec_context)
			return AV_SAMPLE_FMT_NONE;

		return aud_codec_context->sample_fmt;
	}

	int Open(VString out_file, AVCodecID codec_id = AV_CODEC_ID_AAC, AVSampleFormat sample_fmt = AV_SAMPLE_FMT_FLTP){
		Close();

		file = out_file;

		aud_codec = avcodec_find_encoder(codec_id);
        avcodec_register(aud_codec);
 
        if(!aud_codec)
            return 0;
 
        aud_codec_context = avcodec_alloc_context3(aud_codec);
        if(!aud_codec_context)
            return 0;

		aud_codec_context->bit_rate = 44100;
        aud_codec_context->sample_rate = 22050; //select_sample_rate(aud_codec);
        aud_codec_context->sample_fmt = sample_fmt;
        aud_codec_context->channel_layout = AV_CH_LAYOUT_MONO;
        aud_codec_context->channels = av_get_channel_layout_nb_channels(aud_codec_context->channel_layout);
 
        aud_codec_context->codec = aud_codec;
        aud_codec_context->codec_id = codec_id;
 
        int ret = avcodec_open2(aud_codec_context, aud_codec, NULL);
		if (ret < 0)
			return 0;

		outctx = avformat_alloc_context();
        ret = avformat_alloc_output_context2(&outctx, NULL, "mp4", file);
 
        outctx->audio_codec = aud_codec;
        outctx->audio_codec_id = codec_id;
 
        audio_st = avformat_new_stream(outctx, aud_codec);
 
        audio_st->codecpar->bit_rate = aud_codec_context->bit_rate;
        audio_st->codecpar->sample_rate = aud_codec_context->sample_rate;
        audio_st->codecpar->channels = aud_codec_context->channels;
        audio_st->codecpar->channel_layout = aud_codec_context->channel_layout;
        audio_st->codecpar->codec_id = codec_id;
        audio_st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_st->codecpar->format = sample_fmt;
        audio_st->codecpar->frame_size = aud_codec_context->frame_size;
        audio_st->codecpar->block_align = aud_codec_context->block_align;
        audio_st->codecpar->initial_padding = aud_codec_context->initial_padding;
 
        outctx->streams = new AVStream*[1];
        outctx->streams[0] = audio_st;
 
        av_dump_format(outctx, 0, file, 1);
 
        if (!(outctx->oformat->flags & AVFMT_NOFILE))
        {
            if (avio_open(&outctx->pb, file, AVIO_FLAG_WRITE) < 0)
                return 0;
        }
 
        ret = avformat_write_header(outctx, NULL);
 
        aud_frame = av_frame_alloc();
        aud_frame->nb_samples = aud_codec_context->frame_size;
        aud_frame->format = aud_codec_context->sample_fmt;
        aud_frame->channel_layout = aud_codec_context->channel_layout;
 
        int buffer_size = av_samples_get_buffer_size(NULL, aud_codec_context->channels, aud_codec_context->frame_size,
            aud_codec_context->sample_fmt, 0);
 
        av_frame_get_buffer(aud_frame, buffer_size / aud_codec_context->channels);
 
        if (!aud_frame)
            return 0;
 
        aud_frame_counter = 0;

		// Buffers
		int src_samples_linesize;
        src_nb_samples = 1024 * 4;
        src_channels = aud_codec_context->channels;

		ret = av_samples_alloc_array_and_samples((uint8_t***)&aud_samples, &src_samples_linesize, src_channels,
            src_nb_samples, AV_SAMPLE_FMT_FLTP, 0);

		//t = 0;
		//tincr = 0;
		//tincr2 = 0;

		return 1;
	}

	float** aud_samples;
	float t, tincr, tincr2;
	int src_channels, src_nb_samples;




	static int select_sample_rate(AVCodec *codec){
        const int *p;
        int best_samplerate = 0;
 
        if (!codec->supported_samplerates)
            return 44100;
 
        p = codec->supported_samplerates;
        while (*p) {
            best_samplerate = FFMAX(*p, best_samplerate);
            p++;
 
        }
        return best_samplerate;
    }

	    int finish_audio_encoding(){
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
 
        fflush(stdout);
 
        int ret = avcodec_send_frame(aud_codec_context, NULL);
        if (ret < 0)
            return 0;
 
        while (true)
        {
            ret = avcodec_receive_packet(aud_codec_context, &pkt);
            if (!ret)
            {
                if (pkt.pts != AV_NOPTS_VALUE)
                    pkt.pts = av_rescale_q(pkt.pts, aud_codec_context->time_base, audio_st->time_base);
                if (pkt.dts != AV_NOPTS_VALUE)
                    pkt.dts = av_rescale_q(pkt.dts, aud_codec_context->time_base, audio_st->time_base);
 
                av_write_frame(outctx, &pkt);
                av_packet_unref(&pkt);
            }
            if (ret == -AVERROR(AVERROR_EOF))
                break;
            else if (ret < 0)
                return 0;
        }
 
        av_write_trailer(outctx);
		return 1;
    }

	int WriteTest(){
		get_audio_frame(aud_samples[0], aud_samples[1], src_nb_samples, &t, &tincr, &tincr2);

		encode_audio_samples((uint8_t **)aud_samples);

		//if(aud_frame_counter < 100)
		//	return 1;

		return 0;		
	}

	int Write(uint8_t ** data, int datasize){
		int buffer_size = av_samples_get_buffer_size(NULL, aud_codec_context->channels, aud_codec_context->frame_size,
            aud_codec_context->sample_fmt, 0);

		uint8_t *d[8] = {data[0], 0, 0, 0, 0, 0, 0, 0};

		while(datasize > 0){
			encode_audio_samples(d, min(datasize, buffer_size));
			datasize -= buffer_size;
			d[0] += buffer_size;
		}
		
		return 1;
	}

	int Write(VString data){
		unsigned char *d = data, *t = data.endu();

		int buffer_size = av_samples_get_buffer_size(NULL, aud_codec_context->channels, aud_codec_context->frame_size, aud_codec_context->sample_fmt, 0);
		buffer_size = 1024;

		while(d < t){
			uint8_t *dt[8] = {(uint8_t*)d, 0, 0, 0, 0, 0, 0, 0};
			encode_audio_samples(dt, min(t - d, buffer_size));

			d += buffer_size;
		}

		/*
		
			int len = min(t - d, src_nb_samples);

			memcpy(aud_samples[0], d, len);
			//memcpy(aud_samples[1], d, len);

			encode_audio_samples((uint8_t **)aud_samples);

			d += len;
		}*/

		return 1;
	}

	int Write(MaticalsAudioLink &al){
		unsigned char *d = al.data, *t = al.data.endu();

		while(d < t){
			int len = min(t - d, src_nb_samples);

			memcpy(aud_samples[0], d, len);
			memcpy(aud_samples[1], d, len);

			encode_audio_samples((uint8_t **)aud_samples);

			d += len;
		}

		return 1;
	}

	    int encode_audio_samples(uint8_t **aud_samples, int size = 0)
    {
        int ret;
 
        int buffer_size = av_samples_get_buffer_size(NULL, aud_codec_context->channels, aud_codec_context->frame_size,
            aud_codec_context->sample_fmt, 0);

		
		if(size){
			aud_frame->linesize[0] = size;
			buffer_size = size;
		}
 
        for (size_t i = 0; i < buffer_size / aud_codec_context->channels; i++)
        {
            aud_frame->data[0][i] = aud_samples[0][i];
            //aud_frame->data[1][i] = aud_samples[1][i];
        }
		//aud_frame->data[0] = aud_samples[0];
		//aud_frame->linesize[0] = size;

 
        aud_frame->pts = aud_frame_counter++;
 
        ret = avcodec_send_frame(aud_codec_context, aud_frame);
        if (ret < 0)
            return 0;
 
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
 
        fflush(stdout);
 
        while (true)
        {
            ret = avcodec_receive_packet(aud_codec_context, &pkt);
            if (!ret)
            {
                av_packet_rescale_ts(&pkt, aud_codec_context->time_base, audio_st->time_base);
 
                pkt.stream_index = audio_st->index;
                av_write_frame(outctx, &pkt);
                av_packet_unref(&pkt);
            }
            if (ret == AVERROR(EAGAIN))
                break;
            else if (ret < 0)
                return 0;
            else
                break;
        }
 
        return 0;
    }

	void get_audio_frame(float *left_samples, float *right_samples, int frame_size, float* t, float* tincr, float* tincr2){
		/* encode a single tone sound */
    static int once = 0;
	if(!once){
		once = 1;
		*t = 0;
		*tincr = 2 * M_PI * 440.0 / 44100;
	}

    for (int j = 0; j < frame_size; j++) {
		float v = sin(*t);
            *left_samples ++ = v;
			*right_samples ++ = v;

            *t += *tincr;
        }
        
		return ;
	}

		    void cleanup()
    {
        //if (vid_frame)
        //{
        //    av_frame_free(&vid_frame);
        //}
        if (aud_frame)
        {
            av_frame_free(&aud_frame);
        }
        if (outctx)
        {
            for (int i = 0; i < outctx->nb_streams; i++)
                av_freep(&outctx->streams[i]);
 
            avio_close(outctx->pb);
            av_free(outctx);
        }
 
        if (aud_codec_context)
        {
            avcodec_close(aud_codec_context);
            av_free(aud_codec_context);
        }
 
        //if (vid_codec_context)
        //{
        //    avcodec_close(vid_codec_context);
        //    av_free(vid_codec_context);
        //}
    }

	void Close(){

	}

};