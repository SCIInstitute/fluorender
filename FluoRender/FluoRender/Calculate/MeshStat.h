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
#ifndef FL_MeshStat_h
#define FL_MeshStat_h

#include <Progress.h>

class MeshData;
namespace flrd
{
	class MeshStat : public Progress
	{
	public:
		MeshStat(MeshData* md);
		virtual ~MeshStat() {}

		void Run();

		//get results
		int GetVertexNum() { return m_vertex_num; }
		int GetTriangleNum() { return m_triangle_num; }
		int GetNormalNum() { return m_normal_num; }
		double GetArea() { return m_area; }
		double GetVolume() { return m_volume; }

	private:
		MeshData *m_md;
		//result
		int m_vertex_num;
		int m_triangle_num;
		int m_normal_num;
		double m_area;
		double m_volume;
	};

}
#endif//FL_MeshStat_h
