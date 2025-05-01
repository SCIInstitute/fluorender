/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include <QVideoEncoder.h>
#include <compatibility.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#include <Debug.h>

#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC

QVideoEncoder::QVideoEncoder()
{
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
	format_context_ = NULL;
	av_codec_context_ = NULL;
	filename_ = L"";
	width_ = 0;
	height_ = 0;
	actual_width_ = 0;
	actual_height_ = 0;
	fps_ = 0;
	bitrate_ = 0;
	gop_ = 0;
	valid_ = false;
}


void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{
	char log_message[1024];
	vsnprintf(log_message, sizeof(log_message), fmt, vl);
	OutputDebugStringA(log_message);
}

bool QVideoEncoder::open(
	const std::wstring& f,
	size_t w, size_t h, size_t len,
	size_t fps, size_t bitrate)
{
	av_log_set_level(AV_LOG_DEBUG);
	av_log_set_callback(ffmpeg_log_callback);

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
	width_ = ((size_t)(w / 16)) * 16;
	height_ = ((size_t)(h / 16)) * 16;
	fps_ = fps;
	bitrate_ = bitrate;
	if (len <= fps * 3)
	{
		gop_ = 0;
		avformat_alloc_output_context2(
			&format_context_, NULL, "mpeg", ws2s(filename_).c_str());
	}
	else
	{
		gop_ = fps > 30 ? 30 : fps;
		avformat_alloc_output_context2(
			&format_context_, NULL, NULL, ws2s(filename_).c_str());
	}
	if (!format_context_) {
		std::cerr << "Could not deduce output" <<
			"format from file extension: using MPEG.\n";
		avformat_alloc_output_context2(
			&format_context_, NULL, "mpeg", ws2s(filename_).c_str());
	}
	if (!format_context_) return false;
	const AVOutputFormat* format = format_context_->oformat;
	/* Add the audio and video streams using the default format codecs
	 * and initialize the codecs. */
	if (format->video_codec == AV_CODEC_ID_NONE)
		return false;
	/* Now that all the parameters are set, we can open the audio and
	 * video codecs and allocate the necessary encode buffers. */
	if (!open_video()) return false;
	//av_dump_format(format_context_, 0, ws2s(filename_).c_str(), 1);
	/* open the output file, if needed */
	if (!(format->flags & AVFMT_NOFILE)) {
		int ret = avio_open(&format_context_->pb,
			ws2s(filename_).c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			fprintf(stderr, "Could not open '%s': %d\n", ws2s(filename_).c_str(), ret);
			return false;
		}
	}
	/* Write the stream header, if any. */
	int ret = avformat_write_header(format_context_, NULL);
	if (ret < 0) {
		fprintf(stderr, "Error occurred when opening output file: %d\n", ret);
		return false;
	}
	valid_ = true;
	return true;
}

QVideoEncoder::~QVideoEncoder() {

}

bool QVideoEncoder::open_video()
{
	const AVOutputFormat* format = format_context_->oformat;
	const AVCodec* video_codec = avcodec_find_encoder(format->video_codec);

	/* find the encoder */
	if (!video_codec) {
		fprintf(stderr, "Could not find encoder for '%s'\n",
			avcodec_get_name(format->video_codec));
		return false;
	}
	output_stream_.st = avformat_new_stream(format_context_, video_codec);
	if (!output_stream_.st) {
		fprintf(stderr, "Could not allocate stream\n");
		return false;
	}
	//output_stream_.st->id = format_context_->nb_streams - 1;

	av_codec_context_ = avcodec_alloc_context3(video_codec);
	if (!av_codec_context_) {
		fprintf(stderr, "Could not allocate codec context\n");
		return false;
	}
	av_codec_context_->codec_id = format->video_codec;
	av_codec_context_->bit_rate = bitrate_;
	/* Resolution must be a multiple of two. */
	av_codec_context_->width = static_cast<int>(width_);
	av_codec_context_->height = static_cast<int>(height_);
	av_codec_context_->time_base = { 1, static_cast<int>(fps_) };
	av_codec_context_->gop_size = static_cast<int>(gop_); /* emit one intra frame every twelve frames at most */
	av_codec_context_->pix_fmt = STREAM_PIX_FMT;

	if (av_codec_context_->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
		/* just for testing, we also add B frames */
		av_codec_context_->max_b_frames = 2;
	}
	if (av_codec_context_->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
		/* Needed to avoid using macroblocks in which some coeffs overflow.
			* This does not happen with normal video, it just happens here as
			* the motion of the chroma plane does not match the luma plane. */
		av_codec_context_->mb_decision = 2;
	}
	/* Some formats want stream headers to be separate. */
	if (format_context_->oformat->flags & AVFMT_GLOBALHEADER)
		av_codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	/* open the codec */
	if (avcodec_open2(av_codec_context_, video_codec, NULL) < 0) {
		fprintf(stderr, "Could not open video codec\n");
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
	if (av_codec_context_->pix_fmt != AV_PIX_FMT_YUV420P) {
		output_stream_.tmp_frame = alloc_picture();
		if (!output_stream_.tmp_frame) {
			fprintf(stderr, "Could not allocate temporary picture\n");
			return false;
		}
	}

	// Set the codec parameters for the stream
	int ret = avcodec_parameters_from_context(output_stream_.st->codecpar, av_codec_context_);
	if (ret < 0) {
		fprintf(stderr, "Could not copy codec parameters\n");
		return false;
	}

	return true;
}

AVFrame * QVideoEncoder::alloc_picture() {
	AVFrame *picture;
	int ret;
	picture = av_frame_alloc();
	if (!picture)
		return NULL;
	picture->format = AV_PIX_FMT_YUV420P;
	picture->width = static_cast<int>(width_);
	picture->height = static_cast<int>(height_);
	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(picture, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate frame data.\n");
		return NULL;
	}
	return picture;
}

void QVideoEncoder::close()
{
	if (!valid_) return;
	//flush the remaining (delayed) frames.
	int64_t last_dts = 0;
	AVPacket *pkt = av_packet_alloc();
	if (!pkt)
		return;
	while (true)
	{
		int ret;
		AVFrame* frame = output_stream_.frame;

		// Send the frame to the encoder
		ret = avcodec_send_frame(av_codec_context_, frame);
		if (ret == AVERROR(EAGAIN)) {
			// The encoder is not ready to accept a new frame, try again
			continue;
		}
		else if (ret < 0) {
			fprintf(stderr, "Error sending a frame for encoding: %d\n", ret);
			break;
		}

		// Receive the encoded packet from the encoder
		while (ret >= 0) {
			ret = avcodec_receive_packet(av_codec_context_, pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				break;
			}
			else if (ret < 0) {
				fprintf(stderr, "Error encoding video frame: %d\n", ret);
				break;
			}

			if (last_dts == pkt->pts)
				break;
			last_dts = pkt->dts;

			// Process the encoded packet (e.g., write it to a file)
			ret = write_frame(&av_codec_context_->time_base, pkt);
			av_packet_unref(pkt);
		}

		if (ret == AVERROR_EOF)
			break;
	}

	// Flush the encoder
	avcodec_send_frame(av_codec_context_, NULL);
	while (avcodec_receive_packet(av_codec_context_, pkt) == 0) {
		// Process the encoded packet (e.g., write it to a file)
		write_frame(&av_codec_context_->time_base, pkt);
		av_packet_unref(pkt);
	}
	av_packet_free(&pkt);

	// Write the trailer to the output media file
	av_write_trailer(format_context_);
	/* Close each codec. */
	avcodec_free_context(&av_codec_context_);
	av_frame_free(&output_stream_.frame);
	av_frame_free(&output_stream_.tmp_frame);
	sws_freeContext(output_stream_.sws_ctx);
	swr_free(&output_stream_.swr_ctx);
	const AVOutputFormat* format = format_context_->oformat;
	if (!(format->flags & AVFMT_NOFILE))
		/* Close the output file. */
		avio_closep(&format_context_->pb);
	/* free the stream */
	avformat_free_context(format_context_);

	valid_ = false;
}

void QVideoEncoder::log_packet(const AVFormatContext *fmt_ctx,
	const AVPacket *pkt) {
	AVRational *time_base =
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

bool QVideoEncoder::write_video_frame(size_t frame_num)
{
	if (!valid_) return false;

	// Set the presentation timestamp (pts) for the frame
	output_stream_.frame->pts = frame_num;
	output_stream_.next_pts = frame_num + 1;
	AVFrame* frame = av_frame_clone(output_stream_.frame); // Clone the frame to store in the queue
	frame_queue_.push(frame);

	// Allocate a new packet
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		fprintf(stderr, "Could not allocate packet\n");
		return false;
	}

	// Process frames from the queue
	while (!frame_queue_.empty()) {
		AVFrame* queued_frame = frame_queue_.front();
		frame_queue_.pop();

		// Send the frame to the encoder
		int ret = avcodec_send_frame(av_codec_context_, queued_frame);
		av_frame_free(&queued_frame); // Free the frame after sending it
		if (ret < 0) {
			fprintf(stderr, "Error sending a frame for encoding: %d\n", ret);
			av_packet_free(&pkt);
			return false;
		}

		// Receive the encoded packet from the encoder
		ret = avcodec_receive_packet(av_codec_context_, pkt);
		if (ret == AVERROR(EAGAIN)) {
			// Encoder needs more frames, break the loop and continue feeding frames
			break;
		}
		else if (ret == AVERROR_EOF) {
			// Encoder has been fully flushed, no more packets
			av_packet_free(&pkt);
			return false;
		}
		else if (ret < 0) {
			fprintf(stderr, "Error encoding video frame: %d\n", ret);
			av_packet_free(&pkt);
			return false;
		}

		// Write the encoded packet to the output file
		ret = write_frame(&av_codec_context_->time_base, pkt);
		av_packet_unref(pkt);

		if (ret < 0) {
			fprintf(stderr, "Error while writing video frame: %d\n", ret);
			av_packet_free(&pkt);
			return false;
		}
	}

	av_packet_free(&pkt);
	return true;
}

int QVideoEncoder::write_frame(const AVRational *time_base,
	AVPacket *pkt)
{
	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(pkt, *time_base, output_stream_.st->time_base);
	pkt->stream_index = output_stream_.st->index;
	/* Write the compressed frame to the media file. */
	//log_packet(format_context_, pkt);
	return av_interleaved_write_frame(format_context_, pkt);
}

//main method to set the RGB data.
bool QVideoEncoder::set_frame_rgb_data(unsigned char * data) {
	/* as we only generate a YUV420P picture, we must convert it
		* to the codec pixel format if needed */
	if (!output_stream_.sws_ctx) {
		output_stream_.sws_ctx = sws_getContext(
			static_cast<int>(width_), static_cast<int>(height_), AV_PIX_FMT_RGB24,
			static_cast<int>(width_), static_cast<int>(height_), AV_PIX_FMT_YUV420P,
			SCALE_FLAGS, NULL, NULL, NULL);
		if (!output_stream_.sws_ctx) {
			fprintf(stderr, "Could not initialize the conversion context\n");
			return false;
		}
	}
	unsigned char * temp_array = new unsigned char[width_ * height_ * 3 + 16];
	unsigned char * aligned_data = &temp_array[0];
	//align the data
	if ((uintptr_t)data & 15) {
		while ((uintptr_t)aligned_data & 15)
			aligned_data++;
	}
	//this is where data gets cropped if width/height are not divisible by 16
	for (size_t i = 0; i < height_; i++)
		std::memcpy(
			aligned_data + (3 * i * width_),
			data + (3 * i * actual_width_),
			3 * width_);
	//now convert the pixel format
	uint8_t * inData[] = { aligned_data }; // RGB24 have one plane
	int inLinesize[] = { (int)(3 * width_) }; // RGB stride
	int ret = av_frame_make_writable(output_stream_.frame);
	if (ret < 0)
	{
		delete[]temp_array;
		return false;
	}
	sws_scale(output_stream_.sws_ctx,
		inData, inLinesize,
		0, static_cast<int>(height_), output_stream_.frame->data,
		output_stream_.frame->linesize);
	delete[]temp_array;
	return true;
}
