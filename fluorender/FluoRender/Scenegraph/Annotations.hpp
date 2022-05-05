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

#ifndef ANNOTATIONS_HPP
#define ANNOTATIONS_HPP

#include <Node.hpp>
#include <Names.hpp>
#include <FLIVR/TextRenderer.h>
#include <string>
#include <vector>

namespace fluo
{
	struct Atext
	{
		Point m_pos;
		std::string m_txt;
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

		//load
		int LoadData(const std::wstring &filename);
		void SaveData(const std::wstring &filename);

		//text
		void addText(const fluo::Point &pos, const std::string &str, const std::string &info);

		//draw
		void Draw(int nx, int ny, Transform &mv, Transform &p, bool persp);

	protected:
		virtual ~Annotations();

	private:
		std::vector<Atext> alist_;
		flvr::TextRenderer m_text_renderer;

	private:
		Atext buildAtext(const std::string str);
		bool insideClippingPlanes(Point &pos);
	};
}

#endif//_ANNOTATIONS_H_
