/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#ifndef FL_CompGenerator_h
#define FL_CompGenerator_h

#ifdef _WIN32
#include <CL/cl.h>
#endif
#ifdef _DARWIN
#include <OpenCL/cl.h>
#endif
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>

using namespace std;

class VolumeData;
namespace FL
{
	class ComponentGenerator
	{
	public:
		ComponentGenerator(VolumeData* vd, int device_id = 0);
		~ComponentGenerator();

		void OrderID_3D();
		void OrderID_2D();
		void ShuffleID_3D();
		void ShuffleID_2D();
		void InitialGrow(bool, int,
			float, float,
			float, float,
			float, float,
			float, float,
			float, float);
		void SizedGrow(bool, int,
			unsigned int, unsigned int,
			float, float,
			float, float,
			float, float,
			float, float,
			float, float);
		void Cleanup(int, unsigned int);
		void MatchSlices_CPU(unsigned int,
			float, float, float);

		//update progress
		boost::signals2::signal<void ()> m_sig_progress;

	private:
		VolumeData *m_vd;
		bool m_init;
		cl_device_id m_device;
		cl_context m_context;

		struct Cell;
		struct Edge
		{
			Cell* cell1;
			Cell* cell2;
			unsigned int sizei;//voxel count
			float size;//overlapping size
			float x;//x of overlapping center
			float y;//y of overlapping center
		};
		struct Cell
		{
			~Cell()
			{
				for (size_t i = 0; i<edges.size(); ++i)
					delete edges[i];
			}
			unsigned int id;
			unsigned int sizei;//voxel count
			float size;
			float x;//x of cell center
			float y;//y of cell center
			vector<Edge*> edges;
		};
		typedef boost::unordered_map<unsigned int, Cell*> CellList;
		typedef boost::unordered_map<unsigned int, Cell*>::iterator CellListIter;

		typedef boost::unordered_map<unsigned int, Edge*> ReplaceList;
		typedef boost::unordered_map<unsigned int, Edge*>::iterator ReplaceListIter;

	private:
		unsigned int reverse_bit(unsigned int val, unsigned int len);
		void InitializeCellList(unsigned int* page, void* page_data,
			int bits, float sscale, size_t nx, size_t ny,
			CellList* cell_list);
		void ClearCellList(CellList* cell_list);
		static bool sort_ol(const Edge* e1, const Edge* e2);
	};

	inline unsigned int ComponentGenerator::reverse_bit(unsigned int val, unsigned int len)
	{
		unsigned int res = val;
		int s = len - 1;

		for (val >>= 1; val; val >>= 1)
		{
			res <<= 1;
			res |= val & 1;
			s--;
		}
		res <<= s;
		res <<= 32 - len;
		res >>= 32 - len;
		return res;
	}

	inline void ComponentGenerator::InitializeCellList(unsigned int* page,
		void* page_data, int bits, float sscale, size_t nx, size_t ny,
		CellList* cell_list)
	{
		unsigned int index;
		unsigned int label_value;
		float data_value;
		CellListIter iter;

		for (size_t i = 0; i<nx; ++i)
		for (size_t j = 0; j<ny; ++j)
		{
			index = nx*j + i;
			label_value = page[index];
			if (bits == 8)
				data_value = ((unsigned char*)page_data)[index] / 255.0f;
			else if (bits == 16)
				data_value = ((unsigned short*)page_data)[index] / 65535.0f * sscale;
			if (label_value)
			{
				iter = cell_list->find(label_value);
				if (iter != cell_list->end())
				{
					iter->second->x = (iter->second->x *
						iter->second->sizei + i) /
						(iter->second->sizei + 1);
					iter->second->y = (iter->second->y *
						iter->second->sizei + j) /
						(iter->second->sizei + 1);
					iter->second->sizei++;
					iter->second->size += data_value;
				}
				else
				{
					Cell* cell = new Cell();
					cell->id = label_value;
					cell->sizei = 1;
					cell->size = data_value;
					cell->x = i;
					cell->y = j;
					cell_list->insert(pair<unsigned int, Cell*>(label_value, cell));
				}
			}
		}
	}

	inline void ComponentGenerator::ClearCellList(CellList* cell_list)
	{
		CellListIter iter;
		for (iter = cell_list->begin(); iter != cell_list->end(); ++iter)
			delete iter->second;
		cell_list->clear();
	}

	inline bool ComponentGenerator::sort_ol(const Edge* e1, const Edge* e2)
	{
		float e1_size1 = e1->cell1->size;
		float e1_size2 = e1->cell2->size;
		float e1_size_ol = e1->size;
		float e2_size1 = e2->cell1->size;
		float e2_size2 = e2->cell2->size;
		float e2_size_ol = e2->size;
		return e1_size_ol / e1_size1 + e1_size_ol / e1_size2 >
			e2_size_ol / e2_size1 + e2_size_ol / e2_size2;
	}

}
#endif//FL_CompGenerator_h