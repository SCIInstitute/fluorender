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

#include "Ruler.h"
#include <FLIVR/Quaternion.h>
#include <Types/Utils.h>

using namespace FL;

int Ruler::m_num = 0;

Ruler::Ruler()
{
	m_id = m_num;
	m_num++;
	m_name = wxString::Format("Ruler %d", m_num);
	m_disp = true;
	m_tform = 0;
	m_ruler_type = 0;
	m_finished = false;
	m_use_color = false;

	//time-dependent
	m_time_dep = false;
	m_time = 0;

	//brush size
	m_brush_size = 0.0;
}

Ruler::~Ruler()
{
}

//data
int Ruler::GetNumBranch()
{
	return m_ruler.size();
}

int Ruler::GetNumPoint()
{
	//branch should be all connected
	//if there are more than one branch
	//the first point of a branch other than the first is shared
	//exclude it from num count
	int count = 0;
	bool first = true;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		count += first ? it->size() : it->size() - 1;
		first = false;
	}
	return count;
}

RulerPoint *Ruler::GetPoint(int index)
{
	if (index < 0)
		return 0;
	int count = 0;
	int size, seq, inc;
	bool first = true;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		size = it->size();
		inc = first ? size : size - 1;;
		if (index >= count && index < count + inc)
		{
			seq = index - count;
			if (!first && size > seq + 1)
				seq++;
			return (*it)[seq].get();
		}
		count += inc;
		first = false;
	}
	return 0;
}

RulerPoint* Ruler::GetLastPoint()
{
	for (auto it = m_ruler.rbegin();
		it != m_ruler.rend(); ++it)
	{
		if (it->size() < 2)
			continue;
		return it->back().get();
	}
	return 0;
}

pRulerPoint Ruler::GetPPoint(int index)
{
	if (index < 0)
		return 0;
	int count = 0;
	int size, seq, inc;
	bool first = true;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		size = it->size();
		inc = first ? size : size - 1;;
		if (index >= count && index < count + inc)
		{
			seq = index - count;
			if (!first && size > seq + 1)
				seq++;
			return (*it)[seq];
		}
		count += inc;
		first = false;
	}
	return 0;
}

int Ruler::GetNumBranchPoint(int nb)
{
	int branch_num = GetNumBranch();
	if (nb < 0 || nb >= branch_num)
		return 0;
	return m_ruler.at(nb).size();
}

RulerPoint* Ruler::GetPoint(int nb, int index)
{
	int branch_num = GetNumBranch();
	if (nb < 0 || nb >= branch_num)
		return 0;
	RulerBranch &branch = m_ruler.at(nb);
	if (index < 0 || index >= branch.size())
		return 0;
	return branch[index].get();
}

pRulerPoint Ruler::FindPoint(Point& point)
{
	bool first = true;
	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		for (size_t j = first ? 0 : 1; j < m_ruler[i].size(); ++j)
		{
			if (m_ruler[i][j]->GetPoint() == point)
			{
				return m_ruler[i][j];
			}
			first = false;
		}
	}
	return nullptr;
}

int Ruler::GetRulerType()
{
	return m_ruler_type;
}

void Ruler::SetRulerType(int type)
{
	m_ruler_type = type;
}

bool Ruler::GetFinished()
{
	return m_finished;
}

void Ruler::SetFinished()
{
	m_finished = true;
}

double Ruler::GetLength()
{
	double length = 0.0;
	Point p1, p2;

	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		for (size_t i = 1; i < it->size(); ++i)
		{
			p1 = (*it)[i - 1]->GetPoint();
			p2 = (*it)[i]->GetPoint();
			length += (p2 - p1).length();
		}
	}

	return length;
}

double Ruler::GetLengthObject(double spcx, double spcy, double spcz)
{
	double length = 0.0;
	Point p1, p2;

	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		for (size_t i = 1; i < it->size(); ++i)
		{
			p1 = (*it)[i - 1]->GetPoint();
			p2 = (*it)[i]->GetPoint();
			p1 = Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
			p2 = Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
			length += (p2 - p1).length();
		}
	}

	return length;
}

double Ruler::GetAngle()
{
	double angle = 0.0;

	if (m_ruler.empty())
		return angle;

	if (m_ruler_type == 0 ||
		m_ruler_type == 3)
	{
		if (m_ruler[0].size() >= 2)
		{
			Vector v = m_ruler[0][1]->GetPoint() - m_ruler[0][0]->GetPoint();
			v.normalize();
			angle = atan2(-v.y(), (v.x() > 0.0 ? 1.0 : -1.0)*sqrt(v.x()*v.x() + v.z()*v.z()));
			angle = FLTYPE::r2d(angle);
			angle = angle < 0.0 ? angle + 180.0 : angle;
		}
	}
	else if (m_ruler_type == 4)
	{
		if (m_ruler[0].size() >= 3)
		{
			Vector v1, v2;
			v1 = m_ruler[0][0]->GetPoint() - m_ruler[0][1]->GetPoint();
			v1.normalize();
			v2 = m_ruler[0][2]->GetPoint() - m_ruler[0][1]->GetPoint();
			v2.normalize();
			angle = acos(Dot(v1, v2));
			angle = FLTYPE::r2d(angle);
		}
	}

	return angle;
}

void Ruler::Scale(double spcx, double spcy, double spcz)
{
	bool first = true;
	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		for (size_t j = first ? 0 : 1; j < m_ruler[i].size(); ++j)
		{
			m_ruler[i][j]->ScalePoint(spcx, spcy, spcz);
		}
		first = false;
	}
}

bool Ruler::AddPoint(Point &point)
{
	if (m_ruler.empty())
	{
		m_ruler.push_back(RulerBranch());
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point)));
	}
	else if (m_ruler_type == 2 &&
		m_ruler.back().size() == 1)
		return false;
	else if ((m_ruler_type == 0 ||
		m_ruler_type == 3) &&
		m_ruler.back().size() == 2)
		return false;
	else if (m_ruler_type == 4 &&
		m_ruler.back().size() == 3)
		return false;
	else
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point)));

	if (m_ruler_type == 2 &&
		m_ruler.back().size() == 1)
		m_finished = true;
	else if ((m_ruler_type == 0 ||
		m_ruler_type == 3) &&
		m_ruler.back().size() == 2)
		m_finished = true;
	else if (m_ruler_type == 4 &&
		m_ruler.back().size() == 3)
		m_finished = true;
	return true;
}

bool Ruler::AddPointAfterId(
	Point &point, unsigned int id,
	std::set<unsigned int> &cid)
{
	if (m_ruler_type != 1)
		return false;
	if (m_ruler.empty())
	{
		m_ruler.push_back(RulerBranch());
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, id)));
		return true;
	}

	bool first = true;
	bool found = false;
	size_t ri, rj;
	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		for (size_t j = first ? 0 : 1; j < m_ruler[i].size(); ++j)
		{
			if (cid.find(m_ruler[i][j]->m_id) != cid.end())
			{
				ri = i; rj = j;
				found = true;
				break;
			}
			first = false;
		}
		if (found)
			break;
	}

	if (!found)
	{
		m_ruler.push_back(RulerBranch());
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, id)));
		return false;
	}

	if (rj == m_ruler[ri].size()-1)//last one
		m_ruler[ri].push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, id)));
	else
	{
		m_ruler.push_back(RulerBranch());
		m_ruler.back().push_back(m_ruler[ri][rj]);
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, id)));
	}

	return true;
}

void Ruler::SetTransform(Transform *tform)
{
	m_tform = tform;
}

bool Ruler::AddBranch(pRulerPoint point)
{
	if (!point ||
		m_ruler.empty() ||
		m_ruler_type != 1)
		return false;

	//add branch
	m_ruler.push_back(RulerBranch());
	m_ruler.back().push_back(point);
	return true;
}

void Ruler::Clear()
{
	m_ruler.clear();
}

void Ruler::Reverse()
{
	if (m_ruler.empty())
		return;
	if (m_ruler[0].size() > 1)
		std::reverse(std::begin(m_ruler[0]), std::end(m_ruler[0]));
}

wxString Ruler::GetDelInfoValues(wxString del)
{
	wxString output;

	for (size_t i = 0; i < m_info_values.length(); i++)
	{
		if (m_info_values[i] == '\t')
			output += del;
		else
			output += m_info_values[i];
	}

	return output;
}

wxString Ruler::GetPosValues()
{
	wxString output;

	//x string
	output += "x\t";
	for (size_t i = 0; i < m_ruler.size(); ++i)
		for (size_t j = 0; j < m_ruler[i].size(); ++j)
		{
			output += std::to_string(m_ruler[i][j]->GetPoint().x());
			if (i == m_ruler.size() - 1)
				output += "\n";
			else
				output += "\t";
		}
	//y string
	output += "y\t";
	for (size_t i = 0; i < m_ruler.size(); ++i)
		for (size_t j = 0; j < m_ruler[i].size(); ++j)
		{
			output += std::to_string(m_ruler[i][j]->GetPoint().y());
			if (i == m_ruler.size() - 1)
				output += "\n";
			else
				output += "\t";
		}
	//z string
	output += "z\t";
	for (size_t i = 0; i < m_ruler.size(); ++i)
		for (size_t j = 0; j < m_ruler[i].size(); ++j)
		{
			output += std::to_string(m_ruler[i][j]->GetPoint().z());
			if (i == m_ruler.size() - 1)
				output += "\n";
			else
				output += "\t";
		}

	return output;
}

wxString Ruler::GetPosNames()
{
	wxString output;

	output += "Coords\t";

	int count = 1;
	for (size_t i = 0; i < m_ruler.size(); ++i)
		for (size_t j = 0; j < m_ruler[i].size(); ++j)
		{
			output += "Point" + std::to_string(count++);
			if (i == m_ruler[i].size() - 1)
				output += "\n";
			else
				output += "\t";
		}

	return output;
}

void Ruler::SaveProfile(wxString &filename)
{
}

void Ruler::FinishEllipse(Vector view)
{
	if (m_ruler_type != 5 ||
		m_ruler.empty() ||
		m_ruler.back().size() != 2)
		return;

	Point p0 = m_ruler.back()[0]->GetPoint();
	Point p1 = m_ruler.back()[1]->GetPoint();
	Vector p01 = p0 - p1;
	Vector axis = Cross(p01, view);
	axis.normalize();
	axis = Cross(p01, axis);
	axis.normalize();
	Point p2, p3, pc;
	pc = Point((p0 + p1) / 2.0);
	Vector halfd = p0 - pc;
	Quaternion q0(halfd);
	Quaternion q(90.0, axis);
	q.Normalize();
	Quaternion q2 = (-q) * q0 * q;
	p2 = Point(q2.x, q2.y, q2.z);
	p3 = -p2;
	p2 = Point(pc + p2);
	p3 = Point(pc + p3);
	AddPoint(p2);
	AddPoint(p3);

	//nail
	m_ruler.back()[0]->SetLocked();
	m_ruler.back()[1]->SetLocked();
	m_ruler.back()[2]->SetLocked();
	m_ruler.back()[3]->SetLocked();

	m_finished = true;
}

Point Ruler::GetCenter()
{
	Point result;
	if (m_ruler.empty() ||
		m_ruler.back().empty())
		return result;
	bool first = true;
	int count = 0;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		for (size_t i = first ? 0 : 1; i < it->size(); ++i)
		{
			result += (*it)[i]->GetPoint();
			count++;
		}
		first = false;
	}
	if (count)
		result /= count;
	return result;
}

