//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <UndoableMask.h>
#include <Global.h>
#include <BrushDefault.h>
#include <VolumeData.h>
#include <Texture.h>

bool UndoableMask::trim_mask_undos_head()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return true;
	auto tex = vd->GetTexture();
	if (!tex)
		return false;
	int nmask = tex->nmask();
	if (nmask < 0)
		return true;
	if (mask_undos_.size() <= undo_num + 1)
		return true;
	if (mask_undo_pointer_ == 0)
		return false;
	while (mask_undos_.size() > undo_num + 1 &&
		mask_undo_pointer_ > 0 &&
		mask_undo_pointer_ < mask_undos_.size())
	{
		delete[](unsigned char*)(mask_undos_.front());
		mask_undos_.erase(mask_undos_.begin());
		mask_undo_pointer_--;
	}
	return true;
}

bool UndoableMask::trim_mask_undos_tail()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return true;
	auto tex = vd->GetTexture();
	if (!tex)
		return false;
	int nmask = tex->nmask();
	if (nmask < 0)
		return true;
	if (mask_undos_.size() <= undo_num + 1)
		return true;
	if (mask_undo_pointer_ == mask_undos_.size() - 1)
		return false;
	while (mask_undos_.size() > undo_num + 1 &&
		mask_undo_pointer_ >= 0 &&
		mask_undo_pointer_ < mask_undos_.size() - 1)
	{
		delete[](unsigned char*)(mask_undos_.back());
		mask_undos_.pop_back();
	}
	return true;
}

bool UndoableMask::get_undo()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return false;
	auto tex = vd->GetTexture();
	if (!tex)
		return false;
	int nmask = tex->nmask();
	if (nmask < 0)
		return false;
	if (mask_undo_pointer_ <= 0)
		return false;
	return true;
}

bool UndoableMask::get_redo()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return false;
	auto tex = vd->GetTexture();
	if (!tex)
		return false;
	int nmask = tex->nmask();
	if (nmask < 0)
		return false;
	if (mask_undo_pointer_ >= mask_undos_.size() - 1)
		return false;
	return true;
}

void UndoableMask::set_mask(void* mask_data)
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return;
	auto tex = vd->GetTexture();
	if (!tex)
		return;
	int nmask = tex->nmask();
	if (nmask < 0)
		return;

	if (mask_undo_pointer_ > -1 &&
		mask_undo_pointer_ < mask_undos_.size() - 1)
	{
		mask_undos_.insert(
			mask_undos_.begin() + mask_undo_pointer_ + 1,
			mask_data);
		mask_undo_pointer_++;
		if (!trim_mask_undos_head())
			trim_mask_undos_tail();
	}
	else
	{
		mask_undos_.push_back(mask_data);
		mask_undo_pointer_ = static_cast<int>(mask_undos_.size()) - 1;
		trim_mask_undos_head();
	}
}

void UndoableMask::push_mask()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return;
	auto tex = vd->GetTexture();
	if (!tex)
		return;
	int nmask = tex->nmask();
	if (nmask < 0)
		return;
	if (mask_undo_pointer_<0 ||
		mask_undo_pointer_>mask_undos_.size() - 1)
		return;

	int nx = tex->nx();
	int ny = tex->ny();
	int nz = tex->nz();
	//duplicate at pointer position
	unsigned long long mem_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	void* new_data = (void*)new (std::nothrow) unsigned char[mem_size];
	memcpy(new_data, mask_undos_[mask_undo_pointer_], size_t(mem_size));
	if (mask_undo_pointer_ < mask_undos_.size() - 1)
	{
		mask_undos_.insert(
			mask_undos_.begin() + mask_undo_pointer_ + 1,
			new_data);
		mask_undo_pointer_++;
		if (!trim_mask_undos_head())
			trim_mask_undos_tail();
	}
	else
	{
		mask_undos_.push_back(new_data);
		mask_undo_pointer_++;
		trim_mask_undos_head();
	}

	//#ifdef _DEBUG
	//		DBMIUINT8 img;
	//		img.nx = nx_; img.ny = ny_; img.nc = 1; img.nt = img.nx;
	//		img.data = (unsigned char*)(mask_undos_[0]);
	//		DBMIUINT8 img2;
	//		img2.nx = nx_; img2.ny = ny_; img2.nc = 1; img2.nt = img2.nx;
	//		img2.data = (unsigned char*)(mask_undos_[1]);
	//#endif
	//update mask data
	//nrrdWrap_va(data_[nmask],
	//	mask_undos_[mask_undo_pointer_],
	//	nrrdTypeUChar, 3, (size_t)nx,
	//	(size_t)ny, (size_t)nz);
}

void UndoableMask::pop_mask()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return;
	auto tex = vd->GetTexture();
	if (!tex)
		return;
	int nmask = tex->nmask();
	if (nmask < 0)
		return;
	if (mask_undo_pointer_ <= 0 ||
		mask_undo_pointer_ > mask_undos_.size() - 1)
		return;

	delete[](unsigned char*)(mask_undos_.back());
	mask_undos_.pop_back();
	mask_undo_pointer_--;

	//update mask data
	//nrrdWrap_va(data_[nmask_],
	//	mask_undos_[mask_undo_pointer_],
	//	nrrdTypeUChar, 3, (size_t)nx_,
	//	(size_t)ny_, (size_t)nz_);
}

void UndoableMask::mask_undos_backward()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return;
	auto tex = vd->GetTexture();
	if (!tex)
		return;
	int nmask = tex->nmask();
	if (nmask < 0)
		return;
	if (mask_undo_pointer_ <= 0 ||
		mask_undo_pointer_ > mask_undos_.size() - 1)
		return;

	//move pointer
	mask_undo_pointer_--;

	//update mask data
	//nrrdWrap_va(data_[nmask_],
	//	mask_undos_[mask_undo_pointer_],
	//	nrrdTypeUChar, 3, (size_t)nx_,
	//	(size_t)ny_, (size_t)nz_);
}

void UndoableMask::mask_undos_forward()
{
	size_t undo_num = glbin_brush_def.m_paint_hist_depth;
	auto vd = m_vd.lock();
	if (!vd || undo_num == 0)
		return;
	auto tex = vd->GetTexture();
	if (!tex)
		return;
	int nmask = tex->nmask();
	if (nmask < 0)
		return;
	if (mask_undo_pointer_<0 ||
		mask_undo_pointer_>mask_undos_.size() - 2)
		return;

	//move pointer
	mask_undo_pointer_++;

	//update mask data
	//nrrdWrap_va(data_[nmask_],
	//	mask_undos_[mask_undo_pointer_],
	//	nrrdTypeUChar, 3, (size_t)nx_,
	//	(size_t)ny_, (size_t)nz_);
}

void UndoableMask::clear_undos()
{
	//mask data now managed by the undos
	for (size_t i = 0; i < mask_undos_.size(); ++i)
	{
		if (mask_undos_[i])
			delete[](unsigned char*)(mask_undos_[i]);
	}
	mask_undos_.clear();
	mask_undo_pointer_ = -1;
}

