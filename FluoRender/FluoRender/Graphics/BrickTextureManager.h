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

#ifndef BrickTextureManager_h
#define BrickTextureManager_h

#include <memory>
#include <unordered_map>

namespace flvr
{
	class Brick;
	class BrickTexture;

	struct BrickTextureKey
	{
		uint32_t brick_id;
		CompType comp;

		bool operator==(const BrickTextureKey& rhs) const
		{
			return brick_id == rhs.brick_id &&
				comp == rhs.comp;
		}
	};

	struct BrickTextureKeyHash
	{
		size_t operator()(
			const BrickTextureKey& k) const
		{
			return std::hash<uint32_t>()(k.brick_id) ^
				(std::hash<int>()(
					static_cast<int>(k.comp)) << 1);
		}
	};

	class BrickTextureManager
	{
	public:
		static BrickTextureManager& instance();

		std::shared_ptr<BrickTexture>
			find(
				uint32_t brick_id,
				CompType comp);

		std::shared_ptr<BrickTexture>
			acquire(
				const std::shared_ptr<Brick>& brick,
				CompType comp);

		void release(
			uint32_t brick_id,
			CompType comp);

		void clear();

		size_t size() const
		{
			return textures_.size();
		}

	private:
		BrickTextureManager() = default;

	private:
		std::unordered_map<
			BrickTextureKey,
			std::shared_ptr<BrickTexture>,
			BrickTextureKeyHash> textures_;
	};
}

#endif// BrickTextureManager_h