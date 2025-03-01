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
#ifndef _SCRIPTVISITORS_H_
#define _SCRIPTVISITORS_H_

#include <InfoVisitor.hpp>
#include <compatibility.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

class RoiVisitor : public fluo::NodeVisitor
{
public:
	RoiVisitor(std::string& valname,
		std::string& bgname,
		double var,
		int mode, 
		int filter) :
		fluo::NodeVisitor(),
		valname_(valname),
		bgname_(bgname),
		var_cut_(var),
		out_mode_(mode),
		filter_mode_(filter)
	{
		setTraversalMode(fluo::NodeVisitor::TRAVERSE_CHILDREN);
	}

	virtual void apply(fluo::Node& node)
	{
		getValues(&node);
		traverse(node);
	}

	virtual void apply(fluo::Group& group)
	{
		std::string type;
		group.getValue("type", type);
		if (type == "time")
			t_ = group.getName();
		if (type == "channel")
			ch_ = group.getName();
		traverse(group);
	}

	void computeOut()
	{
		bool has_bg = !bg_values_.empty();
		out_values_ = in_values_;//dup
		if (has_bg)
		{
			//get ratio
			for (auto it1 = out_values_.begin();
				it1 != out_values_.end(); ++it1)
			{
				CompValues& cv = *it1;
				for (auto it2 = cv.begin();
					it2 != cv.end(); ++it2)
				{
					BaseValues& bv = it2->second;
					size_t size = std::min(bg_values_.size(), bv.size());
					for (size_t i = 0; i < size; ++i)
						bv[i] = bg_values_[i] <= 0.0 ? bv[i] : bv[i] / bg_values_[i];

				}
			}
		}
		//filter ratio to get the constant
		int ch = 0;
		for (auto it1 = out_values_.begin();
			it1 != out_values_.end(); ++it1, ++ch)
		{
			CompValues& cv = *it1;
			for (auto it2 = cv.begin();
				it2 != cv.end(); ++it2)
			{
				BaseValues& bv = it2->second;
				//double mean = filter(bv);
				double mean1, mean2;
				if (filter_mode_ == 0)
					mean1 = filter(bv);
				else
					filter2(bv, mean1, mean2);
				for (size_t i = 0; i < bv.size(); ++i)
				{
					//dff = (in - mean * bkg) / (mean * bkg)
					auto it = in_values_[ch].find(it2->first);
					if (it == in_values_[ch].end())
						continue;
					double in = it->second[i];
					double f = 0;
					if (filter_mode_ == 0)
						f = mean1 * bg_values_[i];
					else
						f = (mean1 * i + mean2) * bg_values_[i];
					bv[i] = f <= 0.0 ? in : (in - f) / f;
				}
			}
		}
	}

	void output(fluo::Group* group)
	{
		switch (out_mode_)
		{
		case 0:
		default:
			output0(group);
			break;
		case 1:
			output1(group);
			break;
		}
	}

protected:
	void getValues(fluo::Object* object)
	{
		if (!object)
			return;
		int time = std::stoi(t_);
		int chann = std::stoi(ch_);
		std::string str;
		object->getValue("type", str);
		if (str == "backg_stat")
		{
			//get background mean value
			fluo::ValueTuple vt{ bgname_, "", "" };
			if (object->getValue(vt))
			{
				//expand bg values
				if (bg_values_.size() <= time)
					bg_values_.resize(time + 1);
				double dval = std::stod(std::get<2>(vt));
				bg_values_[time] = dval;
			}
		}
		else if (str == "comp" ||
			str == "ruler")
		{
			//local value
			fluo::ValueTuple vt{ valname_, "", "" };
			if (object->getValue(vt))
			{
				std::string rid = object->getName();
				//expand chann
				if (in_values_.size() <= chann)
					in_values_.resize(chann + 1);
				//get ruler
				CompValues& cv = in_values_.at(chann);
				auto rit = cv.find(rid);
				if (rit == cv.end())
				{
					cv.insert(std::pair<std::string, BaseValues>(rid, BaseValues()));
					rit = cv.find(rid);
				}
				BaseValues& bv = rit->second;
				if (time >= bv.size())
					bv.resize(time + 1);
				double dval = std::stod(std::get<2>(vt));
				bv[time] = dval;
			}
		}
	}

	void output0(fluo::Group* group)
	{
		int ch = 0;
		for (auto it1 = out_values_.begin();
			it1 != out_values_.end(); ++it1, ++ch)
		{
			CompValues& cv = *it1;
			for (auto it2 = cv.begin();
				it2 != cv.end(); ++it2)
			{
				BaseValues& bv = it2->second;
				for (size_t i = 0; i < bv.size(); ++i)
				{
					//write values
					fluo::Group* timeg = group->getOrAddGroup(std::to_string(i));
					fluo::Group* chg = timeg->getOrAddGroup(std::to_string(ch));
					fluo::Group* cmdg = chg->getOrAddGroup("roi");
					fluo::Node* rnode = cmdg->getOrAddNode(it2->first);
					rnode->addSetValue("roi_dff", bv[i]);
				}
			}
		}
	}
	void output1(fluo::Group* group)
	{
		std::string str("roi_dff");
		fluo::Group* cmdg = group->getOrAddGroup(str);
		cmdg->addSetValue("type", str);
		int ch = 0;
		for (auto it1 = out_values_.begin();
			it1 != out_values_.end(); ++it1, ++ch)
		{
			CompValues& cv = *it1;
			fluo::Group* chg = cmdg->getOrAddGroup(std::to_string(ch));
			chg->addSetValue("type", std::string("channel"));
			for (auto it2 = cv.begin();
				it2 != cv.end(); ++it2)
			{
				str = it2->first;
				BaseValues& bv = it2->second;
				fluo::Group* roig = chg->getOrAddGroup(str);
				roig->addSetValue("type", std::string("roi"));
				for (size_t i = 0; i < bv.size(); ++i)
				{
					//write values
					str = std::to_string(i);
					fluo::Node* tn = roig->getOrAddNode(str);
					tn->addSetValue("roi_dff", bv[i]);
				}
			}
		}
	}

private:
	int out_mode_;
	int filter_mode_;//0-constant; 1-ls
	std::string valname_;
	std::string bgname_;
	double var_cut_;
	std::string t_;
	std::string ch_;
	typedef std::vector<double> BaseValues;
	typedef std::unordered_map<std::string, BaseValues> CompValues;
	typedef std::vector<CompValues> ChannValues;
	ChannValues in_values_;
	ChannValues out_values_;
	BaseValues bg_values_;

	double filter(BaseValues& bv)
	{
		size_t on = bv.size();
		if (!on)
			return 0;
		on = std::max(size_t(1), on / 10);
		BaseValues temp = bv;//copy
		double mean = 0, var = 0;
		do
		{
			stats(temp, mean, var);
			if (var < var_cut_)
				break;
			//remove outliers
			size_t n = temp.size();
			double d, z = 1;
			int c = 0;
			do
			{
				for (auto it = temp.begin();
					it != temp.end();)
				{
					d = (*it - mean) / var;//z value
					if (d > z)
						it = temp.erase(it);
					else
						++it;
				}
				if (temp.size() == n)
					z /= 2;
				else
					break;
				c++;
			} while (c < 7 && temp.size() > on);//run 7 times max
		} while (temp.size() > on);
		return mean;
	}

	//least squares
	bool filter2(BaseValues& bv, double& b1, double& b2)
	{
		size_t on = bv.size();
		if (!on)
			return false;
		on = std::max(size_t(1), on / 10);
		BaseValues temp = bv;//copy
		double mean = 0, var = 0;
		b1 = b2 = 0;
		do
		{
			stats2(temp, mean, b1, b2, var);
			if (var < var_cut_)
				break;
			//remove outliers
			size_t n = temp.size();
			double d, z = 1;
			int c = 0;
			do
			{
				int i = 0;
				for (auto it = temp.begin();
					it != temp.end();)
				{
					d = (*it - mean) / var;//z value
					if (d > z)
						it = temp.erase(it);
					else
						++it;
					i++;
				}
				if (temp.size() == n)
					z /= 2;
				else
					break;
				c++;
			} while (c < 7 && temp.size() > on);//run 7 times max
		} while (temp.size() > on);
		return true;
	}

	void stats(BaseValues& bv, double& mean, double& var)
	{
		if (bv.empty())
			return;
		double sum = 0;
		for (auto it : bv)
			sum += it;
		mean = sum / bv.size();
		sum = 0;
		for (auto it : bv)
			sum += (it - mean) * (it - mean);
		var = sum / bv.size();
	}

	void stats2(BaseValues& bv, double& mean, double& b1, double& b2, double& var)
	{
		if (bv.empty())
			return;
		double x, y;
		double sumx = 0, sumy = 0, sumxy = 0, sumxx = 0;
		for (size_t i = 0; i < bv.size(); ++i)
		{
			x = static_cast<double>(i);
			y = bv[i];
			sumx += x;
			sumxx += x * x;
			sumy += y;
			sumxy += x * y;
		}
		double n = static_cast<double>(bv.size());
		mean = sumy / n;
		b1 = (n * sumxy - sumx * sumy) / (n * sumxx - sumx * sumx);
		b2 = (sumy - b1 * sumx) / n;
		sumy = 0;
		for (size_t i = 0; i < bv.size(); ++i)
		{
			x = static_cast<double>(i);
			y = b1 * x + b2;
			sumy += (bv[i] - y) * (bv[i] - y);
		}
		var = sumy / n;
	}
};

class OutCoordVisitor : public fluo::NodeVisitor
{
public:
	OutCoordVisitor(std::ofstream& ofs,
		std::set<std::string>& tnames) :
		fluo::NodeVisitor(),
		ofs_(&ofs),
		tnames_(tnames),
		dim_(0)
	{
		setTraversalMode(fluo::NodeVisitor::TRAVERSE_CHILDREN);
	}

	virtual void apply(fluo::Node& node)
	{
	}

	virtual void apply(fluo::Group& group)
	{
		std::string type;
		group.getValue("type", type);
		group.getValue("dim", dim_);
		if (tnames_.find(type) != tnames_.end())
			printValues(&group);
		traverse(group);
	}

protected:
	void printValues(fluo::Group* group)
	{
		if (!group)
			return;

		std::string str;
		for (size_t i = 0; i < group->getNumChildren(); ++i)
		{
			fluo::Group* ruler_group = group->getChild(i)->asGroup();
			if (!ruler_group)
				continue;
			//name line
			ruler_group->getValue("name", str);
			*ofs_ << ruler_group->getName() << ", " << str << std::endl;
			//value line
			for (size_t j = 0; j < ruler_group->getNumChildren(); ++j)
			{
				fluo::Node* point_node = ruler_group->getChild(j);
				if (!point_node)
					continue;

				std::vector<fluo::Point> coords;
				fluo::Point point;
				fluo::ValueVector names =
					point_node->getValueNames(3);
				*ofs_ << j << std::endl;
				for (auto it = names.begin();
					it != names.end();)
				{
					if (!IS_NUMBER(*it))
					{
						++it;
						continue;
					}
					if (point_node->getValue(*it, point))
						coords.push_back(point);
					//t
					*ofs_ << *it;
					++it;
					if (it == names.end())
						*ofs_ << std::endl;
					else
						*ofs_ << ", ";
				}
				//x
				for (size_t k = 0; k < coords.size(); ++k)
				{
					*ofs_ << coords[k].x();
					if (k == coords.size() - 1)
						*ofs_ << std::endl;
					else
						*ofs_ << ", ";
				}
				//y
				for (size_t k = 0; k < coords.size(); ++k)
				{
					*ofs_ << coords[k].y();
					if (k == coords.size() - 1)
						*ofs_ << std::endl;
					else
						*ofs_ << ", ";
				}
				//z
				if (dim_ == 0 || dim_ == 3)
				{
					for (size_t k = 0; k < coords.size(); ++k)
					{
						*ofs_ << coords[k].z();
						if (k == coords.size() - 1)
							*ofs_ << std::endl;
						else
							*ofs_ << ", ";
					}
				}
			}
		}
	}

private:
	std::ofstream* ofs_;
	std::set<std::string> tnames_;
	long dim_;
};

class OutTempVisitor : public fluo::NodeVisitor
{
public:
	OutTempVisitor(std::ofstream& ofs,
		std::set<std::string>& vnames,
		int num) : fluo::NodeVisitor(),
		ofs_(&ofs),
		vnames_(vnames),
		chnum_(num)
	{
		setTraversalMode(fluo::NodeVisitor::TRAVERSE_CHILDREN);
	}

	virtual void apply(fluo::Node& node)
	{
		printValues(&node);
		traverse(node);
	}

	virtual void apply(fluo::Group& group)
	{
		std::string type;
		group.getValue("type", type);
		if (type == "time")
			t_ = group.getName();
		if (type == "channel")
			ch_ = group.getName();
		traverse(group);
	}

protected:
	void printValues(fluo::Object* object)
	{
		if (!object)
			return;
		std::string str;
		object->getValue("type", str);
		if (str == "backg_stat")
		{
			for (auto it = vnames_.begin();
				it != vnames_.end(); ++it)
			{
				//local value
				fluo::ValueTuple vt{ *it, "", "" };
				if (object->getValue(vt))
				{
					auto vit = gvalues_.find(*it);
					if (vit == gvalues_.end())
						gvalues_.insert(std::pair<std::string, std::string>(
							*it, std::get<2>(vt)));
					else
						vit->second = std::get<2>(vt);
				}
			}
		}
		else if (str == "comp")
		{
			if (chnum_ > 1)
				*ofs_ << "CH-" << ch_ << " ";
			str = object->getName();
			*ofs_ << "ID-" << str << "," << t_;
			for (auto it = vnames_.begin();
				it != vnames_.end(); ++it)
			{
				*ofs_ << ",";
				//local value
				fluo::ValueTuple vt{ *it, "", "" };
				if (object->getValue(vt))
					*ofs_ << std::get<2>(vt);
				else
				{
					//global value
					auto vit = gvalues_.find(*it);
					if (vit == gvalues_.end())
						*ofs_ << "0";
					else
						*ofs_ << vit->second;
				}
			}
			*ofs_ << "\\n\\" << std::endl;
		}
		else if (str == "ruler")
		{
			if (chnum_ > 1)
				*ofs_ << "CH-" << ch_ << " ";
			str = object->getName();
			*ofs_ << "ID-" << str << "," << t_;
			for (auto it = vnames_.begin();
				it != vnames_.end(); ++it)
			{
				*ofs_ << ",";
				//local value
				fluo::ValueTuple vt{ *it, "", "" };
				if (object->getValue(vt))
					*ofs_ << std::get<2>(vt);
				else
				{
					//global value
					auto vit = gvalues_.find(*it);
					if (vit == gvalues_.end())
						*ofs_ << "0";
					else
						*ofs_ << vit->second;
				}
			}
			if (vnames_.find("intensity") != vnames_.end())
			{
				fluo::ValueVector names =
					object->getValueNames(3);
				for (auto it = names.begin();
					it != names.end(); ++it)
				{
					if (!IS_NUMBER(*it))
						continue;
					fluo::ValueTuple vt;
					std::get<0>(vt) = *it;
					if (object->getValue(vt))
					{
						*ofs_ << ",";
						*ofs_ << std::get<2>(vt);
					}
				}
			}
			*ofs_ << "\\n\\" << std::endl;
		}
	}

private:
	std::ofstream* ofs_;
	std::set<std::string> vnames_;
	int chnum_;
	std::string t_;
	std::string ch_;
	std::unordered_map<std::string, std::string> gvalues_;
};

class OutCsvVisitor : public fluo::NodeVisitor
{
public:
	OutCsvVisitor(std::ofstream& ofs,
		std::set<std::string>& tnames) :
		fluo::NodeVisitor(),
		ofs_(&ofs),
		tnames_(tnames)
	{
		setTraversalMode(fluo::NodeVisitor::TRAVERSE_CHILDREN);
	}

	virtual void apply(fluo::Node& node)
	{
	}

	virtual void apply(fluo::Group& group)
	{
		std::string type;
		group.getValue("type", type);
		if (tnames_.find(type) != tnames_.end())
			printValues(&group);
		traverse(group);
	}

protected:
	void printValues(fluo::Group* group)
	{
		if (!group)
			return;
		std::string vname(group->getName());

		std::string str;
		for (size_t i = 0; i < group->getNumChildren(); ++i)
		{
			//channels
			fluo::Group* chg = group->getChild(i)->asGroup();
			if (!chg)
				continue;
			//name line
			std::string chstr = "CH" + std::string(chg->getName());
			for (size_t j = 0; j < chg->getNumChildren(); ++j)
			{
				//rois
				fluo::Group* roig = chg->getChild(j)->asGroup();
				if (!roig)
					continue;
				str = chstr + " ID " + roig->getName();
				*ofs_ << str << std::endl;
				//value line
				for (size_t k = 0; k < roig->getNumChildren(); ++k)
				{
					if (k > 0)
						*ofs_ << ", ";
					fluo::Node* tn = roig->getChild(k);
					double dval;
					tn->getValue(vname, dval);
					*ofs_ << dval;
				}
				*ofs_ << std::endl;
			}
		}
	}

private:
	std::ofstream* ofs_;
	std::set<std::string> tnames_;
};

#endif//_SCRIPTVISITORS_H_