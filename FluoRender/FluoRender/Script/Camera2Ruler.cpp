/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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

#include "Camera2Ruler.h"
#include <Distance/RulerHandler.h>
#include <opencv2/opencv.hpp>
#include <wx/wfstream.h>
#include <wx/fileconf.h>

using namespace flrd;

Camera2Ruler::Camera2Ruler() :
	m_del_list1(false),
	m_del_list2(false),
	m_list1(0),
	m_list2(0),
	m_list_out(0)
{

}

Camera2Ruler::~Camera2Ruler()
{
	if (m_del_list1)
	{
		m_list1->DeleteRulers();
		delete m_list1;
	}
	if (m_del_list2)
	{
		m_list2->DeleteRulers();
		delete m_list2;
	}
}

void Camera2Ruler::SetList(int i, RulerList* list)
{
	if (i == 1)
		m_list1 = list;
	else if (i == 2)
		m_list2 = list;
}

void Camera2Ruler::SetList(int i, const std::string& config)
{
	RulerList* list = 0;
	wxFileInputStream is(config);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//views
	if (fconfig.Exists("/views"))
	{
		fconfig.SetPath("/views");
		int num = fconfig.Read("num", 0l);
		if (num < 1)
			return;

		if (fconfig.Exists("/views/0/rulers"))
		{
			fconfig.SetPath("/views/0/rulers");
			RulerHandler handler;
			list = new RulerList;
			handler.SetRulerList(list);
			handler.Read(fconfig, 0);
		}
	}

	if (list)
	{
		if (i == 1)
		{
			if (m_del_list1)
				delete m_list1;
			m_list1 = list;
			m_del_list1 = true;
		}
		else if (i == 2)
		{
			if (m_del_list2)
				delete m_list2;
			m_list2 = list;
			m_del_list2 = true;
		}
	}
}

void Camera2Ruler::Run()
{
	if (!m_list1 || !m_list2)
		return;
}

