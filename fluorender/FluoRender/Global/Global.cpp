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
#include <Global/Global.h>

using namespace FL;

bool Value::_precise = false;
std::unordered_map<std::string, Value::ValueType> Value::_value_map
{
	{"Referenced*", vt_pReferenced},
	{"bool", vt_bool},
	{"char", vt_char},
	{"unsigned char", vt_unsigned_char},
	{"short", vt_short},
	{"unsigned short", vt_unsigned_short},
	{"long", vt_long},
	{"unsigned long", vt_unsigned_long},
	{"long long", vt_long_long},
	{"unsigned long long", vt_unsigned_long_long},
	{"float", vt_float},
	{"double", vt_double},
	{"string", vt_string},
	{"wstring", vt_wstring},
	{"Point", vt_Point},
	{"Vector", vt_Vector},
	{"BBox", vt_BBox},
	{"HSVColor", vt_HSVColor},
	{"Color", vt_Color},
	{"Plane", vt_Plane},
	{"PlaneSet", vt_PlaneSet},
	{"Quaternion", vt_Quaternion},
	{"Ray", vt_Ray},
	{"Transform", vt_Transform},
	{"GLfloat4", vt_GLfloat4},
	{"GLint4", vt_GLint4}
};

Global Global::instance_;
Global::Global()
{
	volume_factory_ = ref_ptr<VolumeFactory>(new VolumeFactory());
	mesh_factory_ = ref_ptr<MeshFactory>(new MeshFactory());
	annotation_factory_ = ref_ptr<AnnotationFactory>(new AnnotationFactory());

	agent_factory_ = ref_ptr<FUI::AgentFactory>(new FUI::AgentFactory());
}
