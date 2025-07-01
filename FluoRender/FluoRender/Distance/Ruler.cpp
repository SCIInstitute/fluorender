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

#include <Ruler.h>
#include <Quaternion.h>
#include <Utils.h>
#include <fstream>

using namespace flrd;

int Ruler::m_num = 0;

Ruler::Ruler()
{
	m_id = m_num;
	m_group = 0;
	m_num++;
	m_name = L"Ruler " + std::to_wstring(m_num);
	m_disp = true;
	m_tform.load_identity();
	m_mode = RulerMode::None;
	m_finished = false;
	m_use_color = false;

	//work time
	m_work_time = 0;
	//transient
	m_transient = false;
	m_trans_time = 0;
	//display
	m_disp_part[0] = true;//point
	m_disp_part[1] = true;//line
	m_disp_part[2] = true;//name

	//brush size
	m_brush_size = 0.0;

	m_scale = 1;

	m_mean_int = 0;

	m_interp = 1;
}

Ruler::~Ruler()
{
}

//data
int Ruler::GetNumBranch()
{
	return static_cast<int>(m_ruler.size());
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
		count += static_cast<int>(first ? it->size() : it->size() - 1);
		first = false;
	}
	return count;
}

int Ruler::GetNumBranchPoint(int nb)
{
	int branch_num = GetNumBranch();
	if (nb < 0 || nb >= branch_num)
		return 0;
	return static_cast<int>(m_ruler.at(nb).size());
}

RulerPoint *Ruler::GetRulerPoint(int index)
{
	if (index < 0)
		return 0;
	int count = 0;
	int size, seq, inc;
	bool first = true;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		size = static_cast<int>(it->size());
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

RulerPoint* Ruler::GetLastRulerPoint()
{
	for (auto it = m_ruler.rbegin();
		it != m_ruler.rend(); ++it)
	{
		if (it->empty())
			continue;
		return it->back().get();
	}
	return 0;
}

pRulerPoint Ruler::GetPRulerPoint(int index)
{
	if (index < 0)
		return 0;
	int count = 0;
	int size, seq, inc;
	bool first = true;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		size = static_cast<int>(it->size());
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

RulerPoint* Ruler::GetRulerPoint(int nb, int index)
{
	int branch_num = GetNumBranch();
	if (nb < 0 || nb >= branch_num)
		return 0;
	RulerBranch &branch = m_ruler.at(nb);
	if (index < 0 || index >= branch.size())
		return 0;
	return branch[index].get();
}

pRulerPoint Ruler::GetPRulerPoint(int nb, int index)
{
	int branch_num = GetNumBranch();
	if (nb < 0 || nb >= branch_num)
		return 0;
	RulerBranch &branch = m_ruler.at(nb);
	if (index < 0 || index >= branch.size())
		return 0;
	return branch[index];
}

pRulerPoint Ruler::FindPRulerPoint(fluo::Point& point)
{
	bool first = true;
	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		for (size_t j = first ? 0 : 1; j < m_ruler[i].size(); ++j)
		{
			if (m_ruler[i][j]->GetPoint(m_work_time, m_interp) == point)
			{
				return m_ruler[i][j];
			}
			first = false;
		}
	}
	return nullptr;
}

pRulerPoint Ruler::FindNearestPRulerPoint(fluo::Point& point, size_t &ri, size_t &rj)
{
	bool first = true, found = false;
	double dist, min_dist;
	size_t mini, minj;
	for (size_t i = m_ruler.size(); i > 0; --i)
	{
		for (size_t j = m_ruler[i-1].size(); j > 0; --j)
		{
			dist = (m_ruler[i-1][j-1]->GetPoint(m_work_time, m_interp) - point).length2();
			if (first || dist < min_dist)
			{
				min_dist = dist;
				mini = i-1;
				minj = j-1;
				found = true;
			}
			first = false;
		}
	}
	if (found)
	{
		ri = mini;
		rj = minj;
		return m_ruler[mini][minj];
	}
	return nullptr;
}

pRulerPoint Ruler::FindBranchPRulerPoint(fluo::Point& point, size_t& ri, size_t& rj)
{
	bool found = false;
	double dist;
	double min_dist = std::numeric_limits<double>::max();
	size_t mini;
	for (size_t i = m_ruler.size(); i > 0; --i)
	{
		if (m_ruler[i-1].size() < 2)
			continue;
		dist = (m_ruler[i-1][0]->GetPoint(m_work_time, m_interp) - point).length2();
		if (dist < min_dist)
		{
			min_dist = dist;
			mini = i-1;
			found = true;
		}
	}
	if (found)
	{
		ri = mini;
		rj = 0;
		return m_ruler[mini][0];
	}
	return nullptr;
}

RulerMode Ruler::GetRulerMode()
{
	return m_mode;
}

void Ruler::SetRulerMode(RulerMode type)
{
	m_mode = type;
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
	fluo::Point p1, p2;

	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		for (size_t i = 1; i < it->size(); ++i)
		{
			p1 = (*it)[i - 1]->GetPoint(m_work_time, m_interp);
			p2 = (*it)[i]->GetPoint(m_work_time, m_interp);
			length += (p2 - p1).length();
		}
	}

	return length;
}

double Ruler::GetLengthObject(double spcx, double spcy, double spcz)
{
	double length = 0.0;
	fluo::Point p1, p2;

	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		for (size_t i = 1; i < it->size(); ++i)
		{
			p1 = (*it)[i - 1]->GetPoint(m_work_time, m_interp);
			p2 = (*it)[i]->GetPoint(m_work_time, m_interp);
			p1 = fluo::Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
			p2 = fluo::Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
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

	if (m_mode == RulerMode::Line ||
		m_mode == RulerMode::Probe)
	{
		if (m_ruler[0].size() >= 2)
		{
			fluo::Vector v = m_ruler[0][1]->GetPoint(m_work_time, m_interp) -
				m_ruler[0][0]->GetPoint(m_work_time, m_interp);
			v.normalize();
			angle = atan2(-v.y(), (v.x() > 0.0 ? 1.0 : -1.0)*
				sqrt(v.x()*v.x() + v.z()*v.z()));
			angle = fluo::r2d(angle);
			angle = angle < 0.0 ? angle + 180.0 : angle;
		}
	}
	else if (m_mode == RulerMode::Protractor)
	{
		if (m_ruler[0].size() >= 3)
		{
			fluo::Vector v1, v2;
			v1 = m_ruler[0][0]->GetPoint(m_work_time, m_interp) -
				m_ruler[0][1]->GetPoint(m_work_time, m_interp);
			v1.normalize();
			v2 = m_ruler[0][2]->GetPoint(m_work_time, m_interp) -
				m_ruler[0][1]->GetPoint(m_work_time, m_interp);
			v2.normalize();
			angle = acos(Dot(v1, v2));
			angle = fluo::r2d(angle);
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

bool Ruler::AddPoint(fluo::Point &point)
{
	if (m_ruler.empty())
	{
		m_ruler.push_back(RulerBranch());
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, m_work_time)));
	}
	else if (m_mode == RulerMode::Locator &&
		m_ruler.back().size() == 1)
		return false;
	else if ((m_mode == RulerMode::Line ||
		m_mode == RulerMode::Probe) &&
		m_ruler.back().size() == 2)
		return false;
	else if (m_mode == RulerMode::Protractor &&
		m_ruler.back().size() == 3)
		return false;
	else
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, m_work_time)));

	if (m_mode == RulerMode::Locator &&
		m_ruler.back().size() == 1)
		m_finished = true;
	else if ((m_mode == RulerMode::Line ||
		m_mode == RulerMode::Probe) &&
		m_ruler.back().size() == 2)
		m_finished = true;
	else if (m_mode == RulerMode::Protractor &&
		m_ruler.back().size() == 3)
		m_finished = true;
	return true;
}

bool Ruler::AddPointAfterId(
	fluo::Point &point, unsigned int id,
	std::set<unsigned int> &cid,
	std::set<unsigned int> &bid)
{
	if (m_mode != RulerMode::Polyline)
		return false;
	if (m_ruler.empty())
	{
		m_ruler.push_back(RulerBranch());
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, id, bid, m_work_time)));
		return true;
	}

	bool found = false;
	size_t ri, rj;
	for (size_t i = m_ruler.size(); i > 0; --i)
	{
		for (size_t j = m_ruler[i-1].size(); j > 0; --j)
		{
			//if (cid.find(m_ruler[i][j]->m_id) != cid.end())
			for (auto sit = cid.begin(); 
				sit != cid.end(); ++sit)
				if (m_ruler[i-1][j-1]->MatchId(*sit))
				{
					ri = i-1; rj = j-1;
					found = true;
					break;
				}
		}
		if (found)
			break;
	}

	//search for nearest point
	if (!found)
	{
		if (FindNearestPRulerPoint(point, ri, rj))
			found = true;
	}

	if (!found)
		return false;

	if (rj == m_ruler[ri].size()-1)//last one
		m_ruler[ri].push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, id, bid, m_work_time)));
	else
	{
		m_ruler.push_back(RulerBranch());
		m_ruler.back().push_back(m_ruler[ri][rj]);
		m_ruler.back().push_back(
			std::make_shared<RulerPoint>(RulerPoint(point, id, bid, m_work_time)));
	}

	return true;
}

void Ruler::SetTransform(const fluo::Transform &tform)
{
	m_tform = tform;
}

fluo::Transform Ruler::GetTransform()
{
	return m_tform;
}

bool Ruler::AddBranch(pRulerPoint point)
{
	if (!point ||
		m_ruler.empty() ||
		m_mode != RulerMode::Polyline)
		return false;

	//add branch
	m_ruler.push_back(RulerBranch());
	m_ruler.back().push_back(point);
	return true;
}

void Ruler::DeletePoint(pRulerPoint &point)
{
	if (!point)
		return;

	bool first = true;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		for (auto it2 = it->begin();
			it2 != it->end(); ++it2)
		{
			if (*it2 == point)
			{
				it->erase(it2);
				if ((first && it->empty()) ||
					(!first && it->size() < 2))
					m_ruler.erase(it);
				return;
			}
		}
		first = false;
	}
}

void Ruler::Prune(int len)
{
	size_t lastj;
	bool found;
	for (size_t i = m_ruler.size(); i > 0; --i)
	{
		//if the branch is shorter than len, remove it
		if (m_ruler[i-1].size() <= len + 1)
			m_ruler.erase(m_ruler.begin() + i - 1);
		else
		{
			found = false;
			//find the starting point of the branch
			pRulerPoint pp = m_ruler[i-1][0];
			for (size_t ii = 0; ii < m_ruler.size(); ++ii)
			{
				for (size_t jj = 0; jj < m_ruler[ii].size(); ++jj)
				{
					if (m_ruler[ii][jj] == pp && ii != i-1)
					{
						//found
						lastj = m_ruler[ii].size() - 1;
						if (lastj - jj <= len)
						{
							//remove remainder
							m_ruler[ii].resize(jj + 1);
							//copy i branch to ii
							m_ruler[ii].insert(m_ruler[ii].end(),
								m_ruler[i-1].begin() + 1, m_ruler[i-1].end());
							//delete branch i
							m_ruler.erase(m_ruler.begin() + i - 1);
							//set flag and break
							found = true;
							break;
						}
					}
				}
				if (found)
					break;
			}
		}
	}
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

std::string Ruler::GetDelInfoValues(const std::string& del)
{
	std::string output;

	for (size_t i = 0; i < m_info_values.length(); i++)
	{
		if (m_info_values[i] == '\t')
			output += del;
		else
			output += m_info_values[i];
	}

	return output;
}

std::string Ruler::GetPosValues()
{
	std::string output;

	//x string
	output += "x\t";
	for (size_t i = 0; i < m_ruler.size(); ++i)
	for (size_t j = 0; j < m_ruler[i].size(); ++j)
	{
		output += std::to_string(
			m_ruler[i][j]->GetPoint(m_work_time, m_interp).x());
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
		output += std::to_string(
			m_ruler[i][j]->GetPoint(m_work_time, m_interp).y());
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
		output += std::to_string(
			m_ruler[i][j]->GetPoint(m_work_time, m_interp).z());
		if (i == m_ruler.size() - 1)
			output += "\n";
		else
			output += "\t";
	}

	return output;
}

std::string Ruler::GetPosNames()
{
	std::string output;

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

//profile
void Ruler::SetPaintIntensity(int ival, double dval)
{
	//m_profile.clear();
	ProfileBin bin;
	bin.m_pixels = ival;
	bin.m_accum = dval;
	bin.dist = 0;
	m_profile.push_back(bin);
}

void Ruler::SaveProfile(const std::string &filename)
{
	if (m_profile.empty())
		return;
	std::ofstream ofs(filename.c_str());
	if (!ofs)
		return;
	//header
	ofs << "Distance\tValue\n";
	//data
	for (size_t i = 0; i < m_profile.size(); ++i)
	{
		ofs << m_profile[i].dist << "\t";
		if (m_profile[i].m_pixels <= 0)
			ofs << "0.0\n";
		else
			ofs << m_profile[i].m_accum / m_profile[i].m_pixels << "\n";
	}
	ofs.close();
}

double Ruler::GetProfileMaxValue()
{
	double dval, max_val = 0.0;
	if (m_profile.empty())
		return 0.0;
	for (size_t i = 0; i < m_profile.size(); ++i)
	{
		//for each profile
		int pixels = m_profile[i].m_pixels;
		if (pixels <= 0)
			dval = 0;
		else
		{
			dval = m_profile[i].m_accum / pixels;
			max_val = std::max(max_val, dval);
		}
	}
	return max_val;
}

void Ruler::GetProfileMaxValue(double &val, double &dist)
{
	val = 0;
	dist = 0;
	double dval;
	if (m_profile.empty())
		return;
	for (size_t i = 0; i < m_profile.size(); ++i)
	{
		//for each profile
		int pixels = m_profile[i].m_pixels;
		if (pixels <= 0)
			dval = 0;
		else
		{
			dval = m_profile[i].m_accum / pixels;
			if (dval > val)
			{
				val = dval;
				dist = m_profile[i].dist;
			}
		}
	}
}

void Ruler::FinishEllipse(fluo::Vector view)
{
	if (m_mode != RulerMode::Ellipse ||
		m_ruler.empty() ||
		m_ruler.back().size() != 2)
		return;

	fluo::Point p0 = m_ruler.back()[0]->GetPoint(m_work_time, m_interp);
	fluo::Point p1 = m_ruler.back()[1]->GetPoint(m_work_time, m_interp);
	fluo::Vector p01 = p0 - p1;
	fluo::Vector axis = Cross(p01, view);
	axis.normalize();
	axis = Cross(p01, axis);
	axis.normalize();
	fluo::Point p2, p3, pc;
	pc = fluo::Point((p0 + p1) / 2.0);
	fluo::Vector halfd = p0 - pc;
	fluo::Quaternion q0(halfd);
	fluo::Quaternion q(90.0, axis);
	q.Normalize();
	fluo::Quaternion q2 = (-q) * q0 * q;
	p2 = fluo::Point(q2.x, q2.y, q2.z);
	p3 = -p2;
	p2 = fluo::Point(pc + p2);
	p3 = fluo::Point(pc + p3);
	AddPoint(p2);
	AddPoint(p3);

	//nail
	m_ruler.back()[0]->SetLocked();
	m_ruler.back()[1]->SetLocked();
	m_ruler.back()[2]->SetLocked();
	m_ruler.back()[3]->SetLocked();

	m_finished = true;
}

fluo::Point Ruler::GetCenter()
{
	fluo::Point result;
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
			result += (*it)[i]->GetPoint(m_work_time, m_interp);
			count++;
		}
		first = false;
	}
	if (count)
		result /= count;
	return result;
}

