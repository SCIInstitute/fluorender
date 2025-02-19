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
#ifndef _INTERPOLATOR_H_
#define _INTERPOLATOR_H_

#include <FlKey.h>
#include <FlKeyDouble.h>
#include <FlKeyQuaternion.h>
#include <FlKeyBoolean.h>
#include <FlKeyInt.h>
#include <FlKeyColor.h>
#include <vector>
#include <string>

typedef struct
{
	int id;		//identifier
	double t;	//key time
	double dt;	//duration
	int type;	//interpolation method: 0-linear; 1-spline
	std::vector<FlKey*> keys; //keys
	std::string desc;//descriptions
} FlKeyGroup;

class Interpolator
{
public:
	Interpolator();
	~Interpolator();

	//create
	//return group id
	int Begin(double t, double dt);
	//return successfulness
	int AddKey(FlKey *key);
	//return completeness
	int End();

	//get info
	int GetKeyNum();
	double GetFirstT();
	double GetLastT();
	FlKeyGroup* GetKeyGroup(int index);
	FlKeyGroup* GetKeyGroupFromTime(double t);
	int GetKeyIndex(int id);
	int GetKeyIndexFromTime(double t);
	int GetKeyID(int index);
	double GetKeyTime(int index);
	double GetKeyDuration(int index);
	int GetKeyType(int index);
	std::string GetKeyDesc(int index);
	int GetLastIndex()
	{ return (int)m_key_list.size() - 1;}
	std::vector<FlKeyGroup*>* GetKeyList()
	{ return &m_key_list; }

	//modify
	void Clear();
	void RemoveKey(int id);
	void FixTime(int index = 0);
	void ChangeTime(int index, double time);
	void ChangeDuration(int index, double duration);
	void MoveKeyBefore(int from_idx, int to_idx);
	void MoveKeyAfter(int from_idx, int to_idx);

	//get values
	bool GetDouble(FlKeyCode keycode, double t, double &dval);
	bool GetQuaternion(FlKeyCode keycode, double t, fluo::Quaternion &qval);
	bool GetBoolean(FlKeyCode keycode, double t, bool &bval);
	bool GetInt(FlKeyCode keycode, double t, int &ival);
	bool GetColor(FlKeyCode keycode, double t, fluo::Color &cval);
	//get values at index
	bool GetDouble(FlKeyCode keycode, int index, double &dval);
	bool GetQuaternion(FlKeyCode keycode, int index, fluo::Quaternion &qval);
	bool GetBoolean(FlKeyCode keycode, int index, bool &bval);
	bool GetInt(FlKeyCode keycode, int index, int &ival);
	bool GetColor(FlKeyCode keycode, int index, fluo::Color &cval);

	static int m_id;

private:
	//adding: 0-disabled; 1-enabled
	int m_adding;
	std::vector<FlKeyGroup*> m_key_list;

private:
	FlKey* SearchKey(FlKeyCode keycode, FlKeyGroup* g);
	bool StepDouble(FlKeyCode keycode, FlKeyGroup* g, double &dval);
	bool LinearDouble(FlKeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, double &dval);
	bool StepColor(FlKeyCode keycode, FlKeyGroup* g, fluo::Color &cval);
	bool LinearColor(FlKeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, fluo::Color &cval);
	bool StepQuaternion(FlKeyCode keycode, FlKeyGroup* g, fluo::Quaternion &qval);
	bool LinearQuaternion(FlKeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, fluo::Quaternion &qval);
	double Smooth(double ft, bool s1, bool s2);

};

#endif//_INTERPOLATOR_H_
