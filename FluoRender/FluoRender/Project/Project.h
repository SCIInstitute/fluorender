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

#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <Progress.h>
#include <wx/fileconf.h>
#include <string>

class Project : public Progress
{
public:
	Project();
	~Project();

	void Open(const std::wstring& filename);
	void Save(const std::wstring& filename, bool inc);//inc: save incrementally
	void Reset();

	void ExportRulerList(const std::wstring& filename);
	void SaveRulerList(const std::string& gst_name, int vi);
	void ReadRulerList(const std::string& gst_name, int vi);

	void SaveConfig(wxFileConfig& fconfig, const std::wstring& filename);

private:
};

#endif//_PROJECT_H_
