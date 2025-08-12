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
#ifndef __QVideoEncoder_H
#define __QVideoEncoder_H

#include <string>

struct AVStream;
struct AVFrame;
struct SwsContext;
struct SwrContext;
struct AVFormatContext;
struct AVCodecContext;
struct AVRational;
struct AVPacket;
class VideoEncoder
{
public:
	VideoEncoder();
	virtual ~VideoEncoder();
	bool open(const std::wstring& f, size_t w, size_t h, size_t len, size_t fps, size_t bitrate);
	void close();
	bool set_frame_rgb_data(unsigned char * data, bool);
	bool write_video_frame(size_t frame_num);//main method to set the RGB data.

protected:
	//video basics
	size_t width_, height_, bitrate_, gop_, fps_;
	//the original dimensions (video is cropped so width and height are both
	//divisible by 16.)
	size_t actual_width_, actual_height_;
	std::wstring filename_;
	bool valid_;

	// a wrapper around a single output AVStream
	typedef struct OutputStream {
		AVStream *st;
		/* pts of the next frame that will be generated */
		int64_t next_pts;
		int samples_count;
		AVFrame *frame;
		AVFrame *tmp_frame;
		float t, tincr, tincr2;
		SwsContext *sws_ctx;
		SwrContext *swr_ctx;
	} OutputStream;

	//codec and format details.
	OutputStream output_stream_;
	AVFormatContext *format_context_;
	AVCodecContext* av_codec_context_;

	//interior functions
	bool open_video();
	AVFrame * alloc_picture();
	int write_frame(const AVRational *time_base, AVPacket *pkt);
};

#endif // QVideoEncoder_H
