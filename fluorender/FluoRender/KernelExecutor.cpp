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
#include "KernelExecutor.h"
#include <wx\wfstream.h>
#include <wx\txtstrm.h>

KernelExecutor::KernelExecutor()
	: m_vd(0),
	m_vd_r(0)
{
}

KernelExecutor::~KernelExecutor()
{
}

void KernelExecutor::SetCode(wxString &code)
{
	m_code = code;
}

void KernelExecutor::LoadCode(wxString &filename)
{
	if (!wxFileExists(filename))
	{
		m_message = "Kernel file " +
			filename + " doesn't exist.\n";
		return;
	}
	wxFileInputStream input(filename);
	wxTextInputStream cl_file(input);
	if (!input.IsOk())
	{
		m_message = "Kernel file " +
			filename + " reading failed.\n";
		return;
	}
	m_code = "";
	while (!input.Eof())
		m_code += cl_file.ReadLine();
	m_message = "Kernel file " +
		filename + " read.\n";
}

void KernelExecutor::SetVolume(VolumeData *vd)
{
	m_vd = vd;
}

VolumeData* KernelExecutor::GetVolume()
{
	return m_vd;
}

VolumeData* KernelExecutor::GetResult()
{
	return m_vd_r;
}

bool KernelExecutor::GetMessage(wxString &msg)
{
	if (m_message == "")
		return false;
	else
	{
		msg = m_message;
		return true;
	}
}

void KernelExecutor::Execute()
{

}

bool KernelExecutor::ExecuteKernel(KernelProgram* kernel,
	GLuint data_id, void* result,
	size_t brick_x, size_t brick_y,
	size_t brick_z)
{
	return true;
}
