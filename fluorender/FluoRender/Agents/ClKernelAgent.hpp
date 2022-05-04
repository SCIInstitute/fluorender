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
#ifndef _CLKERNELAGENT_H_
#define _CLKERNELAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>

class ClKernelDlg;
namespace fluo
{
	class ClKernelAgent : public InterfaceAgent
	{
	public:
		ClKernelAgent(ClKernelDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const ClKernelAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "ClKernelAgent"; }

		virtual void setObject(Renderview* an);
		virtual Renderview* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual ClKernelAgent* asClKernelAgent() { return this; }
		virtual const ClKernelAgent* asClKernelAgent() const { return this; }

		void RunKernel(int v);
		void RunKernel();

		void copy_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);
		void box_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);
		void gauss_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);
		void median_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);
		void min_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);
		void max_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);
		void sobel_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);
		void morph_filter(void* data, void* result,
			int brick_x, int brick_y, int brick_z);

	protected:
		ClKernelDlg &dlg_;

		virtual void setupInputs();

	private:
	};
}

#endif//_CLKERNELAGENT_H_