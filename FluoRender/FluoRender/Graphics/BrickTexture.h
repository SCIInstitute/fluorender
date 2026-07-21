//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#ifndef BrickTexture_h
#define BrickTexture_h

#include <memory>

namespace flvr
{
	class Brick;
	class Texture;

	class BrickTexture : public Texture
	{
	public:
		BrickTexture(
			const TextureDesc& desc,
			std::weak_ptr<Brick> brick,
			CompType comp);

		virtual ~BrickTexture() = default;

		std::shared_ptr<Brick> brick() const
		{
			return brick_.lock();
		}

		CompType component() const
		{
			return comp_;
		}

		void mark_used(uint64_t frame)
		{
			last_used_frame_ = frame;
		}

		uint64_t last_used_frame() const
		{
			return last_used_frame_;
		}

	private:
		std::weak_ptr<Brick> brick_;
		CompType comp_;

		uint64_t last_used_frame_ = 0;
	};

}

#endif// BrickTexture_h