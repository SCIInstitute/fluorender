/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef ANNOTATIONS_HPP
#define ANNOTATIONS_HPP

#include <SceneGraph/Node/Node.hpp>
#include <string>
#include <vector>

namespace fluo
{
	class AText
	{
	public:
		AText() {}
		AText(const std::string &str, const FLTYPE::Point &pos) { m_txt = str; m_pos = pos; }
		~AText() {}

		std::string GetText() { return m_txt; };
		FLTYPE::Point GetPos() { return m_pos; };
		void SetText(std::string str) { m_txt = str; }
		void SetPos(FLTYPE::Point pos) { m_pos = pos; }
		void SetInfo(std::string str) { m_info = str; }

		friend class Annotations;

	private:
		std::string m_txt;
		FLTYPE::Point m_pos;
		std::string m_info;
	};

	class Annotations : public Node
	{
	public:
		Annotations();
		Annotations(const Annotations& data, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new Annotations(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const Annotations*>(obj) != NULL;
		}

		virtual const char* className() const { return "Annotations"; }

		virtual Annotations* asAnnotations() { return this; }
		virtual const Annotations* asAnnotations() const { return this; }

	protected:
		virtual ~Annotations();

	private:
		std::vector<AText*> m_alist;
	};
}

#endif//_ANNOTATIONS_H_
