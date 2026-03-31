/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <msk_writer.h>
#include <nrrd_utility.h>
#include <sstream>
#include <inttypes.h>

MSKWriter::MSKWriter()
{
	m_data = 0;
	m_use_spacings = false;
	m_time = 0;
	m_channel = 0;
}

MSKWriter::~MSKWriter()
{
}

void MSKWriter::SetData(const std::shared_ptr<fluo::RawData>& data)
{
	m_data = data;
}

void MSKWriter::SetSpacing(const fluo::Vector& spc)
{
	m_spc = spc;
	m_use_spacings = true;
}

void MSKWriter::SetCompression(bool value)
{
}

void MSKWriter::Save(const std::wstring& filename, int mode)
{
	if (!m_data || !m_data->IsAllocated())
		return;

	// Convert filename to UTF-8 / narrow string
	std::string path(filename.begin(), filename.end());

	// --- Create a TEMPORARY NRRD ---
	Nrrd* nrrd = nrrdNew();

	const auto& size = m_data->GetSize();

	int nrrd_type = ToNrrdType(m_data->GetFormat());
	if (nrrd_type == nrrdTypeUnknown)
	{
		nrrdNuke(nrrd);
		return;
	}

	// Wrap RawData memory (NRRD DOES NOT OWN IT)
	nrrdWrap_va(
		nrrd,
		const_cast<void*>(static_cast<const void*>(m_data->GetData())),
		nrrd_type,
		3,
		size[0],
		size[1],
		size[2]
	);

	// Explicitly tell NRRD it does NOT own the memory
	//nrrd->dataOwn = nrrdDataOwnNone;

	// --- Axis / spacing info (interpretation layer) ---
	if (m_use_spacings)
	{
		nrrdAxisInfoSet_va(
			nrrd,
			nrrdAxisInfoSpacing,
			m_spc.x(),
			m_spc.y(),
			m_spc.z()
		);

		nrrdAxisInfoSet_va(
			nrrd,
			nrrdAxisInfoMin,
			0.0, 0.0, 0.0
		);

		nrrdAxisInfoSet_va(
			nrrd,
			nrrdAxisInfoMax,
			m_spc.x() * size[0],
			m_spc.y() * size[1],
			m_spc.z() * size[2]
		);
	}

	// --- Save ---
	nrrdSave(path.c_str(), nrrd, nullptr);

	// --- Destroy temporary NRRD ---
	nrrdNuke(nrrd);
}
void MSKWriter::SetTC(int t, int c)
{
	m_time = t;
	m_channel = c;
}
