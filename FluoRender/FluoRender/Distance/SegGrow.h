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
#ifndef FL_SegGrow_h
#define FL_SegGrow_h

#include <Point.h>
#include <set>
#include <unordered_map>
#include <memory>

class VolumeData;
namespace flvr
{
	class TextureBrick;
	class KernelProgram;
	class Argument;
}
namespace flrd
{
	class RulerHandler;
	struct BranchPoint
	{
		unsigned int id;
		unsigned int sum;
		fluo::Point ctr;
		std::set<unsigned int> cid;//parent
		std::set<unsigned int> bid;//ids from multiple bricks
	};

	class SegGrow
	{
	public:
		SegGrow();
		~SegGrow();

		void SetVolumeData(VolumeData* vd);
		void SetBranches(int val) { m_branches = val; }
		void SetIter(int val) { m_iter = val; }
		void SetSizeThresh(int size) { m_sz_thresh = size; }
		void SetRulerHandler(RulerHandler* handler);
		void Compute();

	private:
		VolumeData *m_vd;
		RulerHandler *m_handler;
		int m_branches;//max number of branches to detect
		int m_iter;//iterations
		int m_sz_thresh;//threshold for exclude small comps
		std::unordered_map<unsigned int, BranchPoint> m_list;

		bool CheckBricks();
		bool CheckBrickPair(unsigned int id1, unsigned int id2,
			std::vector<std::set<unsigned int>> &pairs);
		void CollectIds(std::vector<unsigned int> &ids,
			std::vector<unsigned int> &cids,
			std::vector<std::set<unsigned int>> &merge_list);
		void MergeIds(std::vector<std::set<unsigned int>> &merge_list);
		void CheckBorders(int d0, int d1, int n0, int n1,
			std::vector<unsigned int> &ids,
			flvr::TextureBrick* nb,
			flvr::KernelProgram *kernel_prog, int kernel,
			std::weak_ptr<flvr::Argument> arg_tex,
			std::vector<std::set<unsigned int>> &brick_pairs,
			std::vector<std::set<unsigned int>> &merge_list);
	};
}
#endif//FL_SegGrow_h