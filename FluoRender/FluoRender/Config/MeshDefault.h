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
#ifndef _MESHDEFAULT_H_
#define _MESHDEFAULT_H_

class MeshData;
class MeshGroup;
namespace flrd
{
	class BaseConvVolMesh;
}
class MeshDefault
{
public:
	MeshDefault();
	~MeshDefault();

	void Read();
	void Save();

	void Set(MeshData* md);
	void Set(flrd::BaseConvVolMesh* cvm);
	void Apply(MeshData* md);
	void Copy(MeshData* m1, MeshData* m2);//m2 to m1
	void Apply(MeshGroup* g);
	void Apply(flrd::BaseConvVolMesh* cvm);

public:
	//mesh conversion settings
	bool m_use_transfer = false; // Use transfer function
	bool m_use_mask = true; // Use mask for volume data
	double m_iso = 0.5; // Iso value for contouring
	int m_downsample = 1; // Downsampling factor in x and y
	int m_downsample_z = 1; // Downsampling factor in z
	double m_simplify = 0.0; // Simplification factor
	double m_smooth_n = 0.0; // Smoothing factor in normal direction
	double m_smooth_t = 0.0; // smoothing factor in tangential direction
};

#endif//_MESHDEFAULT_H_