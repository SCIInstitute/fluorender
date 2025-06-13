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

#ifndef _UNDOABLEAMASK_H
#define _UNDOABLEAMASK_H

#include <Undoable.h>
#include <memory>
#include <vector>

class VolumeData;
class UndoableMask : public Undoable
{
public:
	UndoableMask(const std::shared_ptr<VolumeData>& vd) :
		m_vd(vd)
	{ }
	~UndoableMask() {}

	bool trim_mask_undos_head();
	bool trim_mask_undos_tail();
	bool get_undo();
	bool get_redo();
	void set_mask(void* mask_data);
	void push_mask();
	void pop_mask();
	void mask_undos_forward();
	void mask_undos_backward();
	void clear_undos();

private:
	std::weak_ptr<VolumeData> m_vd;
	std::vector<void*> mask_undos_;
	int mask_undo_pointer_ = -1;

};

#endif//_UNDOABLEAMASK_H