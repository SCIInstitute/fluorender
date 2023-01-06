/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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
#include "mpg_reader.h"
#include <compatibility.h>

MPGReader::MPGReader()
{
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;

	m_valid_spc = false;
	m_xspc = 0.0;
	m_yspc = 0.0;
	m_zspc = 0.0;

	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_batch = false;
	m_cur_batch = -1;
}

MPGReader::~MPGReader()
{
}

void MPGReader::SetFile(string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
	m_id_string = m_path_name;
}

void MPGReader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int MPGReader::Preprocess()
{
	wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;
	m_data_name = name;

	int i, videoStream, frameFinished;
	ffmpeg::AVFormatContext* pFormatCtx = NULL;
	ffmpeg::AVCodecContext* pCodecCtxOrig = NULL;
	ffmpeg::AVCodecContext* pCodecCtx = NULL;
	ffmpeg::AVCodec* pCodec = NULL;
	ffmpeg::AVPacket packet;

	// Open video file
	std::string str = ws2s(m_path_name);
	if (ffmpeg::avformat_open_input(&pFormatCtx, str.c_str(), NULL, NULL) != 0)
		return READER_OPEN_FAIL; // Couldn't open file

	// Retrieve stream information
	if (ffmpeg::avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return READER_OPEN_FAIL; // Couldn't find stream information

	// Find the first video stream
	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == ffmpeg::AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1)
		return READER_OPEN_FAIL; // Didn't find a video stream

	// Get a pointer to the codec context for the video stream
	pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec = ffmpeg::avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL)
		return READER_OPEN_FAIL; // Codec not found
	// Copy context
	pCodecCtx = ffmpeg::avcodec_alloc_context3(pCodec);
	if (ffmpeg::avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0)
		return READER_OPEN_FAIL; // Error copying codec context
	// Open codec
	if (ffmpeg::avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		return READER_OPEN_FAIL; // Could not open codec

	m_mpg_info.clear();
	m_mpg_info.reserve(pFormatCtx->streams[videoStream]->nb_frames);

	// Allocate video frame
	ffmpeg::AVFrame* pFrame = ffmpeg::av_frame_alloc();
	//read frame
	while (ffmpeg::av_read_frame(pFormatCtx, &packet) >= 0)
	{
		if (packet.stream_index == videoStream)
		{
			// Decode video frame
			ffmpeg::avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			// Did we get a video frame?
			if (frameFinished)
			{
				FrameInfo info = get_frame_info(packet.dts, packet.pts);
				//info.pts = packet.pts < 0 ? 0 : packet.pts;
				//info.dts = packet.dts < 0 ? 0 : packet.dts;
				//info.pos = get_pos(info.dts, info.pts);
				m_mpg_info.push_back(info);
			}
		}
		// Free the packet that was allocated by av_read_frame
		ffmpeg::av_free_packet(&packet);
	}

	//get time num
	m_time_num = m_mpg_info.size();
	m_cur_time = 0;

	m_chan_num = 3;
	m_x_size = pCodecCtx->width;
	m_y_size = pCodecCtx->height;
	m_slice_num = 1;

	m_valid_spc = true;
	m_xspc = 1;
	m_yspc = 1;
	m_zspc = 1;

	m_max_value = 255.0;
	m_scalar_scale = 1;

	ffmpeg::av_frame_free(&pFrame);
	// Close the codecs
	ffmpeg::avcodec_close(pCodecCtx);
	ffmpeg::avcodec_close(pCodecCtxOrig);
	// Close the video file
	ffmpeg::avformat_close_input(&pFormatCtx);

	return READER_OK;
}

void MPGReader::SetSliceSeq(bool ss)
{
	//do nothing
}

bool MPGReader::GetSliceSeq()
{
	return false;
}

void MPGReader::SetChannSeq(bool cs)
{
	//do nothing
}

bool MPGReader::GetChannSeq()
{
	return false;
}

void MPGReader::SetDigitOrder(int order)
{
	//do nothing
}

int MPGReader::GetDigitOrder()
{
	return 0;
}

void MPGReader::SetTimeId(wstring &id)
{
	//do nothing
}

wstring MPGReader::GetTimeId()
{
	return wstring(L"");
}

void MPGReader::SetBatch(bool batch)
{
}

int MPGReader::LoadBatch(int index)
{
	int result = -1;
	if (index >= 0 && index < (int)m_batch_list.size())
	{
		m_path_name = m_batch_list[index];
		Preprocess();
		result = index;
		m_cur_batch = result;
	}
	else
		result = -1;

	return result;
}

double MPGReader::GetExcitationWavelength(int chan)
{
	return 0.0;
}

Nrrd* MPGReader::Convert(int t, int c, bool get_max)
{
	if (t < 0) t = 0;
	if (t >= m_time_num) t = m_time_num - 1;
	if (c < 0) c = 0;
	if (c > 2) c = 2;

	Nrrd *data = 0;
	if (m_mpg_info.empty())
		return data;

	int i, videoStream, frameFinished;
	ffmpeg::AVFormatContext* pFormatCtx = NULL;
	ffmpeg::AVCodecContext* pCodecCtxOrig = NULL;
	ffmpeg::AVCodecContext* pCodecCtx = NULL;
	ffmpeg::AVCodec* pCodec = NULL;
	struct ffmpeg::SwsContext* sws_ctx = NULL;
	ffmpeg::AVPacket packet;

	// Open video file
	std::string str = ws2s(m_path_name);
	if (ffmpeg::avformat_open_input(&pFormatCtx, str.c_str(), NULL, NULL) != 0)
		return data; // Couldn't open file

	// Retrieve stream information
	if (ffmpeg::avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return data; // Couldn't find stream information

	// Find the first video stream
	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == ffmpeg::AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1)
		return data; // Didn't find a video stream

	// Get a pointer to the codec context for the video stream
	pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec = ffmpeg::avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL)
		return data; // Codec not found
	// Copy context
	pCodecCtx = ffmpeg::avcodec_alloc_context3(pCodec);
	if (ffmpeg::avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0)
		return data; // Error copying codec context
	// Open codec
	if (ffmpeg::avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		return data; // Could not open codec

	// Allocate video frame
	ffmpeg::AVFrame* pFrame = ffmpeg::av_frame_alloc();

	// Allocate an AVFrame structure
	ffmpeg::AVFrame* pFrameRGB = ffmpeg::av_frame_alloc();
	if (pFrameRGB == NULL)
		return data;

	// Determine required buffer size and allocate buffer
	int numBytes = ffmpeg::avpicture_get_size(ffmpeg::AV_PIX_FMT_RGB24, pCodecCtx->width,
		pCodecCtx->height);
	uint8_t* buffer = (uint8_t*)ffmpeg::av_malloc(numBytes * sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	ffmpeg::avpicture_fill((ffmpeg::AVPicture*)pFrameRGB, buffer, ffmpeg::AV_PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height);

	// initialize SWS context for software scaling
	sws_ctx = ffmpeg::sws_getContext(
		pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		ffmpeg::AV_PIX_FMT_RGB24,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL);

	//seek frame
	int64_t target = m_mpg_info[t].tgt;
	if (ffmpeg::av_seek_frame(pFormatCtx, videoStream, target, AVSEEK_FLAG_FRAME) < 0)
		return data;

	//read frame
	size_t count = 0;
	while (ffmpeg::av_read_frame(pFormatCtx, &packet) >= 0)
	{
		if (packet.stream_index == videoStream)
		{
			// Decode video frame
			ffmpeg::avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			// Did we get a video frame?
			if (frameFinished)
			{
				if (m_mpg_info[t].pos == count)
				{
					// Convert the image from its native format to RGB
					ffmpeg::sws_scale(sws_ctx, (uint8_t const* const*)pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height,
						pFrameRGB->data, pFrameRGB->linesize);

					//extract channel
					unsigned long long total_size = (unsigned long long)m_x_size * (unsigned long long)m_y_size;
					uint8_t* val = new unsigned char[total_size];
					unsigned long long index;
					for (index = 0; index < total_size; ++index)
						val[index] = *(pFrameRGB->data[0] + index * 3 + c);

					//create nrrd
					data = nrrdNew();
					nrrdWrap(data, (uint8_t*)val, nrrdTypeUChar,
						3, (size_t)m_x_size, (size_t)m_y_size, (size_t)1);
					nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
					nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc * m_x_size, m_yspc * m_y_size, m_zspc);
					nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
					nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)1);

					ffmpeg::av_free_packet(&packet);
					break;
				}

				count++;
			}
		}
		// Free the packet that was allocated by av_read_frame
		ffmpeg::av_free_packet(&packet);
	}

	// Free the RGB image
	ffmpeg::av_free(buffer);
	ffmpeg::av_frame_free(&pFrameRGB);
	// Free the YUV frame
	ffmpeg::av_frame_free(&pFrame);
	// Close the codecs
	ffmpeg::avcodec_close(pCodecCtx);
	ffmpeg::avcodec_close(pCodecCtxOrig);
	// Close the video file
	ffmpeg::avformat_close_input(&pFormatCtx);

	return data;
}

wstring MPGReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

wstring MPGReader::GetCurMaskName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".msk";
	wstring mask_name = woss.str();
	return mask_name;
}

wstring MPGReader::GetCurLabelName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".lbl";
	wstring label_name = woss.str();
	return label_name;
}
