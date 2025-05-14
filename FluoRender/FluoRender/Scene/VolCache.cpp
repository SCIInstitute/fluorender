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
#include <VolCache.h>
#include <Texture.h>
#include <Global.h>
#include <DataManager.h>
#include <MovieMaker.h>
#include <base_reader.h>
#include <msk_reader.h>
#include <lbl_reader.h>
#include <msk_writer.h>
#include <string>

using namespace flrd;

bool CQCallback::handle_data = false;
bool CQCallback::handle_mask = false;
bool CQCallback::handle_label = false;
bool CQCallback::access_data = false;
bool CQCallback::access_mask = false;
bool CQCallback::access_label = false;
bool CQCallback::return_data = false;
bool CQCallback::return_mask = false;
bool CQCallback::return_label = false;
bool CQCallback::save_data = false;
bool CQCallback::save_mask = false;
bool CQCallback::save_label = false;
bool CQCallback::build_tex = false;
bool CQCallback::time_cond0 = false;
bool CQCallback::cond0 = false;

void CQCallback::ReadVolCache(VolCache& vol_cache)
{
	if (!handle_data && !handle_label && !handle_mask &&
		!access_data && !access_mask && !access_label &&
		!build_tex)
		return;

	cond0 = false;
	if (time_cond0)
	{
		int cur_time = glbin_moviemaker.GetSeqCurNum();
		int frame = static_cast<int>(vol_cache.m_tnum);
		cond0 = cur_time == frame;
	}

	vol_cache.m_valid = true;
	if (handle_data)
	{
		if (cond0)
			AccessData(vol_cache, return_data);
		else
			HandleData(vol_cache);
	}
	if (handle_mask)
	{
		if (cond0)
			AccessMask(vol_cache, return_mask);
		else
			HandleMask(vol_cache);
	}
	if (handle_label)
	{
		if (cond0)
			AccessLabel(vol_cache, return_label);
		else
			HandleLabel(vol_cache);
	}
	if (access_data)
	{
		AccessData(vol_cache, return_data);
	}
	if (access_mask)
	{
		AccessMask(vol_cache, return_mask);
	}
	if (access_label)
	{
		AccessLabel(vol_cache, return_label);
	}
	if (build_tex)
	{
		BuildTex(vol_cache);
	}
}

void CQCallback::FreeVolCache(VolCache& vol_cache)
{
	if (!handle_data && !handle_label && !handle_mask &&
		!save_data && !save_mask && !save_label &&
		!build_tex)
		return;

	if (save_data)
	{
		SaveData(vol_cache);
	}
	if (save_mask)
	{
		SaveMask(vol_cache);
	}
	if (save_label)
	{
		SaveLabel(vol_cache);
	}
	if (handle_data && !cond0)
	{
		if (vol_cache.m_data)
		{
			nrrdNuke((Nrrd*)vol_cache.m_data);
			vol_cache.m_data = 0;
		}
	}
	if (handle_mask && !cond0)
	{
		if (vol_cache.m_mask)
		{
			nrrdNuke((Nrrd*)vol_cache.m_mask);
			vol_cache.m_mask = 0;
		}
	}
	if (handle_label && !cond0)
	{
		if (vol_cache.m_label)
		{
			nrrdNuke((Nrrd*)vol_cache.m_label);
			vol_cache.m_label = 0;
		}
	}
	vol_cache.m_valid = false;
}

void CQCallback::SetHandleFlags(int flags)
{
	handle_data = flags & HDL_DATA;
	handle_mask = flags & HDL_MASK;
	handle_label = flags & HDL_LABEL;
	access_data = flags & ACS_DATA;
	access_mask = flags & ACS_MASK;
	access_label = flags & ACS_LABEL;
	return_data = flags & RET_DATA;
	return_mask = flags & RET_MASK;
	return_label = flags & RET_LABEL;
	build_tex = flags & BLD_TEX;
	time_cond0 = flags & TIME_COND0;
}

bool CQCallback::HandleData(VolCache& vol_cache)
{
	//get volume, readers
	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return false;

	int frame = static_cast<int>(vol_cache.m_tnum);
	int chan = cur_vol->GetCurChannel();

	Nrrd* data = 0;
	data = reader->Convert(frame, chan, true);
	vol_cache.m_data = data;
	vol_cache.m_valid &= (data != 0);
	return data != 0;
}

bool CQCallback::HandleMask(VolCache& vol_cache)
{
	//get volume, readers
	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return false;

	int frame = static_cast<int>(vol_cache.m_tnum);
	int chan = cur_vol->GetCurChannel();

	MSKReader msk_reader;
	std::wstring mskname = reader->GetCurMaskName(frame, chan);
	msk_reader.SetFile(mskname);
	Nrrd* mask = msk_reader.Convert(frame, chan, true);
	if (!mask)
	{
		int resx, resy, resz;
		cur_vol->GetResolution(resx, resy, resz);
		double spcx, spcy, spcz;
		cur_vol->GetSpacings(spcx, spcy, spcz);
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
	return mask != 0;
}

bool CQCallback::HandleLabel(VolCache& vol_cache)
{
	//get volume, readers
	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return false;

	int frame = static_cast<int>(vol_cache.m_tnum);
	int chan = cur_vol->GetCurChannel();

	LBLReader lbl_reader;
	std::wstring lblname = reader->GetCurLabelName(frame, chan);
	lbl_reader.SetFile(lblname);
	Nrrd* label = lbl_reader.Convert(frame, chan, true);
	if (!label)
	{
		int resx, resy, resz;
		cur_vol->GetResolution(resx, resy, resz);
		double spcx, spcy, spcz;
		cur_vol->GetSpacings(spcx, spcy, spcz);
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
	return label != 0;
}

bool CQCallback::AccessData(VolCache& vol_cache, bool ret)
{
	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;

	Nrrd* data = cur_vol->GetVolume(ret);
	vol_cache.m_data = data;
	vol_cache.m_valid &= (data != 0);
	return data != 0;
}

bool CQCallback::AccessMask(VolCache& vol_cache, bool ret)
{
	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;

	Nrrd* mask = cur_vol->GetMask(ret);
	vol_cache.m_mask = mask;
	vol_cache.m_valid &= (mask != 0);
	return mask != 0;
}

bool CQCallback::AccessLabel(VolCache& vol_cache, bool ret)
{
	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;

	Nrrd* label = cur_vol->GetLabel(ret);
	vol_cache.m_label = label;
	vol_cache.m_valid &= (label != 0);
	return label != 0;
}

bool CQCallback::SaveData(VolCache& vol_cache)
{
	return true;
}

bool CQCallback::SaveMask(VolCache& vol_cache)
{
	if (!vol_cache.m_valid || !vol_cache.m_modified)
		return false;

	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return false;

	int chan = cur_vol->GetCurChannel();
	int frame = static_cast<int>(vol_cache.m_tnum);
	MSKWriter msk_writer;
	msk_writer.SetData((Nrrd*)vol_cache.GetNrrdMask());
	double spcx, spcy, spcz;
	cur_vol->GetSpacings(spcx, spcy, spcz);
	msk_writer.SetSpacings(spcx, spcy, spcz);
	std::wstring filename = reader->GetCurMaskName(frame, chan);
	msk_writer.Save(filename, 0);
	return true;
}

bool CQCallback::SaveLabel(VolCache& vol_cache)
{
	if (!vol_cache.m_valid || !vol_cache.m_modified)
		return false;

	VolumeData* cur_vol = glbin_current.vol_data;
	if (!cur_vol)
		return false;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return false;

	int chan = cur_vol->GetCurChannel();
	int frame = static_cast<int>(vol_cache.m_tnum);
	MSKWriter msk_writer;
	msk_writer.SetData((Nrrd*)vol_cache.GetNrrdLabel());
	double spcx, spcy, spcz;
	cur_vol->GetSpacings(spcx, spcy, spcz);
	msk_writer.SetSpacings(spcx, spcy, spcz);
	std::wstring filename = reader->GetCurLabelName(frame, chan);
	msk_writer.Save(filename, 1);
	return true;
}

bool CQCallback::BuildTex(VolCache& vol_cache)
{
	return true;
}

