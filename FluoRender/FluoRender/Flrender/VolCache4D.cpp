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
#include <VolCache4D.h>
#include <Texture.h>
#include <Global.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <RenderView.h>
#include <base_reader.h>
#include <msk_reader.h>
#include <lbl_reader.h>
#include <msk_writer.h>
#include <string>

using namespace flvr;

void VolCache4D::SetHandleFlags(int flags)
{
	handle_data = flags & CQCallback::HDL_DATA;
	handle_mask = flags & CQCallback::HDL_MASK;
	handle_label = flags & CQCallback::HDL_LABEL;
	access_data = flags & CQCallback::ACS_DATA;
	access_mask = flags & CQCallback::ACS_MASK;
	access_label = flags & CQCallback::ACS_LABEL;
	return_data = flags & CQCallback::RET_DATA;
	return_mask = flags & CQCallback::RET_MASK;
	return_label = flags & CQCallback::RET_LABEL;
	build_tex = flags & CQCallback::BLD_TEX;
	time_cond0 = flags & CQCallback::TIME_COND0;
}

bool CQCallback::cond0 = false;

void CQCallback::ReadVolCache(VolCache4D& vol_cache)
{
	if (!vol_cache.handle_data &&
		!vol_cache.handle_label &&
		!vol_cache.handle_mask &&
		!vol_cache.access_data &&
		!vol_cache.access_mask &&
		!vol_cache.access_label &&
		!vol_cache.build_tex)
		return;

	cond0 = false;
	if (vol_cache.time_cond0)
	{
		int cur_time = 0;
		auto view_ptr = glbin_current.render_view.lock();
		if (view_ptr)
			cur_time = view_ptr->m_tseq_cur_num;
		int frame = static_cast<int>(vol_cache.m_tnum);
		cond0 = cur_time == frame;
	}

	vol_cache.m_valid = true;
	if (vol_cache.handle_data)
	{
		if (cond0)
			AccessData(vol_cache);
		else
			HandleData(vol_cache);
	}
	if (vol_cache.handle_mask)
	{
		if (cond0)
			AccessMask(vol_cache);
		else
			HandleMask(vol_cache);
	}
	if (vol_cache.handle_label)
	{
		if (cond0)
			AccessLabel(vol_cache);
		else
			HandleLabel(vol_cache);
	}
	if (vol_cache.access_data)
	{
		AccessData(vol_cache);
	}
	if (vol_cache.access_mask)
	{
		AccessMask(vol_cache);
	}
	if (vol_cache.access_label)
	{
		AccessLabel(vol_cache);
	}
	if (vol_cache.build_tex)
	{
		BuildTex(vol_cache);
	}
}

void CQCallback::FreeVolCache(VolCache4D& vol_cache)
{
	if (!vol_cache.handle_data &&
		!vol_cache.handle_label &&
		!vol_cache.handle_mask &&
		!vol_cache.access_data &&
		!vol_cache.access_mask &&
		!vol_cache.access_label &&
		!vol_cache.build_tex)
		return;

	if (vol_cache.save_data)
	{
		SaveData(vol_cache);
	}
	if (vol_cache.save_mask)
	{
		SaveMask(vol_cache);
	}
	if (vol_cache.save_label)
	{
		SaveLabel(vol_cache);
	}
	if (vol_cache.own_data && vol_cache.m_data)
	{
		nrrdNuke((Nrrd*)vol_cache.m_data);
		vol_cache.m_data = 0;
	}
	if (vol_cache.own_mask && vol_cache.m_mask)
	{
		nrrdNuke((Nrrd*)vol_cache.m_mask);
		vol_cache.m_mask = 0;
	}
	if (vol_cache.own_label && vol_cache.m_label)
	{
		nrrdNuke((Nrrd*)vol_cache.m_label);
		vol_cache.m_label = 0;
	}
	vol_cache.m_valid = false;
}

bool CQCallback::HandleData(VolCache4D& vol_cache)
{
	//get volume, readers
	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;
	auto reader = vd->GetReader();
	if (!reader)
		return false;

	int frame = static_cast<int>(vol_cache.m_tnum);
	int chan = vd->GetCurChannel();

	Nrrd* data = 0;
	data = reader->Convert(frame, chan, true);
	vol_cache.m_data = data;
	vol_cache.m_valid &= (data != 0);
	vol_cache.own_data = vol_cache.m_valid;
	return data != 0;
}

bool CQCallback::HandleMask(VolCache4D& vol_cache)
{
	//get volume, readers
	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;
	auto reader = vd->GetReader();
	if (!reader)
		return false;

	int frame = static_cast<int>(vol_cache.m_tnum);
	int chan = vd->GetCurChannel();

	MSKReader msk_reader;
	std::wstring mskname = reader->GetCurMaskName(frame, chan);
	msk_reader.SetFile(mskname);
	Nrrd* mask = msk_reader.Convert(frame, chan, true);
	if (!mask)
	{
		int resx, resy, resz;
		vd->GetResolution(resx, resy, resz);
		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);
		mask = nrrdNew();
		unsigned long long mem_size = (unsigned long long)resx *
			(unsigned long long)resy * (unsigned long long)resz;
		uint8_t* val8 = new (std::nothrow) uint8_t[mem_size]();
		nrrdWrap_va(mask, val8, nrrdTypeUChar, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet_va(mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet_va(mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(mask, nrrdAxisInfoMax, spcx * resx, spcy * resy, spcz * resz);
		nrrdAxisInfoSet_va(mask, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
	}
	vol_cache.m_mask = mask;
	vol_cache.m_valid &= (mask != 0);
	vol_cache.own_mask = vol_cache.m_valid;
	return mask != 0;
}

bool CQCallback::HandleLabel(VolCache4D& vol_cache)
{
	//get volume, readers
	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;
	auto reader = vd->GetReader();
	if (!reader)
		return false;

	int frame = static_cast<int>(vol_cache.m_tnum);
	int chan = vd->GetCurChannel();

	LBLReader lbl_reader;
	std::wstring lblname = reader->GetCurLabelName(frame, chan);
	lbl_reader.SetFile(lblname);
	Nrrd* label = lbl_reader.Convert(frame, chan, true);
	if (!label)
	{
		int resx, resy, resz;
		vd->GetResolution(resx, resy, resz);
		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);
		label = nrrdNew();
		unsigned long long mem_size = (unsigned long long)resx *
			(unsigned long long)resy * (unsigned long long)resz;
		unsigned int* val32 = new (std::nothrow) unsigned int[mem_size]();
		nrrdWrap_va(label, val32, nrrdTypeUInt, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet_va(label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet_va(label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(label, nrrdAxisInfoMax, spcx * resx, spcy * resy, spcz * resz);
		nrrdAxisInfoSet_va(label, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
	}
	vol_cache.m_label = label;
	vol_cache.m_valid &= (label != 0);
	vol_cache.own_label = vol_cache.m_valid;
	return label != 0;
}

bool CQCallback::AccessData(VolCache4D& vol_cache)
{
	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;

	Nrrd* data = vd->GetVolume(vol_cache.return_data);
	vol_cache.m_data = data;
	vol_cache.m_valid &= (data != 0);
	return data != 0;
}

bool CQCallback::AccessMask(VolCache4D& vol_cache)
{
	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;

	Nrrd* mask = vd->GetMask(vol_cache.return_mask);
	vol_cache.m_mask = mask;
	vol_cache.m_valid &= (mask != 0);
	return mask != 0;
}

bool CQCallback::AccessLabel(VolCache4D& vol_cache)
{
	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;

	Nrrd* label = vd->GetLabel(vol_cache.return_label);
	vol_cache.m_label = label;
	vol_cache.m_valid &= (label != 0);
	return label != 0;
}

bool CQCallback::SaveData(VolCache4D& vol_cache)
{
	return true;
}

bool CQCallback::SaveMask(VolCache4D& vol_cache)
{
	if (!vol_cache.m_valid || !vol_cache.m_modified)
		return false;

	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;
	auto reader = vd->GetReader();
	if (!reader)
		return false;

	int chan = vd->GetCurChannel();
	int frame = static_cast<int>(vol_cache.m_tnum);
	MSKWriter msk_writer;
	msk_writer.SetData((Nrrd*)vol_cache.GetNrrdMask());
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	msk_writer.SetSpacings(spcx, spcy, spcz);
	std::wstring filename = reader->GetCurMaskName(frame, chan);
	msk_writer.Save(filename, 0);
	return true;
}

bool CQCallback::SaveLabel(VolCache4D& vol_cache)
{
	if (!vol_cache.m_valid || !vol_cache.m_modified)
		return false;

	auto vd = vol_cache.m_vd.lock();
	if (!vd)
		return false;
	auto reader = vd->GetReader();
	if (!reader)
		return false;

	int chan = vd->GetCurChannel();
	int frame = static_cast<int>(vol_cache.m_tnum);
	MSKWriter msk_writer;
	msk_writer.SetData((Nrrd*)vol_cache.GetNrrdLabel());
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	msk_writer.SetSpacings(spcx, spcy, spcz);
	std::wstring filename = reader->GetCurLabelName(frame, chan);
	msk_writer.Save(filename, 1);
	return true;
}

bool CQCallback::BuildTex(VolCache4D& vol_cache)
{
	return true;
}

VolCache4D* CacheQueue::get_offset(int toffset)
{
	int cur_time = 0;
	auto view_ptr = glbin_current.render_view.lock();
	if (view_ptr)
		cur_time = view_ptr->m_tseq_cur_num;
	int t = std::max(0, cur_time + toffset);
	return get(static_cast<size_t>(t));
}

