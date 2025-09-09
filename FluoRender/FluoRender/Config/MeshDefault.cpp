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

#include <MeshDefault.h>
#include <DataManager.h>
#include <Global.h>
#include <Names.h>
#include <BaseConvVolMesh.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>

MeshDefault::MeshDefault()
{
}

MeshDefault::~MeshDefault()
{
}

void MeshDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	if (f->Exists("/mesh default"))
		f->SetPath("/mesh default");

	f->Read(gstUseTransferFunc, &m_use_transfer, false);
	f->Read(gstUseSelection, &m_use_mask, true);
	f->Read(gstVolMeshThresh, &m_iso, 0.5);
	f->Read(gstVolMeshDownXY, &m_downsample, 1);
	f->Read(gstVolMeshDownZ, &m_downsample_z, 1);
	f->Read(gstVolMeshWeld, &m_vertex_merge, false);
	f->Read(gstVolMeshSimplify, &m_simplify, 0.0);
	f->Read(gstVolMeshSmooth, &m_smooth, 0.0);

}

void MeshDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/mesh default");

	f->Write(gstUseTransferFunc, m_use_transfer);
	f->Write(gstUseSelection, m_use_mask);
	f->Write(gstVolMeshThresh, m_iso);
	f->Write(gstVolMeshDownXY, m_downsample);
	f->Write(gstVolMeshDownZ, m_downsample_z);
	f->Write(gstVolMeshWeld, m_vertex_merge);
	f->Write(gstVolMeshSimplify, m_simplify);
	f->Write(gstVolMeshSmooth, m_smooth);
}

void MeshDefault::Set(MeshData* md)
{
	if (!md)
		return;
}

void MeshDefault::Set(flrd::BaseConvVolMesh* cvm)
{
	if (!cvm)
		return;
	m_use_transfer = cvm->GetUseTransfer();
	m_use_mask = cvm->GetUseMask();
	m_iso = cvm->GetIsoValue();
	m_downsample = cvm->GetDownsample();
	m_downsample_z = cvm->GetDownsampleZ();
	m_vertex_merge = cvm->GetVertexMerge();
	m_simplify = cvm->GetSimplify();
	m_smooth = cvm->GetSmooth();
}

void MeshDefault::Apply(MeshData* md)
{
	if (!md)
		return;
}

void MeshDefault::Copy(MeshData* m1, MeshData* m2)//m2 to m1
{
	if (!m1 || !m2)
		return;
}

void MeshDefault::Apply(MeshGroup* g)
{
	if (!g)
		return;
}

void MeshDefault::Apply(flrd::BaseConvVolMesh* cvm)
{
	if (!cvm)
		return;

	cvm->SetUseTransfer(m_use_transfer);
	cvm->SetUseMask(m_use_mask);
	cvm->SetIsoValue(m_iso);
	cvm->SetDownsample(m_downsample);
	cvm->SetDownsampleZ(m_downsample_z);
	cvm->SetVertexMerge(m_vertex_merge);
	cvm->SetSimplify(m_simplify);
	cvm->SetSmooth(m_smooth);
}