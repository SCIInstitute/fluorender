/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * This class sets up all ffmpeg settings and writes frames for various video
 * containers with the YUV420 codec. You must open and close each file, checking
 * that all functions, including writing a frame, returns true for success.
 *
 * @author Brig Bagley
 * @version 1 October 2014
 * @copyright SCI Institute, Univ. of Utah 2014
 */

//FYI: FFmpeg is a pure C project using C99 math features, 
//in order to enable C++ to use them you have to append -D__STDC_CONSTANT_MACROS to your CXXFLAGS

#include "QVideoEncoder.h"

QVideoEncoder::QVideoEncoder() {
    /* Initialize libavcodec, and register all codecs and formats. */
    ffmpeg::av_register_all();
	output_stream_.frame = 0;
	output_stream_.next_pts = 0;
	output_stream_.samples_count = 0;
	output_stream_.st = 0;
	output_stream_.swr_ctx = 0;
	output_stream_.sws_ctx = 0;
	output_stream_.t = 0;
	output_stream_.tincr = 0;
	output_stream_.tincr2 = 0;
	output_stream_.tmp_frame = 0;
	format_ = NULL;
	format_context_ = NULL;
	video_codec_ = NULL;
    filename_ = "";
	width_ = 0;
	height_ = 0;
	actual_width_ = 0;
	actual_height_ = 0;
	fps_ = 0;
	bitrate_ = 0;
	gop_ = 0;
	valid_ = false;
}

bool QVideoEncoder::open(std::string f, size_t w, size_t h, 
	size_t fps, size_t bitrate) {
	output_stream_.frame = 0;
	output_stream_.next_pts = 0;
	output_stream_.samples_count = 0;
	output_stream_.st = 0;
	output_stream_.swr_ctx = 0;
	output_stream_.sws_ctx = 0;
	output_stream_.t = 0;
	output_stream_.tincr = 0;
	output_stream_.tincr2 = 0;
	output_stream_.tmp_frame = 0;
    filename_ = f;
	// make sure width and height are divisible by 16
	actual_width_ = w;
	actual_height_ = h;
	width_ = ((size_t)(w/16))*16;
	height_ = ((size_t)(h/16))*16;
	fps_ = fps;
	bitrate_ = bitrate;
	gop_ = 12;
    ffmpeg::avformat_alloc_output_context2(
		&format_context_, NULL, NULL, filename_.c_str());
    if (!format_context_) {
        std::cerr << "Could not deduce output" <<
			"format from file extension: using MPEG.\n";
        ffmpeg::avformat_alloc_output_context2(
			&format_context_, NULL, "mpeg", filename_.c_str());
    }
    if (!format_context_) return false;
    format_ = format_context_->oformat;
    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if (format_->video_codec != ffmpeg::AV_CODEC_ID_NONE) 
        if (!add_stream()) return false;
    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    if(!open_video()) return false;
	//ffmpeg::av_dump_format(format_context_, 0, filename_.c_str(), 1);
    /* open the output file, if needed */
    if (!(format_->flags & AVFMT_NOFILE)) {
		int ret = ffmpeg::avio_open(&format_context_->pb, 
			filename_.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
			fprintf(stderr, "Could not open '%s': %d\n", filename_.c_str(),ret);
            return false;
        }
    }
    /* Write the stream header, if any. */
    int ret = ffmpeg::avformat_write_header(format_context_, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file: %d\n",ret);
        return false;
    }
	valid_ = true;
	return true;
}

QVideoEncoder::~QVideoEncoder(){

}

bool QVideoEncoder::add_stream(){
	
    ffmpeg::AVCodecContext *c;
    /* find the encoder */
    video_codec_ = ffmpeg::avcodec_find_encoder(format_->video_codec);
    if (!video_codec_) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                ffmpeg::avcodec_get_name(format_->video_codec));
        return false;
    }
    output_stream_.st = ffmpeg::avformat_new_stream(format_context_, video_codec_);
    if (!output_stream_.st) {
        fprintf(stderr, "Could not allocate stream\n");
        return false;
    }
    output_stream_.st->id = format_context_->nb_streams-1;
    c = output_stream_.st->codec;
    c->codec_id = format_->video_codec;
    c->bit_rate = bitrate_;
    /* Resolution must be a multiple of two. */
    c->width    = width_;
    c->height   = height_;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
        * of which frame timestamps are represented. For fixed-fps content,
        * timebase should be 1/framerate and timestamp increments should be
        * identical to 1. */
    output_stream_.st->time_base = ffmpeg::av_make_q( 1, fps_ );
    c->time_base     = output_stream_.st->time_base;
    c->gop_size      = gop_; /* emit one intra frame every twelve frames at most */
    c->pix_fmt       = STREAM_PIX_FMT;
    if (c->codec_id == ffmpeg::AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == ffmpeg::AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
            * This does not happen with normal video, it just happens here as
            * the motion of the chroma plane does not match the luma plane. */
        c->mb_decision = 2;
    }
    /* Some formats want stream headers to be separate. */
    if (format_context_->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	return true;
}

bool QVideoEncoder::open_video() {

	//open_video(oc, video_codec, &video_st, opt);
	//static void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
    int ret;
    ffmpeg::AVCodecContext *c = output_stream_.st->codec;
    /* open the codec */
    ret = ffmpeg::avcodec_open2(c, video_codec_, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open video codec: %d\n", ret);
        return false;
    }
    /* allocate and init a re-usable frame */
    output_stream_.frame = alloc_picture();
    if (!output_stream_.frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return false;
    }
    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    output_stream_.tmp_frame = NULL;
    if (c->pix_fmt != ffmpeg::AV_PIX_FMT_YUV420P) {
        output_stream_.tmp_frame = alloc_picture();
        if (!output_stream_.tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            return false;
        }
    }
	return true;
}

ffmpeg::AVFrame * QVideoEncoder::alloc_picture() {
	ffmpeg::AVFrame *picture;
    int ret;
    picture = ffmpeg::av_frame_alloc();
    if (!picture)
        return NULL;
    picture->format = ffmpeg::AV_PIX_FMT_YUV420P;
    picture->width  = width_;
    picture->height = height_;
    /* allocate the buffers for the frame data */
    ret = ffmpeg::av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        return NULL;
    }
    return picture;
}

void QVideoEncoder::close() {
	if (!valid_) return;
	//flush the remaining (delayed) frames.
	ffmpeg::AVCodecContext *c;
	c = output_stream_.st->codec;
	int last_dts = 0;
	while(true) {
		int got_packet = 0, ret;
		ffmpeg::AVFrame * frame = output_stream_.frame;
        ffmpeg::AVPacket pkt = { 0 };
        ffmpeg::av_init_packet(&pkt);
        /* encode the image */
		ret = ffmpeg::avcodec_encode_video2(c, &pkt, frame, &got_packet);
		if (last_dts == pkt.pts) break;
		last_dts = pkt.dts;
		if (ret < 0) {
			fprintf(stderr, "Error encoding video frame: %d\n", ret);
			break;
		}
		if(got_packet) 
			ret = write_frame(&c->time_base, &pkt);
	}
	//ffmpeg::avcodec_flush_buffers(c);
    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
	ffmpeg::av_write_trailer(format_context_);
    /* Close each codec. */
	ffmpeg::avcodec_close(output_stream_.st->codec);
    ffmpeg::av_frame_free(&output_stream_.frame);
    ffmpeg::av_frame_free(&output_stream_.tmp_frame);
    ffmpeg::sws_freeContext(output_stream_.sws_ctx);
    ffmpeg::swr_free(&output_stream_.swr_ctx);
	if (!(format_->flags & AVFMT_NOFILE))
        /* Close the output file. */
		ffmpeg::avio_close(format_context_->pb);
    /* free the stream */
    avformat_free_context(format_context_);
	valid_ = false;
}

void QVideoEncoder::log_packet(const ffmpeg::AVFormatContext *fmt_ctx, 
	const ffmpeg::AVPacket *pkt) {
    ffmpeg::AVRational *time_base = 
		&fmt_ctx->streams[pkt->stream_index]->time_base;
    std::cout << "pts: " << pkt->pts <<
				 "\npts_time: " << (pkt->pts * 
				 (double)time_base->num / time_base->den) <<
				 "\ndts: " << pkt->dts <<
				 "\ndts_time: " << (pkt->dts * 
				 (double)time_base->num / time_base->den) <<
				 "\nduration: " << pkt->duration <<
				 "\nduration_time: " << (pkt->duration * 
				 (double)time_base->num / time_base->den) <<
				 "\nstream_index: " << pkt->stream_index << std::endl;
}

bool QVideoEncoder::write_video_frame(size_t frame_num) {
    int ret;
    ffmpeg::AVCodecContext *c;
    int got_packet = 0;
	c = output_stream_.st->codec;
	output_stream_.frame->pts = frame_num;
	output_stream_.next_pts = frame_num+1;
	ffmpeg::AVFrame * frame = output_stream_.frame;
    if (format_context_->oformat->flags & AVFMT_RAWPICTURE) {
        /* a hack to avoid data copy with some raw video muxers */
        ffmpeg::AVPacket pkt;
        ffmpeg::av_init_packet(&pkt);
        if (!frame) return false;
        pkt.flags        |= AV_PKT_FLAG_KEY;
		pkt.stream_index  = output_stream_.st->index;
        pkt.data          = (uint8_t *)frame;
        pkt.size          = sizeof(ffmpeg::AVPicture);
        pkt.pts = pkt.dts = frame->pts;
        ffmpeg::av_packet_rescale_ts(&pkt, c->time_base, 
			output_stream_.st->time_base);
		ret = ffmpeg::av_interleaved_write_frame(format_context_, &pkt);
    } else {
        ffmpeg::AVPacket pkt = { 0 };
        ffmpeg::av_init_packet(&pkt);
        /* encode the image */
		ret = ffmpeg::avcodec_encode_video2(c, &pkt, frame, &got_packet);
		if (ret < 0) {
			fprintf(stderr, "Error encoding video frame: %d\n", ret);
			return false;
		}
		if(got_packet) 
			ret = write_frame(&c->time_base, &pkt);
		else
			ret = 0;
    }
    if (ret < 0) {
        fprintf(stderr, "Error while writing video frame: %d\n", ret);
        return false;
    }
    return (frame || got_packet) ? true : false;
}

ffmpeg::AVFrame * QVideoEncoder::get_video_frame() {
	
	ffmpeg::AVCodecContext *c = output_stream_.st->codec;
    /* check if we want to generate more frames */
    if (ffmpeg::av_compare_ts(output_stream_.next_pts, output_stream_.st->codec->time_base,
                      10, ffmpeg::av_make_q( 1, 1 )) >= 0)
        return NULL;
    if (c->pix_fmt !=  ffmpeg::AV_PIX_FMT_YUV420P) {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
        if (!output_stream_.sws_ctx) {
            output_stream_.sws_ctx = ffmpeg::sws_getContext(c->width, c->height,
                                           ffmpeg::AV_PIX_FMT_YUV420P,
                                          c->width, c->height,
                                          c->pix_fmt,
                                          SCALE_FLAGS, NULL, NULL, NULL);
            if (!output_stream_.sws_ctx) {
                fprintf(stderr,
                        "Could not initialize the conversion context\n");
                return NULL;
            }
        }
        fill_yuv_image(output_stream_.next_pts);
        ffmpeg::sws_scale(output_stream_.sws_ctx,
                  (const uint8_t * const *)output_stream_.tmp_frame->data, output_stream_.tmp_frame->linesize,
                  0, c->height, output_stream_.frame->data, output_stream_.frame->linesize);
    } else {
        fill_yuv_image(output_stream_.next_pts);
    }
    output_stream_.frame->pts = output_stream_.next_pts++;
    return output_stream_.frame;
}

int QVideoEncoder::write_frame(const ffmpeg::AVRational *time_base, 
	ffmpeg::AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(pkt, *time_base, output_stream_.st->time_base);
    pkt->stream_index = output_stream_.st->index;
    /* Write the compressed frame to the media file. */
    //log_packet(format_context_, pkt);
    return av_interleaved_write_frame(format_context_, pkt);
}
/* Prepare a dummy image. */
void QVideoEncoder::fill_yuv_image(int64_t frame_index) {
	ffmpeg::AVFrame *pict = output_stream_.frame;
    size_t x, y, i;
    int ret;
    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    ret = ffmpeg::av_frame_make_writable(pict);
    if (ret < 0)
        return;
    i = frame_index & 0xFFFF;
    /* Y */
    for (y = 0; y < height_; y++)
        for (x = 0; x < width_; x++)
            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;
    /* Cb and Cr */
    for (y = 0; y < height_ / 2; y++) {
        for (x = 0; x < width_ / 2; x++) {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}
//main method to set the RGB data.
bool QVideoEncoder::set_frame_rgb_data(unsigned char * data) {
	/* as we only generate a YUV420P picture, we must convert it
		* to the codec pixel format if needed */
	if (!output_stream_.sws_ctx) {
		output_stream_.sws_ctx = ffmpeg::sws_getContext(
			width_, height_,ffmpeg::AV_PIX_FMT_RGB24,
			width_, height_, ffmpeg::AV_PIX_FMT_YUV420P, 
			SCALE_FLAGS, NULL, NULL, NULL);
		if (!output_stream_.sws_ctx) {
			fprintf(stderr, "Could not initialize the conversion context\n");
			return false;
		}
	}
	unsigned char * temp_array = new unsigned char[width_ * height_ * 3 + 16];
	unsigned char * aligned_data = &temp_array[0];
	//align the data
	if ((uintptr_t)data &15) {
		while((uintptr_t)aligned_data &15)
			aligned_data++;
	}
	//this is where data gets cropped if width/height are not divisible by 16
	for(size_t i = 0; i < height_; i++)
		memcpy(
		aligned_data + (3 * i * width_),
		data         + (3 * i * actual_width_),
		3 * width_);
	//now convert the pixel format
	uint8_t * inData[] = { aligned_data }; // RGB24 have one plane
    int inLinesize[] = { (int)(3*width_) }; // RGB stride
    int ret = ffmpeg::av_frame_make_writable(output_stream_.frame);
    if (ret < 0)
        return false;
	ffmpeg::sws_scale(output_stream_.sws_ctx,
				inData, inLinesize,
				0, height_, output_stream_.frame->data, 
				output_stream_.frame->linesize);
	delete temp_array;
	return true;
}