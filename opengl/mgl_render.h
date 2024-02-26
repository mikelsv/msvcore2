#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

#define AV_TS_MAX_STRING_SIZE 32

#define av_ts2str(ts) av_ts_make_string(__buf, ts)
#define av_ts2timestr(ts, tb) av_ts_make_time_string(__buf, ts, tb)

char __buf[AV_TS_MAX_STRING_SIZE];

static inline char *av_ts_make_string(char *buf, int64_t ts)
{
    if (ts == AV_NOPTS_VALUE) _snprintf(buf, AV_TS_MAX_STRING_SIZE, "NOPTS");
    else                      _snprintf(buf, AV_TS_MAX_STRING_SIZE, "%", ts);
    return buf;
}

static inline char *av_ts_make_time_string(char *buf, int64_t ts, AVRational *tb)
{
    if (ts == AV_NOPTS_VALUE) _snprintf(buf, AV_TS_MAX_STRING_SIZE, "NOPTS");
    else                      _snprintf(buf, AV_TS_MAX_STRING_SIZE, "%.6g", av_q2d(*tb) * ts);
    return buf;
}

class FfmpegRegister{
public:
	FfmpegRegister(){
	    avcodec_register_all();
        av_register_all();
	}
} FfmpegRegister;


typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt){
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

	char buff[MAXCHAR];

    sprintf(buff, "pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);

	print(buff);
}

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    /* Write the compressed frame to the media file. */
    //log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

/* Add an output stream. */
static void add_stream(OutputStream *ost, AVFormatContext *oc,
                       const AVCodec **codec,
                       enum AVCodecID codec_id, int width = 352, int height = 288, int fps = 25)
{
    AVCodecContext *c;
    int i;

    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    ost->st->id = oc->nb_streams-1;
    c = avcodec_alloc_context3(*codec);
    if (!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc = c;

    switch ((*codec)->type){
		
	case AVMEDIA_TYPE_AUDIO:
		c->sample_fmt  = (*codec)->sample_fmts ?
            (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;

		//c->sample_fmt = AV_SAMPLE_FMT_S16;

		//c->bit_rate    = 44100;
		//c->sample_rate = 44100 / 2;
		c->bit_rate    = 64000;
        c->sample_rate = 44100;

		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		c->channel_layout = AV_CH_LAYOUT_MONO;
		ost->st->time_base.num = 1;
		ost->st->time_base.den = c->sample_rate;

		break;

		/*
		case AVMEDIA_TYPE_AUDIO:		
        c->sample_fmt  = (*codec)->sample_fmts ?
            (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        if ((*codec)->channel_layouts) {
            c->channel_layout = (*codec)->channel_layouts[0];
            for (i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        
		ost->st->time_base.num = 1;
		ost->st->time_base.den = c->sample_rate;
        break;
		*/


    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;

        c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width    = width;
        c->height   = height;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
		ost->st->time_base.num = 1;
		ost->st->time_base.den = fps;
        c->time_base       = ost->st->time_base;
		//c->sample_aspect_ratio = ost->st->time_base;
		
		c->framerate.num = fps;
		c->framerate.den = 1;

        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_PIX_FMT;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B-frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }

		//if(av_opt_set(codecx->priv_data, "preset", "slow", 0) < 0) {
		//	print("Could not set options: preset.\r\n");
		//	exit(1);
		//}

		if(av_opt_set(c->priv_data, "tune", "animation", 0) < 0) {
			print("Could not set options: tune.\r\n");
			//exit(1);
		}

		if(av_opt_set(c->priv_data, "crf", "17", 0) < 0) {
			print("Could not set options: crf.\r\n");
			//exit(1);
		}


    break;

    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

/**************************************************************/


static void close_stream(AVFormatContext *oc, OutputStream *ost){
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
//    swr_free(&ost->swr_ctx);
}


static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

static void open_video(AVFormatContext *oc, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;
    AVCodecContext *c = ost->enc;
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        fprintf(stderr, "Could not open video codec: %s\n", "");
        exit(1);
    }

    /* allocate and init a re-usable frame */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }
}

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;
    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }
    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;
    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }
    return frame;
}

static void open_audio(AVFormatContext *oc, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    AVCodecContext *c;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;
    c = ost->enc;
    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        fprintf(stderr, "Could not open audio codec: %d\n", ret);
        exit(1);
    }
    /* init signal generator */
    ost->t     = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;
    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;
    ost->frame     = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                                       c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
                                       c->sample_rate, nb_samples);
    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }
    /* create resampler context */
        ost->swr_ctx = swr_alloc();
        if (!ost->swr_ctx) {
            fprintf(stderr, "Could not allocate resampler context\n");
            exit(1);
        }
        /* set options */
        av_opt_set_int       (ost->swr_ctx, "in_channel_count",   c->channels,       0);
        av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     c->sample_rate,    0);
        av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
        av_opt_set_int       (ost->swr_ctx, "out_channel_count",  c->channels,       0);
        av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    c->sample_rate,    0);
        av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     c->sample_fmt,     0);
        /* initialize the resampling context */
        if ((ret = swr_init(ost->swr_ctx)) < 0) {
            fprintf(stderr, "Failed to initialize the resampling context\n");
            exit(1);
        }
}

class MaticalsAudioLink{
public:
	VString data;
	int rate, bps;
	int lay;

	MaticalsAudioLink(){
		//lay = AV_CH_LAYOUT_MONO;
	}

	MaticalsAudioLink(VString d, int l, int r, int b){
		data = d;
		lay = l;
		rate = r;
		bps = b;
	}

	void Set(VString d, int l, int r, int b){
		data = d;
		lay = l;
		rate = r;
		bps = b;
	}

};


class MaticalsVideoWrite{
	MString file;

	AVFormatContext *oc;
	const AVOutputFormat *fmt;
	OutputStream video_st, audio_st;
	int encode_video, encode_audio;
	int have_video, have_audio;
	const AVCodec *video_codec, *audio_codec;//, *video_codec;
	AVDictionary *opt;
	SwsContext *swsx;
	int ret;

public:
	MWndIPE ipe;

public:
	MaticalsVideoWrite(){
		oc = 0;
		fmt = 0;
		opt = 0;
		video_codec = 0;
		audio_codec = 0;
		swsx = 0;

		Close();
	}

	int MakeAudio(VString output_file, AVCodecID aud_codec_id, AVSampleFormat sample_fmt, int bit_rate){
		Close();

		file = output_file;


	}


	int Open(VString f, int width, int height, int fps){
		Close();

		file = f;

		print(LString() + "Rendering init... " + width + "x" + height + " " + fps + "fps. (" + file + ").\r\n");

		/* allocate the output media context */
		avformat_alloc_output_context2(&oc, NULL, NULL, file);
		if (!oc) {
			printf("Could not deduce output format from file extension: using MPEG.\n");
			avformat_alloc_output_context2(&oc, NULL, NULL, file);
		}
		if (!oc)
			return 1;

		fmt = oc->oformat;

		/* Add the audio and video streams using the default format codecs
		 * and initialize the codecs. */
		if (fmt->video_codec != AV_CODEC_ID_NONE){
			add_stream(&video_st, oc, &video_codec, fmt->video_codec, width, height, fps);
			have_video = 1;
			encode_video = 1;
		}

		if (fmt->audio_codec != AV_CODEC_ID_NONE) {
		    add_stream(&audio_st, oc, &audio_codec, fmt->audio_codec);
		    have_audio = 1;
		    encode_audio = 1;
		}

		/* Now that all the parameters are set, we can open the audio and
		 * video codecs and allocate the necessary encode buffers. */
		if (have_video)
			open_video(oc, video_codec, &video_st, opt);

	    if (have_audio)
	        open_audio(oc, audio_codec, &audio_st, opt);

		av_dump_format(oc, 0, file, 1);

		/* open the output file, if needed */
		if (!(fmt->flags & AVFMT_NOFILE)) {
			ret = avio_open(&oc->pb, file, AVIO_FLAG_WRITE);
			if (ret < 0) {
				fprintf(stderr, "Could not open '%s': %s\n", file,
						"");
				return 1;
			}
		}

		/* Write the stream header, if any. */
		ret = avformat_write_header(oc, &opt);
		if (ret < 0) {
			fprintf(stderr, "Error occurred when opening output file: %s\n",
					"");
			return 1;
		}

		// Scale context
		AVFrame *frame = video_st.frame;

		swsx = sws_getContext(frame->width, frame->height, AV_PIX_FMT_RGB24,
			frame->width, frame->height, video_st.enc->pix_fmt,
			SWS_POINT, NULL, NULL, NULL);


		// Alloc memory
		ipe.New(width, height);

		return 1;
	}

	int WriteFrame(unsigned char *data, unsigned int sz, int frame_id, int frames){
		int ret;
		AVCodecContext *c;
		AVFrame *frame;
		int got_packet = 0;
		AVPacket pkt = { 0 };

		OutputStream *ost = &video_st;
		frame = ost->frame;
		c = ost->enc;

		// Scale
		uint8_t *pdata[8] = {(uint8_t*)data + (frame->width * 3) * (frame->height - 1), 0, 0, 0, 0, 0, 0, 0};
		int linesize[8] = {- frame->width * 3, 0, 0, 0, 0, 0, 0, 0};

		sws_scale(swsx, pdata, linesize, 0, frame->height, frame->data, frame->linesize);

		ost->frame->pts = ost->next_pts++;

		//frame = get_video_frame(ost);

		av_init_packet(&pkt);

		int response = avcodec_send_frame(c, frame);

		while(response >= 0){
			response = avcodec_receive_packet(c, &pkt);
			if(response == AVERROR(EAGAIN) || response == AVERROR_EOF){
				break;
			} else if (response < 0){
				return -1;
			}

			ret = write_frame(oc, &c->time_base, ost->st, &pkt);
			if(ret < 0){
				print(LString() + "Error while writing video frame: " + frame_id + ".\r\n");
				exit(1);
			}
		}

		///* encode the image */
		//ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
		//if (ret < 0){
		//	print(LString() + "Error encoding video frame: " + frame_id + ".\r\n");
		//	exit(1);
		//}

		//if(got_packet){
		//	ret = write_frame(oc, &c->time_base, ost->st, &pkt);
		//} else {
		//	ret = 0;
		//}

		//if(ret < 0){
		//	print(LString() + "Error while writing video frame: " + frame_id + ".\r\n");
		//	exit(1);
		//}

		if(frame_id % (frames / 20) == 0){
			int pc = 100 * frame_id / frames;
			print(LString() + "Render.WriteFrame(): " + frame_id + "/" + frames + "(" + pc + "%).\r\n");
		}

		return (frame || got_packet) ? 0 : 1;
		//return write_video_frame(oc, &video_st);
	}

	int WriteAudioFrame(unsigned char *data, unsigned int sz, int frame_id){
		AVPacket pkt = { 0 };
		int got_packet = 0;
		AVCodecContext *c;
		AVFrame *frame;

		OutputStream *ost = &audio_st;
		frame = ost->frame;
		c = ost->enc;

		frame->data[0] = data;
		frame->data[1] = 0;
		frame->nb_samples = sz;

		ost->frame->pts = ost->next_pts++;

		av_init_packet(&pkt);
		int response = avcodec_send_frame(c, frame);
		
		while(response >= 0){
			response = avcodec_receive_packet(c, &pkt);
			if(response == AVERROR(EAGAIN) || response == AVERROR_EOF){
				break;
			} else if (response < 0){
				return -1;
			}

			ret = write_frame(oc, &c->time_base, ost->st, &pkt);
			if(ret < 0){
				print(LString() + "Error while writing audio frame: " + frame_id + ".\r\n");
				exit(1);
			}
		}

		return (frame || got_packet) ? 0 : 1;
	}

	int WriteTestAudioFrame(){
		AVPacket pkt = { 0 };
		int got_packet = 0;
		AVCodecContext *c;
		AVFrame *frame;

		OutputStream *ost = &audio_st;
		frame = ost->frame;
		c = ost->enc;

		// Generate
		static float t = 0, tincr =	2 * M_PI * 440.0 / c->sample_rate;

		av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;

		ost->frame->pts = ost->next_pts++;

		int ret = av_frame_make_writable(frame);
		uint16_t *samples = (uint16_t*)frame->data[0];

		for(int j = 0; j < c->frame_size; j++) {
            samples[2*j] = (int)(sin(t) * 10000);
            for(int k = 1; k < c->channels; k++)
                samples[2*j + k] = samples[2*j];
            t += tincr;
        }

		int response = avcodec_send_frame(c, frame);
		
		while(response >= 0){
			response = avcodec_receive_packet(c, &pkt);
			if(response == AVERROR(EAGAIN) || response == AVERROR_EOF){
				break;
			} else if (response < 0){
				return -1;
			}

			ret = write_frame(oc, &c->time_base, ost->st, &pkt);
			if(ret < 0){
				print(LString() + "Error while writing test audio frame: " + ost->frame->pts + ".\r\n");
				exit(1);
			}
		}
		
		return (frame || got_packet) ? 0 : 1;
	}

	int FinishAudio(){
		AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;

		if(!encode_audio)
			return 1;

		OutputStream *ost = &video_st;
		AVCodecContext *ctx = ost->enc;
		AVFrame *frame = audio_st.frame;

		int ret = avcodec_send_frame(ctx, NULL);
        if (ret < 0)
            return 0;
 
        while (true)
        {
            ret = avcodec_receive_packet(ctx, &pkt);
            if (!ret)
            {
                if (pkt.pts != AV_NOPTS_VALUE)
                    pkt.pts = av_rescale_q(pkt.pts, ctx->time_base, ctx->time_base);
                if (pkt.dts != AV_NOPTS_VALUE)
                    pkt.dts = av_rescale_q(pkt.dts, ctx->time_base, ctx->time_base);
 
                av_write_frame(oc, &pkt);
                av_packet_unref(&pkt);
            }
            if (ret == -AVERROR(AVERROR_EOF))
                break;
            else if (ret < 0)
                return 0;
        }

		return 1;
	}

	void Close(){
		if(oc){
			FinishAudio();

		    av_write_trailer(oc);

		/* Close each codec. */
			if(have_video)
				close_stream(oc, &video_st);
			if (have_audio)
				close_stream(oc, &audio_st);

			if (!(fmt->flags & AVFMT_NOFILE))
				/* Close the output file. */
				avio_closep(&oc->pb);

			/* free the stream */
			avformat_free_context(oc);
			oc = 0;
		}

		if(swsx){
			sws_freeContext(swsx);
			swsx = 0;
		}

		encode_video = 0;
		encode_audio = 0;

		have_video = 0;
	}

	~MaticalsVideoWrite(){
		Close();
	}

};

#pragma pack(push, 2)
struct WavHeaderGgl{
    unsigned long chunkID; //"RIFF" = 0x46464952
    unsigned long chunkSize; //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
    unsigned long format; //"WAVE" = 0x45564157
    unsigned long subchunk1ID; //"fmt " = 0x20746D66
    unsigned long subchunk1Size; //16 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes]
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned long sampleRate;
    unsigned long byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
	//WORD wExtraFormatBytes;
	unsigned long subchunk2ID;
	unsigned long subchunk2Size;
    //[WORD wExtraFormatBytes;]
    //[Extra format bytes]
};
#pragma pack(pop)

#include <sapi.h>
#include <sphelper.h>

class MaticalsAudioWrite{
	MString out_file, out_data;
	int length, bps;

public:
	MaticalsAudioWrite(){
		length = 0;
	}

	bool Open(VString file, int len){
		out_file = file;
		length = len;
		bps = 100;

		CSpStreamFormat		Fmt;
		Fmt.AssignFormat(SPSF_22kHz16BitMono);

		bps = Fmt.WaveFormatExPtr()->nAvgBytesPerSec;

		// Output
		out_data.Reserve(sizeof(WavHeaderGgl) + length * bps);
		if(!out_data)
			return 0;

		// Header
		WavHeaderGgl wh;
		wh.chunkID = 0x46464952; //"RIFF" = 0x46464952
		wh.chunkSize = 0; //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
		wh.format = 0x45564157; //"WAVE" = 0x45564157
		wh.subchunk1ID = 0x20746D66; //"fmt " = 0x20746D66
		wh.subchunk1Size = 16; //16 [+ sizeof(wExtr
		memcpy(&wh.audioFormat, Fmt.WaveFormatExPtr(), 18);
		//wh.wExtraFormatBytes = 0;
		wh.subchunk2ID = 0x61746164; //"data" = 0x61746164
		wh.subchunk2Size = length * bps;

		wh.chunkSize = sizeof(wh) - 8 + wh.subchunk2Size;		

		memcpy(out_data, &wh, sizeof(wh));

		return 1;
	}

	VString GetData(){
		return VString(out_data.data + sizeof(WavHeaderGgl), length * bps);
	}

	VString GetWavData(VString data){
		if(data.sz < sizeof(WavHeaderGgl))
			return VString();

		WavHeaderGgl *wh = (WavHeaderGgl*)data.data;

		if(wh->subchunk2ID == 0x61746164)
			return data.str(sizeof(WavHeaderGgl), wh->subchunk2Size);

		return VString();
	}

	bool SetWav(float time, VString wav){
		return Set(time, GetWavData(wav));
	}

	int Set(float time, VString data){
		if(!out_data)
			return 0;

		unsigned char *d = out_data.data;
		unsigned int pos = sizeof(WavHeaderGgl) + int(time * bps);

		pos += 4 - (pos % 4);

		memcpy(d + pos, data, min(data.sz, out_data - pos));

		return min(data.sz, out_data - pos);
	}

	bool Save(){
		if(!out_data)
			return 0;

		return SaveFile(out_file, out_data);
	}

};


#ifdef NOOO


class MaticalsVideoSave{
	MString file;

	AVFormatContext *formatx;
	AVStream *stream;
	AVCodec *codec;
	AVCodecContext *codecx;
	AVFrame *frame;

public:
	MWndIPE ipe;

	MaticalsVideoSave(){
		avdevice_register_all();

		formatx = 0;
		stream = 0;
		codec = 0;
		codecx = 0;
		frame = 0;
	}

	int Open(VString f, int width, int height, int fps){		
		Close();

		int ret;
		file = f;		

		avformat_alloc_output_context2(&formatx, NULL, NULL, file);
		if(!formatx){
		  fprintf(stderr, "Could not create output context.\r\n");
		  return 0;
		}

		// Codec
		codec = avcodec_find_encoder_by_name("libx264");
		AVCodec* c = avcodec_find_encoder(codec->id);
		if (!codec){
			print("Codec not found.\r\n");
			return 0;
		}

		stream = avformat_new_stream(formatx, codec);
		if(!stream){
			fprintf(stderr, "Failed allocating output stream.\r\n");
			return 0;
		}

		codecx = avcodec_alloc_context3(c);

		// Encoder codec params
		//stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		stream->codec->codec_id = codec->id;
		stream->codec->codec_type = codec->type;
		stream->codec->width = width;
		stream->codec->height = height;
		stream->codec->pix_fmt = AV_PIX_FMT_YUV422P;

		codecx->width = width;
		codecx->height = height;
		codecx->pix_fmt = AV_PIX_FMT_YUV422P;

		// control rate
		codecx->bit_rate = 2 * 1000 * 1000;
		codecx->rc_buffer_size = 4 * 1000 * 1000;
		codecx->rc_max_rate = 2 * 1000 * 1000;
		codecx->rc_min_rate = 2.5 * 1000 * 1000;
		
		// time base
		codecx->time_base.num = 1;
		codecx->time_base.den = fps;
		//codecx->time_base = av_inv_q(codecx->time_base);
		stream->time_base = codecx->time_base;
		stream->r_frame_rate.num = 1;
		stream->r_frame_rate.den = fps;

		codecx->gop_size = 10; /* emit one intra frame every ten frames */
		codecx->max_b_frames = 1;


		//if(av_opt_set(codecx->priv_data, "preset", "slow", 0) < 0) {
		//	print("Could not set options: preset.\r\n");
		//	exit(1);
		//}

		//if(av_opt_set(codecx->priv_data, "tune", "animation", 0) < 0) {
		//	print("Could not set options: tune.\r\n");
		//	exit(1);
		//}

		//if(av_opt_set(codecx->priv_data, "crf", "17", 0) < 0) {
		//	print("Could not set options: crf.\r\n");
		//	exit(1);
		//}

		if(avcodec_open2(codecx, codec, NULL) < 0) {
			print("Could not open codec.\r\n");
			exit(1);
		}

	//AVStream *o_video_stream = avformat_new_stream(o_fmt_ctx, NULL);
	//{
	//	AVCodecParameters *c = stream->codecpar;
	//	//c = o_video_stream->codec;
	//	c->bit_rate = 400000; //90000
	//	c->codec_id = AV_CODEC_ID_H264;
	//	c->codec_type = AVMEDIA_TYPE_VIDEO;
	//	stream->time_base.num = 1;
	//	stream->time_base.den = 25; //90000
	//	c->width = 1280;
	//	c->height = 720;
	//}

    //avio_open(&o_fmt_ctx->pb, "z.mp4", AVIO_FLAG_WRITE);
    //ret = avformat_write_header(o_fmt_ctx, NULL);
	//formatx->video_codec = codec;
	//formatx->video_codec_id = stream->codec->codec_id;

		if(!(formatx->oformat->flags & AVFMT_NOFILE)) {
			ret = avio_open(&formatx->pb, file, AVIO_FLAG_WRITE);
			if(ret < 0){
				print("Could not open output file '", file, "'.\r\n");	
				return 0;
			}
		}

		//AVFormatContext *ctx = 0;
		//ret = avformat_open_input(&ctx, "test_ok.mp4", NULL, NULL);

		avcodec_parameters_from_context(stream->codecpar, codecx);

		//avcodec_parameters_from_context(stream->codecpar, codecx);
		//st->time_base = sap->sdp_ctx->streams[i]->time_base;
		//stream->time_base = ctx->streams[0]->time_base;
		//ret = avcodec_parameters_copy(stream->codecpar, ctx->streams[0]->codecpar);

		//ret = av_bsf_init(formatx);

		// Headers
		ret = avformat_write_header(formatx, NULL);
		if(ret < 0) {
			print("Error occurred when opening output file.\r\n");
			return 0;
		}

		stream->time_base = codecx->time_base;

		// Frame
		frame = av_frame_alloc();
		if (!frame) {
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = codecx->pix_fmt;
		frame->width  = codecx->width;
		frame->height = codecx->height;

		/* the image can be allocated by any means and av_image_alloc() is
		 * just the most convenient way if av_malloc() is to be used */
		ret = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, codecx->pix_fmt, 32);
		if(ret < 0){
			fprintf(stderr, "Could not allocate raw picture buffer\n");
			exit(1);
		}

		// Alloc memory
		ipe.New(width, height);

		return 1;
	}

	int WriteFrame(unsigned char *data, unsigned int sz, int frame_id){
		AVPacket pkt;
		int got_output;

		if(!data)
			return 0;

		av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;

		frame->pts = frame_id;

		SwsContext *swsx = sws_getContext(frame->width, frame->height, AV_PIX_FMT_RGB24,
			frame->width, frame->height, codecx->pix_fmt,
			SWS_POINT, NULL, NULL, NULL);

		uint8_t *pdata[8] = {(uint8_t*)data + (frame->width * 3) * (frame->height - 1), 0, 0, 0, 0, 0, 0, 0};
		int linesize[8] = {- frame->width * 3, 0, 0, 0, 0, 0, 0, 0};

		sws_scale(swsx, pdata, linesize, 0, frame->height, frame->data, frame->linesize);

		int response = avcodec_send_frame(codecx, frame);

		while(response >= 0){
			response = avcodec_receive_packet(codecx, &pkt);
			if(response == AVERROR(EAGAIN) || response == AVERROR_EOF){
				break;
			} else if (response < 0){
				return -1;
			}

			static int pkt_id = 0;

				pkt.stream_index = pkt_id;
				pkt_id ++;

				pkt.pos = -1;
				pkt.stream_index=0;

				//pkt.duration = codecx->time_base.den / stream->time_base.num / stream->avg_frame_rate.num * stream->avg_frame_rate.den;
				//av_packet_rescale_ts(&pkt, stream->time_base, stream->time_base);
				response = av_interleaved_write_frame(formatx, &pkt);
		  }

		av_packet_unref(&pkt);		

		return 1;
	}


	void Close(){
		if(!formatx)
			return ;

		av_write_trailer(formatx);

		avio_close(formatx->pb);

		avformat_free_context(formatx);

		formatx = 0;
	}

	~MaticalsVideoSave(){
		Close();
	}

};


class VideoGrab2{
	MString file;	

	const AVCodec *nVidiaCodec, *codec;
	AVCodecContext *c;
	AVFrame *frame;
	//ffEnc * ffmpege;
	FILE * outFile;

public:
	MWndIPE ipe;

public:
	VideoGrab2(){
		//avcodec_register_all();

		//ffmpege = 0;

		nVidiaCodec = 0;
		codec = 0;
		outFile = 0;
	}

	int Open(VString f, int width, int height, int fps){
		int ret; 
		file = f;

		avdevice_register_all();

		nVidiaCodec = avcodec_find_encoder_by_name("libx264");
		if(!nVidiaCodec){
			return false;
		}

		/* find the mpeg1 video encoder */
		codec = avcodec_find_encoder(nVidiaCodec->id);
		if (!codec) {
			fprintf(stderr, "Codec not found\n");
			exit(1);
		}
		c = avcodec_alloc_context3(codec);
		if (!c) {
			fprintf(stderr, "Could not allocate video codec context\n");
			exit(1);
		}

		/* put sample parameters */
		c->bit_rate = 4 * 1000 * 1000; //400000;
		c->rc_buffer_size = 16 * 1000 * 1000;
		c->rc_max_rate = 16 * 1000 * 1000;
		//c->rc_min_rate = 2.5 * 1000 * 1000;

		/* resolution must be a multiple of two */
		c->width = width;
		c->height = height;
		/* frames per second */
		c->time_base.num = 1;//{1, 25};//AVRational(1, 25);
		c->time_base.den = fps;//{1, 25};//AVRational(1, 25);
		c->gop_size = 10; /* emit one intra frame every ten frames */
		c->max_b_frames = 1;
		c->pix_fmt = AV_PIX_FMT_YUV422P; //AV_PIX_FMT_YUV420P;
		
		//c->qmin = 80;
		//c->qmax = 80;

		
		if(av_opt_set(c->priv_data, "preset", "slow", 0) < 0) {
			print("Could not set options: preset.\r\n");
			exit(1);
		}

		if(av_opt_set(c->priv_data, "tune", "animation", 0) < 0) {
			print("Could not set options: tune.\r\n");
			exit(1);
		}

		if(av_opt_set(c->priv_data, "crf", "17", 0) < 0) {
			print("Could not set options: crf.\r\n");
			exit(1);
		}

		//if (nVidiaCodec->id == AV_CODEC_ID_H264)
		//	av_opt_set(c->priv_data, "preset", "slow", 0);

		//if (nVidiaCodec->id == AV_CODEC_ID_H264)
		//if(ret = av_opt_set(c->priv_data, "quality", "realtime", 0)){
		//	print("Could not set options: quality.\r\n");
		//}

		/* open it */
		if (avcodec_open2(c, codec, NULL) < 0) {
			fprintf(stderr, "Could not open codec\n");
			exit(1);
		}

		outFile = fopen(file, "wb");
		if (!outFile) {
			fprintf(stderr, "Could not open %s\n", file);
			exit(1);
		}

		frame = av_frame_alloc();
		if (!frame) {
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = c->pix_fmt;
		frame->width  = c->width;
		frame->height = c->height;

		/* the image can be allocated by any means and av_image_alloc() is
		 * just the most convenient way if av_malloc() is to be used */
		ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
		if (ret < 0) {
			fprintf(stderr, "Could not allocate raw picture buffer\n");
			exit(1);
		}

		//frame->data[0] = frame->data[0] + frame->linesize[0] * (frame->height - 1);
		//frame->data[1] = frame->data[2] + frame->linesize[1] * (frame->height - 1);
		//frame->data[2] = frame->data[2] + frame->linesize[2] * (frame->height - 1);
		//frame->linesize[0] *= -1;
		//frame->linesize[1] *= -1;
		//frame->linesize[2] *= -1;

		// Alloc memory
		ipe.New(width, height);

		return ret;
	}

	// https://www.programmersought.com/article/58352347721/
#define RGB2Y(R, G, B)  ( 16  + 0.183f * (R) + 0.614f * (G) + 0.062f * (B) )
#define RGB2U(R, G, B)  ( 128 - 0.101f * (R) - 0.339f * (G) + 0.439f * (B) )
#define RGB2V(R, G, B)  ( 128 + 0.439f * (R) - 0.399f * (G) - 0.040f * (B) )

#define CLIPVALUE(x, minValue, maxValue) ((x) < (minValue) ? (minValue) : ((x) > (maxValue) ? (maxValue) : (x)))

	int WriteFrame(unsigned char *data, unsigned int sz, int frame_id){
		AVPacket pkt;
		int got_output;

		if(!data)
			return 0;

		av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;

		frame->pts = frame_id;

		/*
		//memcpy(frame->data[0], data, frame->width * frame->height * 3);
		for(int h = 0; h < frame->height; h ++){
			unsigned char *p = data + (frame->height - h - 1) * frame->width * 3;
			for(int w = 0; w < frame->width; w ++){			
				//unsigned char *lp = data + (w + (frame->height - h - 1) * frame->width) * 3;

				unsigned char r = p[0];
				unsigned char g = p[1];
				unsigned char b = p[2];

				frame->data[0][w + h * frame->width] = (unsigned char)CLIPVALUE(RGB2Y(r, g, b), 0, 255);
				//frame->data[1][(w + h * frame->width)] = (unsigned char)CLIPVALUE(RGB2U(r, g, b), 0, 255);// << w % 2 == 0 ? 0 : 4;
				//frame->data[2][(w + h * frame->width) / 2] = (unsigned char)CLIPVALUE(RGB2V(r, g, b), 0, 255) << w % 2 == 0 ? 0 : 4;
				//frame->data[2][w + h * frame->width] = data[(w + h * frame->width) * 3 + 2];

				if(w % 2 == 0){
					int num = (w + h * frame->width) / 2;
					//int offset = num / frame->width * (frame->linesize[0] - frame->width);

					frame->data[1][num] = (unsigned char)CLIPVALUE(RGB2U(r, g, b), 0, 255);
					frame->data[2][num] = (unsigned char)CLIPVALUE(RGB2V(r, g, b), 0, 255);
				}

				p += 3;
			}
		}*/

		SwsContext *swsx = sws_getContext(frame->width, frame->height, AV_PIX_FMT_RGB24,
			frame->width, frame->height, c->pix_fmt,
			SWS_POINT, NULL, NULL, NULL);

		int linesize[8] = {-frame->width * 3, 0, 0, 0, 0, 0, 0, 0};
		uint8_t *pdata[8] = {(uint8_t*)data + linesize[0] * (frame->height - 1) * -1, 0, 0, 0, 0, 0, 0, 0};
	
		sws_scale(swsx, pdata, linesize, 0, frame->height, frame->data, frame->linesize);

        /* encode the image */
		//frame->pts = av_rescale_q(frame->pts, c->time_base, c->time_base);

		//av_interleaved_write_uncoded_frame(

        int ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
        if(ret < 0){
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if(got_output){
            print(LString() + "Write frame " + frame_id + "%3d (size= " + pkt.size + ")\r\n");
            
			//ret = av_interleaved_write_frame(this->c, &pkt);
			
			fwrite(pkt.data, 1, pkt.size, outFile);
            av_packet_unref(&pkt);
        }

		return 1;
	}

	int Close(){
		if(!outFile)
			return 0;

		uint8_t endcode[] = { 0, 0, 1, 0xb7 };
		int got_output, ret;

		AVPacket pkt;
		av_init_packet(&pkt);

		do {
			fflush(stdout);
			ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
			if (ret < 0) {
				fprintf(stderr, "Error encoding frame\n");
				exit(1);
			}
			if (got_output) {
				fwrite(pkt.data, 1, pkt.size, outFile);
				av_packet_unref(&pkt);
			}
		} while (got_output);

		/* add sequence end code to have a real mpeg file */
		fwrite(endcode, 1, sizeof(endcode), outFile);
		fclose(outFile);
		avcodec_close(c);
		av_free(c);
		av_freep(&frame->data[0]);
		av_frame_free(&frame);

		outFile = 0;

		return 1;
	}

	~VideoGrab2(){
		Close();
	}
};

struct DecoderStuff{
    AVFormatContext *formatx;
    int nstream;
    AVCodec *codec;
    AVStream *stream;
    AVCodecContext *codecx;
    AVFrame *rawframe;
    AVFrame *rgbframe;
    SwsContext *swsx;
};


class MaticalsVideoDecoder{
	MString file;
	DecoderStuff d;
	GLuint texture;

public:

	MaticalsVideoDecoder(){
		texture = 0;



	}

	bool Is(){
		return texture > 0;
	}

	int Load(VString f){
		file = f;

		avdevice_register_all();

		avformat_open_input(&d.formatx, file, NULL, NULL);
		avformat_find_stream_info(d.formatx, NULL);
		d.nstream = av_find_best_stream(d.formatx, AVMEDIA_TYPE_VIDEO, -1, -1, &d.codec, 0);
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
		dump_timebase("in stream", d.stream);
		dump_timebase("in stream:codec", d.stream->codec); // note: deprecated
		dump_timebase("in codec", d.codecx);
	
		return 1;
	}

	void SetTexture(GLuint tex){
		texture = tex;
	}

	GLuint GetTexture(){
		return texture;
	}

	bool ReadFrame(){
		//read_frame (DecoderStuff &d){

		AVPacket packet;
		int err = 0, haveframe = 0;

		// read
		while (!haveframe && err >= 0 && ((err = av_read_frame(d.formatx, &packet)) >= 0)){
		   if (packet.stream_index == d.nstream){
			   err = avcodec_decode_video2(d.codecx, d.rawframe, &haveframe, &packet);
		   }
		   av_packet_unref(&packet);
		}

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
		} else
			Seek(0);

		return haveframe;
	}

	bool Seek(int frameIndex){
		if(!d.formatx)
			return false;
		
		int timBase = (int64_t(d.codecx->time_base.num) * AV_TIME_BASE) / int64_t(d.codecx->time_base.den);
		int64_t seekTarget = int64_t(frameIndex) * timBase;		
		
		if(av_seek_frame(d.formatx, -1, seekTarget, AVSEEK_FLAG_ANY) < 0)
			return 0;
			//mexErrMsgTxt("av_seek_frame failed.");

		return 1;
	}

	int GetWidth(){
		return d.rawframe->width;
	}

	int GetHeight(){
		return d.rawframe->height;
	}

	unsigned char* GetData(){
		return d.rgbframe->data[0];
	}

	template <typename T>
static void dump_timebase (const char *what, const T *o) {
    if (o)
        printf("%s timebase: %d/%d\n", what, o->time_base.num, o->time_base.den);
    else
        printf("%s timebase: null object\n", what);
}

	void Close(){
		if(texture){
			glDeleteTextures(1, &texture);
			texture = 0;
		}

		avformat_close_input(&d.formatx);
		avcodec_close(d.codecx);
		av_free(d.codecx);
	}

	~MaticalsVideoDecoder(){
		Close();
	}

};

void initmp4(){
		AVCodec * c;
		AVFormatContext *mp4FmtCtx;
		AVOutputFormat *mp4OutFmt;
		AVStream *mp4Stream;

		avformat_alloc_output_context2(&mp4FmtCtx, NULL, "mpeg", "my.mp4");
		mp4OutFmt = mp4FmtCtx->oformat;
        c = avcodec_find_encoder(AV_CODEC_ID_H265);
        if (!c){
            printf("Can not find encoder! \n");
            exit(4);
        }
		mp4Stream = avformat_new_stream(mp4FmtCtx, c);
		if(mp4Stream == NULL){
			printf("avformat_new_stream fail\n");
			exit(3);
		}
		mp4Stream->time_base = av_make_q(1,25);
		mp4Stream->codecpar->codec_id = AV_CODEC_ID_H265;
		mp4Stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		//mp4Stream->codecpar->pix_fmt = AV_PIX_FMT_YUV422P;
		mp4Stream->codecpar->width = 720;
		mp4Stream->codecpar->height = 720;
        //mp4Stream->codecpar->time_base = av_make_q(1,25);
		//mp4Stream->codecpar->gop_size = 0 ;
		//mp4Stream->codecpar->max_b_frames = -1;
		//mp4Stream->codecpar->qmin = 15;
		//mp4Stream->codecpar->qmax = 35;
		if(mp4Stream->codec->codec_id == AV_CODEC_ID_H264){
				printf("set priv_data\n");
	   	    av_opt_set(mp4Stream->codec->priv_data, "preset", "slow", 0);
		}
printf("dump inof:\n");
        av_dump_format(mp4FmtCtx, 0, "my.pm4", 1);

        if (avcodec_open2(mp4Stream->codec, c, NULL) < 0){
            printf("Failed to open encoder! \n");
            exit(5);
         }  

		if(avio_open(&mp4FmtCtx->pb, "my.mp4", AVIO_FLAG_READ_WRITE) < 0){
			printf("avio_open my.pm4 fail\n");
			exit(3);
		}
        if(avformat_write_header(mp4FmtCtx, NULL)<0){
			printf("avformat_write_header fail\n");
			exit(3);
		}
        //video_enc_ctx = mp4Stream->codec;        
}


/*
 * Video encoding example
 */
static void video_encode_example(const char *filename, int codec_id){
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret, x, y, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    printf("Encode video file %s\n", filename);



	const AVCodec *nVidiaCodec = avcodec_find_encoder_by_name("h264_nvenc");
	
    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder(nVidiaCodec->id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* frames per second */
	c->time_base.num = 1;//{1, 25};//AVRational(1, 25);
	c->time_base.den = 25;//{1, 25};//AVRational(1, 25);
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;
    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                         c->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }
    /* encode 1 second of video */
    for (i = 0; i < 25; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;
        fflush(stdout);
        /* prepare a dummy image */
        /* Y * /
        for (y = 0; y < c->height; y++) {
            for (x = 0; x < c->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }
        /* Cb and Cr */
        for (y = 0; y < c->height/2; y++) {
            for (x = 0; x < c->width/2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }
        frame->pts = i;
        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_packet_unref(&pkt);
        }
    }
    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        fflush(stdout);
        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_packet_unref(&pkt);
        }
    }
    /* add sequence end code to have a real mpeg file */
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);
    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    printf("\n");

	//*/
}



// DELETE //

#include <vfw.h>
#pragma comment (lib, "vfw32")

// https://gamedev.ru/code/articles/Avi_Record_Windows
class VideoGrab{
	AVICOMPRESSOPTIONS opt;
	PAVISTREAM pStreamVideo;
	PAVISTREAM pCompressedStreamVideo;
	PAVIFILE pAviFile;	
	MString file;	

public:
	int Init(HWND hWnd){
		int res = ChooseCodec(hWnd, &opt);
		return res;
	}

	bool ChooseCodec(HWND hWnd,  AVICOMPRESSOPTIONS *pOptionsVideo){
		COMPVARS cv; //Настройки кодека
		ZeroMemory(&cv, sizeof(cv));
		cv.cbSize = sizeof(cv);

		//Выбор кодека
		if(ICCompressorChoose(hWnd, ICMF_CHOOSE_DATARATE |         
		   ICMF_CHOOSE_KEYFRAME, NULL, NULL, &cv, ("Выбор и настройка кодека")))
		{
		   ZeroMemory(pOptionsVideo, sizeof(AVICOMPRESSOPTIONS));

		   //Заполняем структуру AVICOMPRESSOPTIONS
		   pOptionsVideo->fccType = streamtypeVIDEO; //тип потока.
		   pOptionsVideo->fccHandler = cv.fccHandler; //четырёхсимвольный код кодека. 

		   //Если были выбраны ключевые кадры,
		   //то заполняем данное поле и устанавливаем соответствующий флаг.
		   if(cv.lKey)
		   {
			  pOptionsVideo->dwKeyFrameEvery = cv.lKey;
			  pOptionsVideo->dwFlags |= AVICOMPRESSF_KEYFRAMES;
		   }

		   //Заполняем поле качества потока.
		   pOptionsVideo->dwQuality  = cv.lQ;

		   //Если был выбран битрейт, то заполняем это поле и устанавливаем флаг.
		   if(cv.lDataRate)
		   {  
			   pOptionsVideo->dwFlags |= AVICOMPRESSF_DATARATE;
			   pOptionsVideo->dwBytesPerSecond  = cv.lDataRate * 1024;
		   }

		   //Незабываем освободить ресурсы полученные через ICCompressorChoose.
		   ICCompressorFree(&cv);
		   return true;
	   }
	   return false;
	}

	bool Open(VString f){
		AVIFileInit();
		file = f;

		return (AVIERR_OK == AVIFileOpen(&pAviFile, file, OF_WRITE | OF_CREATE, NULL) );
	}

	bool CreateVideoStream(DWORD dwFrameRate, size_t WidthFrame, size_t HeightFrame){
		PAVISTREAM *ppStreamVideo = &pStreamVideo;

		//Объявляем и обнуляем структуру AVISTREAMINFO.
		AVISTREAMINFO sStreamInfo;
		ZeroMemory(&sStreamInfo, sizeof(sStreamInfo));

		//Заполняем поля: тип потока, масштаб, количество кадров в секунду и
		//предполагаемый размер буфера кадра в байтах.
		//Если размер буфера заранее не знаем, то можно в это поле занести ноль.
		sStreamInfo.fccType = streamtypeVIDEO;
		sStreamInfo.dwScale = 1;
		sStreamInfo.dwRate = dwFrameRate;
		sStreamInfo.dwSuggestedBufferSize = 0;

		//Заполняем структуру sStreamInfo.rcFrame размерами кадра в пикселях.
		SetRect(&sStreamInfo.rcFrame, 0, 0, WidthFrame, HeightFrame);
 
		//Создаём видео поток.
		return (AVIERR_OK == AVIFileCreateStream(pAviFile, ppStreamVideo, &sStreamInfo));
	}

	bool MakeCompressedVideoStream(size_t WidthFrame, size_t HeightFrame){
        //AVICOMPRESSOPTIONS *pVideoOptions, PAVISTREAM pStreamVideo,
        //PAVISTREAM *ppCompressedStreamVideo
		AVICOMPRESSOPTIONS *pVideoOptions = &opt;
		PAVISTREAM *ppCompressedStreamVideo = &pCompressedStreamVideo;

		//Заполняем структуру  BITMAPINFO
		BITMAPINFO bi;
		ZeroMemory(&bi, sizeof(BITMAPINFO));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); //размер структуры
		bi.bmiHeader.biWidth = WidthFrame; //ширина кадра в пикселях.
		bi.bmiHeader.biHeight = HeightFrame; //высота кадра в пикселях.
		//размер кадра в байтах:
		bi.bmiHeader.biSizeImage = WidthFrame * HeightFrame * 3;
		bi.bmiHeader.biPlanes = 1; //количество цветовых плоскостей, у нас 1.
		bi.bmiHeader.biBitCount = 24; //количество бит на пиксель.
		bi.bmiHeader.biCompression = BI_RGB; //формат, у нас RGB.

		// Вычисляем указатель на массив пикселей, 
		// нужно для определения размера формата.
		void *pBits = ((LPBYTE)&bi) + bi.bmiHeader.biSize + bi.bmiHeader.biClrUsed * sizeof(RGBQUAD);

		//Пытаемся создать поток с компрессией.
		if(AVIERR_OK==AVIMakeCompressedStream(ppCompressedStreamVideo, pStreamVideo, pVideoOptions, NULL) ){

			//Устанавливаем формат.
			if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi,
					((LPBYTE)pBits) - ((LPBYTE) &bi)))
					return true;
      
			//В случае неуспеха удаляем поток.
			AVIStreamRelease(*ppCompressedStreamVideo);
			pCompressedStreamVideo = 0;
		}

		//Если предыдущий вызов AVIStreamSetFormat завершился неудачей,
		//то пытаемся установить другой формат. 16 бит на пиксель.
		bi.bmiHeader.biBitCount = 16;
		if(AVIERR_OK==AVIMakeCompressedStream(ppCompressedStreamVideo,
			pStreamVideo, pVideoOptions, NULL) )
		{

			//Устанавливаем формат.
			if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi,
					((LPBYTE)pBits) - ((LPBYTE) &bi)))
					return true;
      
			//В случае неуспеха удаляем поток.
			AVIStreamRelease(*ppCompressedStreamVideo);
			pCompressedStreamVideo = 0;
		}

		//Если предыдущий вызов AVIStreamSetFormat завершился неудачей,
		//то пытаемся установить другой формат. 8 бит на пиксель.
		bi.bmiHeader.biBitCount = 8;
		if(AVIERR_OK==AVIMakeCompressedStream(ppCompressedStreamVideo,
			pStreamVideo, pVideoOptions, NULL) )
		{

			//Устанавливаем формат.
			if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi,
					((LPBYTE)pBits) - ((LPBYTE) &bi)))
					return true;
      
			//В случае неуспеха удаляем поток.
			AVIStreamRelease(*ppCompressedStreamVideo);
			pCompressedStreamVideo = 0;
		}

		return false;
	}

	bool WriteFrame(unsigned char* data, int size, DWORD dwFrameNum){
		if(!data)
			return 0;

		unsigned int res = AVIStreamWrite(pCompressedStreamVideo, dwFrameNum, 1, data, size, 0, NULL, NULL);
		
		//res = MAKE_AVIERR(res);
		
		return (AVIERR_OK == res);
	}

	bool WriteFrame(BITMAPINFO *pBmp, DWORD dwFrameNum){
		return WriteFrameVideoCompress(pStreamVideo, pBmp, dwFrameNum);
	}

	bool WriteFrameVideoCompress(PAVISTREAM pCompressedStreamVideo, BITMAPINFO *pBmp, DWORD dwFrameNum){
	  return ((AVIERR_OK==AVIStreamWrite(pCompressedStreamVideo, dwFrameNum, 1,
	   ((LPBYTE)pBmp) + pBmp->bmiHeader.biSize + pBmp->bmiHeader.biClrUsed*sizeof(RGBQUAD),
	   pBmp->bmiHeader.biSizeImage, 0, NULL, NULL)));
	}

	bool AviClean(){
	  //Освобождаем видео поток.
	  if (pStreamVideo){
		AVIStreamRelease(pStreamVideo);
		pStreamVideo = 0;
	  }
  
	  //Освобождаем видео поток с компрессией.
	  if (pCompressedStreamVideo){
		 AVIStreamRelease (pCompressedStreamVideo);
		 pCompressedStreamVideo = 0;
	  }
 /*  
	  //Освобождаем аудио поток.
	  if (pStreamAudio)
	  {
		 AVIStreamRelease (pStreamAudio);
		 pStreamAudio = 0;
	  }*/
 
	  //Освобождаем интерфейс Avi файла.
	  if(pAviFile){
		 AVIFileRelease(pAviFile);
		 pAviFile = 0;
	  }

	  //Освобождаем AviFile библиотеку.
	  AVIFileExit();

	  return 1;
	}

	~VideoGrab(){
		AviClean();
	}

};


#endif