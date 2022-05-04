/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <AnnotationPropAgent.hpp>
#include <AnnotationPropPanel.h>

using namespace fluo;

AnnotationPropAgent::AnnotationPropAgent(AnnotationPropPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
	setupInputs();
}

void AnnotationPropAgent::setupInputs()
{
	inputs_ = ValueCollection
	{
		Memo,
		MemoRo
	};
}

void AnnotationPropAgent::setObject(Annotations* obj)
{
	InterfaceAgent::setObject(obj);
}

Annotations* AnnotationPropAgent::getObject()
{
	return dynamic_cast<Annotations*>(InterfaceAgent::getObject());
}

void AnnotationPropAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();

	//memo
	if (update_all || FOUND_VALUE(Memo))
	{
		std::string memo;
		getValue(Memo, memo);
		panel_.m_memo_text->SetValue(memo);
		bool bval;
		getValue(MemoRo, bval);
		if (bval)
		{
			panel_.m_memo_text->SetEditable(false);
			panel_.m_memo_update_btn->Disable();
		}
		else
		{
			panel_.m_memo_text->SetEditable(true);
			panel_.m_memo_update_btn->Enable();
		}
	}
}
