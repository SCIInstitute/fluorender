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
#include <mpg_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

MPGReader::MPGReader():
	BaseReader()
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

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;

	m_stream_index = -1;
	m_av_format_context = NULL;
	m_av_codec_context = NULL;
	m_sws_context = NULL;
	m_frame_yuv = NULL;
	m_frame_rgb = NULL;
	m_frame_buffer = NULL;

	m_cache_size_limit = glbin_settings.m_mpg_cache_size;
}

MPGReader::~MPGReader()
{
	release_cache();
	//release frame cache
	if (m_frame_yuv)
		av_frame_free(&m_frame_yuv);
	if (m_frame_rgb)
		av_frame_free(&m_frame_rgb);
	if (m_frame_buffer)
		av_free(m_frame_buffer);
	// Close the codecs
	if (m_av_codec_context)
		avcodec_free_context(&m_av_codec_context);
	// Close the video file
	if (m_av_format_context)
		avformat_close_input(&m_av_format_context);
}

void MPGReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int MPGReader::Preprocess()
{
	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;
	m_data_name = name;

	AVPacket packet;

	// Open video file
	std::string str = ws2s(m_path_name);
	if (avformat_open_input(&m_av_format_context, str.c_str(), NULL, NULL) != 0)
		return READER_OPEN_FAIL; // Couldn't open file

	// Retrieve stream information
	if (avformat_find_stream_info(m_av_format_context, NULL) < 0)
		return READER_OPEN_FAIL; // Couldn't find stream information

	// Find the first video stream
	m_stream_index = -1;
	for (size_t i = 0; i < m_av_format_context->nb_streams; ++i)
	{
		if (m_av_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_stream_index = static_cast<int>(i);
			break;
		}
	}
	if (m_stream_index == -1)
		return READER_OPEN_FAIL; // Didn't find a video stream

	// Get codec parameters from the video stream
	AVCodecParameters *pCodecPar = m_av_format_context->streams[m_stream_index]->codecpar;

	// Find the decoder for the video stream
	const AVCodec *pCodec = avcodec_find_decoder(pCodecPar->codec_id);
	if (!pCodec)
		return READER_OPEN_FAIL; // Codec not found

	// Copy context
	m_av_codec_context = avcodec_alloc_context3(pCodec);
	if (avcodec_parameters_to_context(m_av_codec_context, pCodecPar) != 0)
		return READER_OPEN_FAIL; // Error copying codec context
	// Open codec
	if (avcodec_open2(m_av_codec_context, pCodec, NULL) < 0)
		return READER_OPEN_FAIL; // Could not open codec

	// initialize SWS context for software scaling
	m_sws_context = sws_getContext(
		m_av_codec_context->width,
		m_av_codec_context->height,
		m_av_codec_context->pix_fmt,
		m_av_codec_context->width,
		m_av_codec_context->height,
		AV_PIX_FMT_RGB24,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL);
	if (!m_sws_context)
		return READER_OPEN_FAIL;

	m_mpg_info.clear();
	size_t num = m_av_format_context->streams[m_stream_index]->nb_frames;
	m_mpg_info.reserve(num);

	// Allocate video frame
	if (!m_frame_yuv)
		m_frame_yuv = av_frame_alloc();
	//read frame
	int i = 0;
	while (av_read_frame(m_av_format_context, &packet) >= 0)
	{
		if (packet.stream_index == m_stream_index)
		{
			// Send the packet with the compressed data to the decoder
			if (avcodec_send_packet(m_av_codec_context, &packet) < 0) {
				// Error sending a packet for decoding
				av_packet_unref(&packet);
				continue;
			}

			// Receive the decoded frame from the decoder
			while (avcodec_receive_frame(m_av_codec_context, m_frame_yuv) >= 0) {
				// Process the decoded frame (m_frame_yuv)
				FrameInfo info = get_frame_info(packet.dts, packet.pts);
				m_mpg_info.push_back(info);
				i++;
				SetProgress(static_cast<int>(std::round(100.0 * i / num)), "NOT_SET");
			}
		}
		// Free the packet that was allocated by av_read_frame
		av_packet_unref(&packet);
	}

	//get time num
	m_time_num = static_cast<int>(m_mpg_info.size());
	m_cur_time = 0;

	m_chan_num = 3;
	m_x_size = m_av_codec_context->width;
	m_y_size = m_av_codec_context->height;
	m_slice_num = 1;

	m_valid_spc = true;
	m_xspc = 1;
	m_yspc = 1;
	m_zspc = 1;

	m_max_value = 255.0;
	m_scalar_scale = 1;

	// Close the codecs
	//avcodec_close(pCodecCtxOrig);

	return READER_OK;
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
	Nrrd *data = 0;
	if (m_mpg_info.empty() ||
		m_stream_index == -1 ||
		!m_av_format_context||
		!m_av_codec_context ||
		!m_sws_context)
		return data;

	t = std::clamp(t, 0, m_time_num - 1);
	c = std::clamp(c, 0, 2);

	// Check if the frame is already in the cache
	auto it = m_frame_cache.find(t);
	if (it != m_frame_cache.end())
	{
		// Move accessed frame to the front of the cache order list
		m_cache_order.remove(t);
		m_cache_order.push_front(t);

		AVFrame* cached_frame = it->second;
		data = get_nrrd(cached_frame, c);
		m_cur_time = t;

		return data;
	}

	if (t == m_cur_time && m_frame_rgb)
	{
		return get_nrrd(m_frame_rgb, c);
	}
	m_cur_time = t;

	// Allocate video frame
	if (!m_frame_yuv)
		return data;

	// Allocate an AVFrame structure
	if (!m_frame_rgb)
	{
		m_frame_rgb = av_frame_alloc();
		if (!m_frame_rgb)
			return data;

		// Determine required buffer size and allocate buffer
		int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_x_size, m_y_size, 1);
		m_frame_buffer = static_cast<uint8_t*>(av_malloc(numBytes));

		// Assign appropriate parts of buffer to image planes in pFrameRGB
		// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
		// of AVPicture
		av_image_fill_arrays(m_frame_rgb->data, m_frame_rgb->linesize, m_frame_buffer, AV_PIX_FMT_RGB24, m_x_size, m_y_size, 1);
	}

	//seek frame
	//int prev_t = std::max(t - 1, 0);
	int64_t target = m_mpg_info[t].dts;
	if (av_seek_frame(m_av_format_context, m_stream_index, target, AVSEEK_FLAG_BACKWARD) < 0)
		return data;

	// Read frame
	AVPacket packet;

	while (av_read_frame(m_av_format_context, &packet) >= 0)
	{
		if (packet.stream_index == m_stream_index)
		{
			// Send the packet with the compressed data to the decoder
			if (avcodec_send_packet(m_av_codec_context, &packet) < 0) {
				// Error sending a packet for decoding
				av_packet_unref(&packet);
				continue;
			}

			// Receive the decoded frame from the decoder
			while (avcodec_receive_frame(m_av_codec_context, m_frame_yuv) >= 0) {
				// Did we get a video frame?
				if (packet.pts == m_mpg_info[t].pts)
				{
					// Convert the image from its native format to RGB
					sws_scale(m_sws_context, m_frame_yuv->data,
						m_frame_yuv->linesize, 0, m_av_codec_context->height,
						m_frame_rgb->data, m_frame_rgb->linesize);

					data = get_nrrd(m_frame_rgb, c);

					// Add the decoded frame to the cache
					add_cache(t, m_frame_rgb);

					av_packet_unref(&packet);
					return data;
				}
			}
		}
		// Free the packet that was allocated by av_read_frame
		av_packet_unref(&packet);
	}
	return data;
}

std::wstring MPGReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring MPGReader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring MPGReader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
	return label_name;
}

MPGReader::FrameInfo MPGReader::get_frame_info(int64_t dts, int64_t pts)
{
	FrameInfo info;
	info.pts = pts;
	info.dts = dts < 0 ? 0 : dts;
	return info;
}

Nrrd* MPGReader::get_nrrd(AVFrame* frame, int c)
{
	//extract channel
	unsigned long long total_size = (unsigned long long)m_x_size * (unsigned long long)m_y_size;
	uint8_t* val = new unsigned char[total_size];
	unsigned long long index;
	for (index = 0; index < total_size; ++index)
		val[index] = *(frame->data[0] + index * 3 + c);

	//create nrrd
	Nrrd* data = nrrdNew();
	nrrdWrap_va(data, (uint8_t*)val, nrrdTypeUChar,
		3, (size_t)m_x_size, (size_t)m_y_size, (size_t)1);
	nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, m_xspc * m_x_size, m_yspc * m_y_size, m_zspc);
	nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)1);

	return data;
}

void MPGReader::add_cache(int t, AVFrame* frame)
{
	// Check if the cache size limit is exceeded
	if (m_cache_size_limit == 0)
		return;
	else if (m_cache_size_limit > 0)
	{
		while (m_frame_cache.size() >= m_cache_size_limit) {
			invalidate_cache();
		}
	}

	// Allocate the frame
	AVFrame* cached_frame = av_frame_alloc();
	if (!cached_frame) {
		return; // Handle allocation failure
	}

	// Allocate the image buffer
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_x_size, m_y_size, 1);
	uint8_t* buffer = static_cast<uint8_t*>(av_malloc(numBytes));
	if (!buffer) {
		av_frame_free(&cached_frame);
		return; // Handle allocation failure
	}

	// Fill the frame data arrays
	av_image_fill_arrays(cached_frame->data, cached_frame->linesize, buffer, AV_PIX_FMT_RGB24, m_x_size, m_y_size, 1);

	// Copy the frame data from the source frame
	av_frame_copy_props(cached_frame, frame); // Copy frame properties
	for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i)
	{
		if (frame->data[i]) {
			memcpy(cached_frame->data[i], frame->data[i], frame->linesize[i] * m_y_size);
		}
	}

	// Add the frame to the cache and update the order list
	m_frame_cache[t] = cached_frame;
	m_cache_order.push_front(t);
}

void MPGReader::invalidate_cache()
{
	// Remove the least recently used frame from the cache
	int lru_frame = m_cache_order.back();
	m_cache_order.pop_back();
	av_frame_free(&m_frame_cache[lru_frame]);
	m_frame_cache.erase(lru_frame);
}

void MPGReader::release_cache()
{
	for (auto& entry : m_frame_cache)
	{
		av_freep(&entry.second->data[0]); // Free the buffer
		av_frame_free(&entry.second);
	}
	m_frame_cache.clear();
	m_cache_order.clear();
}
