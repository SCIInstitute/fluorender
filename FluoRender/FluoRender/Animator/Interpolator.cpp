﻿/*
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
#include <Interpolator.h>
#include <compatibility.h>
#include <inttypes.h>

int Interpolator::m_id = 0;

Interpolator::Interpolator()
{
	m_adding = 0;
}

Interpolator::~Interpolator()
{
	Clear();
}

//return group id, enable adding
int Interpolator::Begin(double t, double dt)
{
	m_id++;
	FlKeyGroup *group = new FlKeyGroup();
	if (!group)
		return -1;
	group->id = m_id;
	group->type = 0;
	group->t = t;
	group->dt = dt;

	m_key_list.push_back(group);
	m_adding = 1;

	return m_id;
}

//return 0: adding disabled
//otherwise return number of valid keys
int Interpolator::AddKey(FlKey *key)
{
	if (!m_adding)
		return 0;

	FlKeyGroup *group = m_key_list[m_key_list.size()-1];
	if (!group)
		return 0;
	group->keys.push_back(key);

	return (int)group->keys.size();
}

//return completeness
int Interpolator::End()
{
	m_adding = 0;
	if (m_key_list.size() == 0)
		return 0;
	else if (m_key_list.size() == 1)
		return 1;
	else
	{
		FlKeyGroup *group0 = m_key_list[0];
		FlKeyGroup *group = m_key_list[m_key_list.size()-1];
		if (!group0 || !group)
			return 0;
		if (group0->keys.size() != group->keys.size())
			return 0;
		for (int i=0; i<(int)group->keys.size(); i++)
			if (group->keys[i]->GetKeyCode() != 
				group0->keys[i]->GetKeyCode())
				return 0;
		return 1;//complete
	}
}

//get info
int Interpolator::GetKeyNum()
{
	return (int)m_key_list.size();
}

double Interpolator::GetFirstT()
{
	if (m_key_list.size()>0 && m_key_list[0])
		return m_key_list[0]->t;
	else return 0;
}

double Interpolator::GetLastT()
{
	if (m_key_list.size() > 0 && m_key_list[m_key_list.size()-1])
		return m_key_list[m_key_list.size()-1]->t;
	else return 0;
}

FlKeyGroup* Interpolator::GetKeyGroup(int index)
{
	if (index>=0 && (size_t)index<m_key_list.size())
		return m_key_list[index];
	else
		return 0;
}

FlKeyGroup* Interpolator::GetKeyGroupFromTime(double t)
{
	int index = GetKeyIndexFromTime(t);
	return GetKeyGroup(index);
}

int Interpolator::GetKeyIndex(int id)
{
	for (size_t i=0; i<m_key_list.size(); i++)
	{
		FlKeyGroup *group = m_key_list[i];
		if (group && group->id==id)
			return static_cast<int>(i);
	}
	return 0;
}

int Interpolator::GetKeyIndexFromTime(double t)
{
	int index = 0;
	for (int i=0; i<(int)m_key_list.size(); i++)
	{
		if (t <= m_key_list[i]->t)
		{
			index = i;
			break;
		}
	}
	return index;
}

int Interpolator::GetKeyID(int index)
{
	if (index>=0 && (size_t)index<m_key_list.size())
		return m_key_list[index]->id;
	else
		return 0;
}

double Interpolator::GetKeyTime(int index)
{
	if (index>=0 && (size_t)index<m_key_list.size())
		return m_key_list[index]->t;
	else
		return 0.0;
}

double Interpolator::GetKeyDuration(int index)
{
	if (index>0 && (size_t)index<m_key_list.size())
	{
		return m_key_list[index]->dt;
	}
	else
		return 0.0;
}

int Interpolator::GetKeyType(int index)
{
	if (index>=0 && (size_t)index<m_key_list.size())
		return m_key_list[index]->type;
	else
		return 0;
}

std::wstring Interpolator::GetKeyDesc(int index)
{
	if (index>=0 && (size_t)index<m_key_list.size())
		return m_key_list[index]->desc;
	else
		return L"";
}

//modify
void Interpolator::Clear()
{
	for (size_t i=0; i<m_key_list.size(); i++)
	{
		FlKeyGroup *group = m_key_list[i];
		if (!group) continue;
		for (int j=0; j<(int)group->keys.size(); j++)
		{
			if (group->keys[j])
				delete group->keys[j];
		}
		delete group;
	}
	m_key_list.clear();
}

//
void Interpolator::RemoveKey(int id)
{
	int i = 0;
	double time = 0.0;
	bool found = false;
	for (i=0; i<(int)m_key_list.size(); i++)
	{
		FlKeyGroup *group = m_key_list[i];
		if (!group) continue;
		if (group->id == id)
		{
			for (int j=0; j<(int)group->keys.size(); j++)
				if (group->keys[j])
					delete group->keys[j];
			m_key_list.erase(m_key_list.begin()+i);
			delete group;
			found = true;
			break;
		}
	}
	if (found)
		FixTime(i);
}

//move before
void Interpolator::MoveKeyBefore(int from_idx, int to_idx)
{
	if (from_idx == to_idx ||
		from_idx == to_idx-1)
		return;

	if (from_idx<0 || from_idx>=(int)m_key_list.size())
		return;
	FlKeyGroup *from_grp = m_key_list[from_idx];
	if (to_idx<0 || to_idx>=(int)m_key_list.size())
		return;
	FlKeyGroup *to_grp = m_key_list[to_idx];

	//insert before
	m_key_list.erase(m_key_list.begin() + from_idx);
	m_key_list.insert(m_key_list.begin() + to_idx, from_grp);

	FixTime(to_idx);
}

//move after
void Interpolator::MoveKeyAfter(int from_idx, int to_idx)
{
	if (from_idx == to_idx ||
		from_idx == to_idx+1)
		return;

	if (from_idx<0 || from_idx>=(int)m_key_list.size())
		return;
	FlKeyGroup *from_grp = m_key_list[from_idx];
	if (to_idx<0 || to_idx>=(int)m_key_list.size())
		return;
	FlKeyGroup *to_grp = m_key_list[to_idx];

	//insert after
	m_key_list.erase(m_key_list.begin() + from_idx);
	m_key_list.insert(m_key_list.begin() + to_idx, from_grp);

	FixTime(from_idx);
}

void Interpolator::FixTime(int index)
{
	if (index < 0 || index >= (int)m_key_list.size())
		return;

	for (size_t i = index; i < m_key_list.size(); ++i)
	{
		if (i == 0)
			continue;
		m_key_list[i]->t = m_key_list[i - 1]->t +
			m_key_list[i]->dt;
	}
}

void Interpolator::ChangeTime(int index, double time)
{
	if (index<0 || index>=(int)m_key_list.size())
		return;

	m_key_list[index]->t = time;
	if (index > 0)
	{
		m_key_list[index]->dt = m_key_list[index]->t -
			m_key_list[index - 1]->t;
	}

	double dura = 0.0;
	for (int i=index+1; i<(int)m_key_list.size(); i++)
	{
		m_key_list[i]->t = m_key_list[i-1]->t +
			m_key_list[i]->dt;
	}
}

void Interpolator::ChangeDuration(int index, double duration)
{
	if (index<0 || index>=(int)m_key_list.size())
		return;

	m_key_list[index]->dt = duration;
	for (int i=index; i<(int)m_key_list.size(); i++)
	{
		if (i > 0)
		{
			m_key_list[i]->t = m_key_list[i - 1]->t +
				m_key_list[i]->dt;
		}
	}
}

bool Interpolator::GetDouble(FlKeyCode keycode, double t, double &dval)
{
	int g1 = -1;
	int g2 = -1;

	for (int i=0; i<(int)m_key_list.size(); i++)
	{
		if (t > m_key_list[i]->t)
			g1 = i;
		else if (t < m_key_list[i]->t)
		{
			g2 = i;
			break;
		}
		else
		{
			g1 = g2 = i;
			break;
		}
	}

	if (g1==-1 && g2==-1)
	{
		dval = 0.0;
		return false;
	}

	if (g1>-1 && g2==-1)
	{
		if (g1 == 0)
			return StepDouble(keycode, m_key_list[0], dval);
		else
			return LinearDouble(keycode, m_key_list[g1-1], m_key_list[g1], t, dval);
	}

	if (g1==-1 && g2>-1)
	{
		if (g2 == (int64_t)m_key_list.size()-1)
			return StepDouble(keycode, m_key_list[g2], dval);
		else
			return LinearDouble(keycode, m_key_list[g2], m_key_list[g2+1], t, dval);
	}

	if (g1>-1 && g2>-1)
		return LinearDouble(keycode, m_key_list[g1], m_key_list[g2], t, dval);

	dval = 0.0;
	return false;
}

bool Interpolator::GetBoolean(FlKeyCode keycode, double t, bool &bval)
{
	int g1 = -1;
	int g2 = -1;

	for (int i=0; i<(int)m_key_list.size(); i++)
	{
		if (t > m_key_list[i]->t)
			g1 = i;
		else if (t < m_key_list[i]->t)
		{
			g2 = i;
			break;
		}
		else
		{
			g1 = g2 = i;
			break;
		}
	}

	if (g1 > -1)
	{
		FlKey *key = SearchKey(keycode, m_key_list[g1]);
		if (!key) return false;
		if (key->GetType() != FLKEY_TYPE_BOOLEAN)
			return false;
		bval = ((FlKeyBoolean*)key)->GetValue();
		return true;
	}
	else if (g2 > -1)
	{
		FlKey *key = SearchKey(keycode, m_key_list[g2]);
		if (!key) return false;
		if (key->GetType() != FLKEY_TYPE_BOOLEAN)
			return false;
		bval = ((FlKeyBoolean*)key)->GetValue();
		return true;
	}
	return false;
}

bool Interpolator::GetInt(FlKeyCode keycode, double t, int &ival)
{
	int g1 = -1;
	int g2 = -1;

	for (int i=0; i<(int)m_key_list.size(); i++)
	{
		if (t > m_key_list[i]->t)
			g1 = i;
		else if (t < m_key_list[i]->t)
		{
			g2 = i;
			break;
		}
		else
		{
			g1 = g2 = i;
			break;
		}
	}

	if (g1 > -1)
	{
		FlKey *key = SearchKey(keycode, m_key_list[g1]);
		if (!key) return false;
		if (key->GetType() != FLKEY_TYPE_INT)
			return false;
		ival = ((FlKeyInt*)key)->GetValue();
		return true;
	}
	else if (g2 > -1)
	{
		FlKey *key = SearchKey(keycode, m_key_list[g2]);
		if (!key) return false;
		if (key->GetType() != FLKEY_TYPE_INT)
			return false;
		ival = ((FlKeyInt*)key)->GetValue();
		return true;
	}
	return false;
}

bool Interpolator::GetColor(FlKeyCode keycode, double t, fluo::Color &cval)
{
	int g1 = -1;
	int g2 = -1;

	for (int i = 0; i < (int)m_key_list.size(); i++)
	{
		if (t > m_key_list[i]->t)
			g1 = i;
		else if (t < m_key_list[i]->t)
		{
			g2 = i;
			break;
		}
		else
		{
			g1 = g2 = i;
			break;
		}
	}

	if (g1 == -1 && g2 == -1)
	{
		cval = fluo::Color(1.0);
		return false;
	}

	if (g1 > -1 && g2 == -1)
	{
		if (g1 == 0)
			return StepColor(keycode, m_key_list[0], cval);
		else
			return LinearColor(keycode, m_key_list[g1 - 1], m_key_list[g1], t, cval);
	}

	if (g1 == -1 && g2 > -1)
	{
		if (g2 == (int64_t)m_key_list.size() - 1)
			return StepColor(keycode, m_key_list[g2], cval);
		else
			return LinearColor(keycode, m_key_list[g2], m_key_list[g2 + 1], t, cval);
	}

	if (g1 > -1 && g2 > -1)
		return LinearColor(keycode, m_key_list[g1], m_key_list[g2], t, cval);

	cval = fluo::Color(1.0);
	return false;
}

bool Interpolator::GetQuaternion(FlKeyCode keycode, double t, fluo::Quaternion &qval)
{
	int g1 = -1;
	int g2 = -1;

	for (int i=0; i<(int)m_key_list.size(); i++)
	{
		if (t > m_key_list[i]->t)
			g1 = i;
		else if (t < m_key_list[i]->t)
		{
			g2 = i;
			break;
		}
		else
		{
			g1 = g2 = i;
			break;
		}
	}

	if (g1==-1 && g2==-1)
	{
		qval = fluo::Quaternion(0, 0, 0, 1);
		return false;
	}

	if (g1>-1 && g2==-1)
	{
		if (g1 == 0)
			return StepQuaternion(keycode, m_key_list[0], qval);
		else
			return LinearQuaternion(keycode, m_key_list[g1-1], m_key_list[g1], t, qval);
	}

	if (g1==-1 && g2>-1)
	{
		if (g2 == (int64_t)m_key_list.size()-1)
			return StepQuaternion(keycode, m_key_list[g2], qval);
		else
			return LinearQuaternion(keycode, m_key_list[g2], m_key_list[g2+1], t, qval);
	}

	if (g1>-1 && g2>-1)
		return LinearQuaternion(keycode, m_key_list[g1], m_key_list[g2], t, qval);

	qval = fluo::Quaternion(0, 0, 0, 1);
	return false;
}

//get values at index
bool Interpolator::GetDouble(FlKeyCode keycode, int index, double &dval)
{
	if (index < 0 || index >= m_key_list.size())
	{
		dval = 0.0;
		return false;
	}

	FlKey *key = SearchKey(keycode, m_key_list[index]);
	if (!key)
	{
		dval = 0.0;
		return false;
	}

	dval = ((FlKeyDouble*)key)->GetValue();
	return true;
}

bool Interpolator::GetBoolean(FlKeyCode keycode, int index, bool &bval)
{
	if (index < 0 || index >= m_key_list.size())
	{
		bval = false;
		return false;
	}

	FlKey *key = SearchKey(keycode, m_key_list[index]);
	if (!key)
	{
		bval = false;
		return false;
	}

	bval = ((FlKeyBoolean*)key)->GetValue();
	return true;
}

bool Interpolator::GetInt(FlKeyCode keycode, int index, int &ival)
{
	if (index < 0 || index >= m_key_list.size())
	{
		ival = 0;
		return false;
	}

	FlKey *key = SearchKey(keycode, m_key_list[index]);
	if (!key)
	{
		ival = 0;
		return false;
	}

	ival = ((FlKeyInt*)key)->GetValue();
	return true;
}

bool Interpolator::GetQuaternion(FlKeyCode keycode, int index, fluo::Quaternion &qval)
{
	if (index < 0 || index >= m_key_list.size())
	{
		qval = fluo::Quaternion(0, 0, 0, 1);
		return false;
	}

	FlKey *key = SearchKey(keycode, m_key_list[index]);
	if (!key)
	{
		qval = fluo::Quaternion(0, 0, 0, 1);
		return false;
	}

	qval = ((FlKeyQuaternion*)key)->GetValue();
	return true;
}

//search
FlKey* Interpolator::SearchKey(FlKeyCode keycode, FlKeyGroup* g)
{
	if (!g) return 0;
	for (int i=0; i<(int)g->keys.size(); i++)
		if (g->keys[i]->GetKeyCode() == keycode)
			return g->keys[i];
	return 0;
}

//interpolations
bool Interpolator::StepDouble(FlKeyCode keycode, FlKeyGroup* g, double &dval)
{
	dval = 0.0;
	if (!g) return false;
	FlKey *key = SearchKey(keycode, g);
	if (!key) return false;
	if (key->GetType()!=FLKEY_TYPE_DOUBLE) return false;
	dval = ((FlKeyDouble*)key)->GetValue();
	return true;
}

bool Interpolator::LinearDouble(FlKeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, double &dval)
{
	dval = 0.0;
	if (!g1 || !g2) return false;
	FlKey *key1 = SearchKey(keycode, g1);
	FlKey *key2 = SearchKey(keycode, g2);
	if (!key1 || !key2) return false;
	if (key1->GetType()!=FLKEY_TYPE_DOUBLE ||
		key2->GetType()!=FLKEY_TYPE_DOUBLE)
		return false;
	double v1, v2, t1, t2;
	v1 = ((FlKeyDouble*)key1)->GetValue();
	v2 = ((FlKeyDouble*)key2)->GetValue();
	t1 = g1->t;
	t2 = g2->t;
	if (t1 == t2)
	{
		dval = v1;
		return true;
	}
	double st = Smooth((t-t1)/(t2-t1), g1->type==1, g2->type==1);
	dval = v1 + st * (v2-v1);
	return true;
}

bool Interpolator::StepColor(FlKeyCode keycode, FlKeyGroup* g, fluo::Color &cval)
{
	cval = fluo::Color(1.0);
	if (!g) return false;
	FlKey *key = SearchKey(keycode, g);
	if (!key) return false;
	if (key->GetType() != FLKEY_TYPE_COLOR) return false;
	cval = ((FlKeyColor*)key)->GetValue();
	return true;
}

bool Interpolator::LinearColor(FlKeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, fluo::Color &cval)
{
	cval = fluo::Color(1.0);
	if (!g1 || !g2) return false;
	FlKey *key1 = SearchKey(keycode, g1);
	FlKey *key2 = SearchKey(keycode, g2);
	if (!key1 || !key2) return false;
	if (key1->GetType() != FLKEY_TYPE_COLOR ||
		key2->GetType() != FLKEY_TYPE_COLOR)
		return false;
	fluo::Color v1, v2;
	double t1, t2;
	v1 = ((FlKeyColor*)key1)->GetValue();
	v2 = ((FlKeyColor*)key2)->GetValue();
	t1 = g1->t;
	t2 = g2->t;
	if (t1 == t2)
	{
		cval = v1;
		return true;
	}
	double st = Smooth((t - t1) / (t2 - t1), g1->type == 1, g2->type == 1);
	cval = v1 + (v2 - v1) * st;
	return true;
}

bool Interpolator::StepQuaternion(FlKeyCode keycode, FlKeyGroup* g, fluo::Quaternion &qval)
{
	qval = fluo::Quaternion(0, 0, 0, 1);
	if (!g) return false;
	FlKey *key = SearchKey(keycode, g);
	if (!key) return false;
	if (key->GetType()!=FLKEY_TYPE_QUATER)
		return false;
	qval = ((FlKeyQuaternion*)key)->GetValue();
	return true;
}

bool Interpolator::LinearQuaternion(FlKeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, fluo::Quaternion &qval)
{
	qval = fluo::Quaternion(0, 0, 0, 1);
	if (!g1 || !g2) return false;
	FlKey *key1 = SearchKey(keycode, g1);
	FlKey *key2 = SearchKey(keycode, g2);
	if (!key1 || !key2) return false;
	if (key1->GetType()!=FLKEY_TYPE_QUATER ||
		key2->GetType()!=FLKEY_TYPE_QUATER)
		return false;
	double t1, t2;
	fluo::Quaternion q1, q2;
	q1 = ((FlKeyQuaternion*)key1)->GetValue();
	q2 = ((FlKeyQuaternion*)key2)->GetValue();
	t1 = g1->t;
	t2 = g2->t;
	if (t1 == t2)
	{
		qval = q1;
		return true;
	}
	double st = Smooth((t-t1)/(t2-t1), g1->type==1, g2->type==1);
	qval = Slerp(q1, q2, st);
	return true;
}

double Interpolator::Smooth(double ft, bool s1, bool s2)
{
	if (s1 && s2)
		return -2.0*ft*ft*ft + 3.0*ft*ft;
	else if (s1)
		return -ft*ft*ft + 2.0*ft*ft;
	else if (s2)
		return -ft*ft*ft + ft*ft + ft;
	else
		return ft;
}
