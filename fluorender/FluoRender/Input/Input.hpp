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
#ifndef _INPUT_H_
#define _INPUT_H_

#include <Flobject/Node.hpp>
#include <string>

#define gstInput "default input"

class Flinput : public fluo::Node
{
public:
	// 
	// Construction and destruction
	//

	// Default constructor
	//
	Flinput();
	Flinput(const Flinput& data, const fluo::CopyOp& copyop = fluo::CopyOp::SHALLOW_COPY, bool copy_values = true);

	virtual Object* clone(const fluo::CopyOp& copyop) const
	{
		return new Flinput(*this, copyop);
	}

	virtual bool isSameKindAs(const Object* obj) const
	{
		return dynamic_cast<const Flinput*>(obj) != NULL;
	}

	virtual const char* className() const { return "Flinput"; }

	virtual Flinput* asFlinput() { return this; }
	virtual const Flinput* asFlinput() const { return this; }

	void Update();

protected:
	// Destructor
	//
	virtual ~Flinput();

private:
	void updateState(const std::string &name, bool state)
	{
		std::string down, up, hold;
		down = name + " down";
		up = name + " up";
		hold = name + " hold";
		bool bval;
		getValue(hold, bval);
		setValue(hold, state);
		if (state && !bval)
			setValue(down, true);
		else
			setValue(down, false);
		if (!state, bval)
			setValue(up, true);
		else
			setValue(up, false);
	}
};

#endif // _INPUT_H_ 
