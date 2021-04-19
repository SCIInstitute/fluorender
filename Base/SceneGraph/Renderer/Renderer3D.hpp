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
#ifndef FL_RENDERER_3D_HPP
#define FL_RENDERER_3D_HPP

#include <ProcessorNode.hpp>
#include <glm/glm.hpp>

namespace fluo
{
class Renderer3D : public ProcessorNode
{
public:

	Renderer3D();

	Renderer3D(const Renderer3D& renderer, const fluo::CopyOp& copyop = fluo::CopyOp::SHALLOW_COPY, bool copy_values = true);

	virtual bool isSameKindAs(const Renderer3D*) const {return true;}

	virtual const char* className() const { return "Renderer3D"; }

	virtual void render(Event& event) {};

protected:
	~Renderer3D();

	virtual void setupInputs();
	virtual void setupOutputs();

	void handleProjection();
	void handleCamera();
	glm::mat4 getObjectMat();
	glm::mat4 getDrawMat();
	glm::mat4 getInvtMat();
};
}
#endif//FL_RENDERER3D
