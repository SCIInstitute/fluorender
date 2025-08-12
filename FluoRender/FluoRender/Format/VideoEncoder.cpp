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
#include <VideoEncoder.h>
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
#include <memory>

#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC

VideoEncoder::VideoEncoder()
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

VideoEncoder::~VideoEncoder()
{
	if (output_stream_.frame) {
		av_frame_free(&output_stream_.frame);
	}
	if (output_stream_.tmp_frame) {
		av_frame_free(&output_stream_.tmp_frame);
	}
	if (output_stream_.sws_ctx) {
		sws_freeContext(output_stream_.sws_ctx);
	}
	if (output_stream_.swr_ctx) {
		swr_free(&output_stream_.swr_ctx);
	}
	if (av_codec_context_) {
		avcodec_free_context(&av_codec_context_);
	}
	if (format_context_) {
		avformat_free_context(format_context_);
	}
}

void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{
	char log_message[1024];
	vsnprintf(log_message, sizeof(log_message), fmt, vl);
	std::wstring str = s2ws(log_message);
	DBGPRINT(str.c_str());
}

bool VideoEncoder::open(const std::wstring& f,
	size_t w, size_t h, size_t len,
	size_t fps, size_t bitrate)
{
	output_stream_ = {}; // Zero-initialize all members
	filename_ = f;

	actual_width_ = w;
	actual_height_ = h;
	width_ = ((w + 15) / 16) * 16;   // Round up to nearest multiple of 16
	height_ = ((h + 15) / 16) * 16;

	fps_ = fps;
	bitrate_ = bitrate;
	gop_ = std::min<size_t>(fps, 30);

	// Try to deduce format from filename
	avformat_alloc_output_context2(&format_context_, NULL, NULL, ws2s(filename_).c_str());
	if (!format_context_) {
		std::cerr << "Could not deduce format, falling back to MPEG.\n";
		avformat_alloc_output_context2(&format_context_, NULL, "mpeg", ws2s(filename_).c_str());
	}
	if (!format_context_) return false;

	const AVOutputFormat* format = format_context_->oformat;
	if (format->video_codec == AV_CODEC_ID_NONE)
		return false;

	if (!open_video()) return false;

	if (!(format->flags & AVFMT_NOFILE)) {
		int ret = avio_open(&format_context_->pb, ws2s(filename_).c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			DBGPRINT(L"Could not open '%s': %d\n", filename_.c_str(), ret);
			return false;
		}
	}

	int ret = avformat_write_header(format_context_, NULL);
	if (ret < 0) {
		DBGPRINT(L"Error occurred when opening output file: %d\n", ret);
		return false;
	}

	valid_ = true;
	return true;
}

void VideoEncoder::close()
{
	if (!valid_) return;

	// Allocate a new packet
	AVPacket *pkt = av_packet_alloc();
	if (!pkt)
	{
		DBGPRINT(L"Could not allocate packet\n");
		return;
	}

	// Flush the encoder by sending a NULL frame
	int ret = avcodec_send_frame(av_codec_context_, NULL);
	if (ret < 0) {
		DBGPRINT(L"Error sending flush frame: %d\n", ret);
		av_packet_free(&pkt);
		return;
	}

	// Receive and process all remaining packets
	while (ret >= 0) {
		ret = avcodec_receive_packet(av_codec_context_, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			break;
		}
		else if (ret < 0) {
			DBGPRINT(L"Error encoding video frame: %d\n", ret);
			break;
		}

		// Process the encoded packet (e.g., write it to a file)
		ret = write_frame(&av_codec_context_->time_base, pkt);
		av_packet_unref(pkt);

		if (ret < 0) {
			DBGPRINT(L"Error while writing video frame: %d\n", ret);
			break;
		}
	}

	av_packet_free(&pkt);

	// Write the trailer to the output media file
	av_write_trailer(format_context_);

	// Free resources in the correct order
	if (output_stream_.frame) {
		av_frame_free(&output_stream_.frame);
		output_stream_.frame = 0;
	}
	if (output_stream_.tmp_frame) {
		av_frame_free(&output_stream_.tmp_frame);
		output_stream_.tmp_frame = 0;
	}
	if (output_stream_.sws_ctx) {
		sws_freeContext(output_stream_.sws_ctx);
		output_stream_.sws_ctx = 0;
	}
	if (output_stream_.swr_ctx) {
		swr_free(&output_stream_.swr_ctx);
		output_stream_.swr_ctx = 0;
	}
	if (av_codec_context_) {
		avcodec_free_context(&av_codec_context_);
		av_codec_context_ = 0;
	}

	if (format_context_)
	{
		const AVOutputFormat* format = format_context_->oformat;
		if (!(format->flags & AVFMT_NOFILE)) {
			// Close the output file
			avio_closep(&format_context_->pb);
		}

		// Free the format context
		avformat_free_context(format_context_);
		format_context_ = 0;
	}

	valid_ = false;
}

bool VideoEncoder::set_frame_rgb_data(unsigned char* data, bool flip_vertical)
{
	if (!output_stream_.sws_ctx) {
		output_stream_.sws_ctx = sws_getContext(
			static_cast<int>(width_), static_cast<int>(height_), AV_PIX_FMT_RGB24,
			static_cast<int>(width_), static_cast<int>(height_), AV_PIX_FMT_YUV420P,
			SCALE_FLAGS, NULL, NULL, NULL);
		if (!output_stream_.sws_ctx) {
			DBGPRINT(L"Could not initialize the conversion context\n");
			return false;
		}
	}

	std::unique_ptr<unsigned char[]> temp_array(new unsigned char[width_ * height_ * 3 + 16]);
	void* aligned_data_void = temp_array.get();
	size_t space = width_ * height_ * 3 + 16;
	std::align(16, width_ * height_ * 3, aligned_data_void, space);
	unsigned char* aligned_data = static_cast<unsigned char*>(aligned_data_void);

	// Copy and optionally flip vertically
	size_t row_bytes = 3 * actual_width_;
	for (size_t i = 0; i < height_; i++) {
		size_t src_row = flip_vertical ? (actual_height_ - 1 - i) : i;
		if (src_row >= actual_height_) continue; // Safety check

		std::memcpy(
			aligned_data + (3 * i * width_),
			data + (src_row * row_bytes),
			std::min(3 * width_, row_bytes)); // Avoid overruns
	}

	int ret = av_frame_make_writable(output_stream_.frame);
	if (ret < 0) {
		DBGPRINT(L"Error making frame writable: %d\n", ret);
		return false;
	}

	uint8_t* inData[] = { aligned_data };
	int inLinesize[] = { static_cast<int>(3 * width_) };

	sws_scale(output_stream_.sws_ctx,
		inData, inLinesize,
		0, static_cast<int>(height_), output_stream_.frame->data,
		output_stream_.frame->linesize);

	return true;
}

bool VideoEncoder::write_video_frame(size_t frame_num)
{
	if (!valid_) return false;

	// Set the presentation timestamp (pts) for the frame
	output_stream_.frame->pts = frame_num;
	output_stream_.next_pts = frame_num + 1;
	AVFrame* frame = output_stream_.frame; // Clone the frame to store in the queue

	// Allocate a new packet
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		DBGPRINT(L"Could not allocate packet\n");
		return false;
	}

	// Send the frame to the encoder
	int ret = avcodec_send_frame(av_codec_context_, frame);
	if (ret < 0) {
		DBGPRINT(L"Error sending a frame for encoding: %d\n", ret);
		av_packet_free(&pkt);
		return false;
	}

	// Receive the encoded packet from the encoder
	ret = avcodec_receive_packet(av_codec_context_, pkt);
	if (ret == AVERROR(EAGAIN)) {
		DBGPRINT(L"Waiting for more frames at [%d]: %d\n", frame_num, ret);
		av_packet_free(&pkt);
		return true;
	}
	else if (ret < 0) {
		DBGPRINT(L"Error encoding video frame: %d\n", ret);
		av_packet_free(&pkt);
		return false;
	}

	// Write the encoded packet to the output file
	ret = write_frame(&av_codec_context_->time_base, pkt);
	av_packet_unref(pkt);

	if (ret < 0) {
		DBGPRINT(L"Error while writing video frame: %d\n", ret);
		av_packet_free(&pkt);
		return false;
	}

	av_packet_free(&pkt);
	return true;
}

bool VideoEncoder::open_video()
{
	const AVOutputFormat* format = format_context_->oformat;
	const AVCodec* video_codec = avcodec_find_encoder(format->video_codec);

	/* find the encoder */
	if (!video_codec) {
		DBGPRINT(L"Could not find encoder for '%s'\n", avcodec_get_name(format->video_codec));
		return false;
	}
	output_stream_.st = avformat_new_stream(format_context_, video_codec);
	if (!output_stream_.st) {
		DBGPRINT(L"Could not allocate stream\n");
		return false;
	}

	av_codec_context_ = avcodec_alloc_context3(video_codec);
	if (!av_codec_context_) {
		DBGPRINT(L"Could not allocate codec context\n");
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
		DBGPRINT(L"Could not open video codec\n");
		avcodec_free_context(&av_codec_context_);
		return false;
	}
	/* allocate and init a re-usable frame */
	output_stream_.frame = alloc_picture();
	if (!output_stream_.frame) {
		DBGPRINT(L"Could not allocate video frame\n");
		avcodec_free_context(&av_codec_context_);
		return false;
	}
	/* If the output format is not YUV420P, then a temporary YUV420P
	 * picture is needed too. It is then converted to the required
	 * output format. */
	output_stream_.tmp_frame = NULL;
	if (av_codec_context_->pix_fmt != AV_PIX_FMT_YUV420P) {
		output_stream_.tmp_frame = alloc_picture();
		if (!output_stream_.tmp_frame) {
			DBGPRINT(L"Could not allocate temporary picture\n");
			av_frame_free(&output_stream_.frame);
			avcodec_free_context(&av_codec_context_);
			return false;
		}
	}

	// Set the codec parameters for the stream
	int ret = avcodec_parameters_from_context(output_stream_.st->codecpar, av_codec_context_);
	if (ret < 0) {
		DBGPRINT(L"Could not copy codec parameters\n");
		av_frame_free(&output_stream_.frame);
		if (output_stream_.tmp_frame) {
			av_frame_free(&output_stream_.tmp_frame);
		}
		avcodec_free_context(&av_codec_context_);
		return false;
	}

	return true;
}

AVFrame* VideoEncoder::alloc_picture() {
	AVFrame* picture;
	int ret;
	picture = av_frame_alloc();
	if (!picture) {
		DBGPRINT(L"Could not allocate frame.\n");
		return NULL;
	}
	picture->format = AV_PIX_FMT_YUV420P;
	picture->width = static_cast<int>(width_);
	picture->height = static_cast<int>(height_);
	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(picture, 32);
	if (ret < 0) {
		DBGPRINT(L"Could not allocate frame data: %d\n", ret);
		av_frame_free(&picture);
		return NULL;
	}
	return picture;
}

int VideoEncoder::write_frame(const AVRational* time_base, AVPacket* pkt) {
	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(pkt, *time_base, output_stream_.st->time_base);
	pkt->stream_index = output_stream_.st->index;
	/* Write the compressed frame to the media file. */
	return av_interleaved_write_frame(format_context_, pkt);
}
