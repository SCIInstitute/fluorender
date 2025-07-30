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
#include <File.h>
#include <compatibility.h>

using namespace flrd;

File::File() :
	mode_(0)
{

}

File::~File()
{

}

void File::beginWrite(const std::wstring& filename)
{
#ifdef _WIN32
	ofs_ = std::ofstream(filename, std::ios::out | std::ios::binary);
#else
    ofs_ = std::ofstream(ws2s(filename), std::ios::out | std::ios::binary);
#endif
	if (ofs_.bad()) return;
	mode_ = 1;
}

void File::endWrite()
{
	ofs_.close();
	mode_ = 0;
}

void File::beginRead(const std::wstring& filename)
{
#ifdef _WIN32
	ifs_ = std::ifstream(filename, std::ios::out | std::ios::binary);
#else
    ifs_ = std::ifstream(ws2s(filename), std::ios::out | std::ios::binary);
#endif
	if (ifs_.bad()) return;
	mode_ = 2;
}

void File::endRead()
{
	ifs_.close();
	mode_ = 0;
}

void File::getPos()
{
	switch (mode_)
	{
	case 1:
		break;
	case 2:
		pos_ = ifs_.tellg();
		break;
	default:
		break;
	}
}

void File::setPos()
{
	switch (mode_)
	{
	case 1:
		break;
	case 2:
		ifs_.seekg(pos_);
		break;
	default:
		break;
	}
}

void File::writeWstring(const std::wstring& s)
{
	if (ofs_.bad()) return;
	std::string str = ws2s(s);
	ofs_.write(str.c_str(), s.size());
}
