/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <RulerList.h>
#include <unordered_set>

using namespace flrd;

void RulerList::Clear()
{
	rulers_.clear();
	byName_.clear();
}

void RulerList::Add(std::shared_ptr<Ruler> ruler)
{
	if (!ruler)
		return;

	const auto& name = ruler->GetName();

	// Optional: enforce uniqueness
	if (byName_.contains(name))
		return; // or assert / throw

	rulers_.push_back(ruler);         // order preserved
	byName_.emplace(name, ruler);     // fast lookup
}

bool RulerList::RemoveByName(const std::wstring& name)
{
	auto it = byName_.find(name);
	if (it == byName_.end())
		return false;

	const auto& ruler = it->second;

	// Remove from ordered list
	auto vit = std::find(rulers_.begin(), rulers_.end(), ruler);
	if (vit != rulers_.end())
		rulers_.erase(vit);

	// Remove from map
	byName_.erase(it);
	return true;
}

std::shared_ptr<Ruler>
RulerList::FindByName(const std::wstring& name) const
{
	auto it = byName_.find(name);
	return it != byName_.end() ? it->second : nullptr;
}

std::shared_ptr<Ruler> RulerList::FindById(unsigned int id) const
{
	for (const auto& r : rulers_)
	{
		if (r->Id() == id)
			return r;
	}
	return nullptr;
}

std::shared_ptr<Ruler> RulerList::GetRuler(size_t index) const
{
	if (index >= rulers_.size())
		return nullptr;
	return rulers_[index];
}

std::vector<unsigned int> RulerList::Groups() const
{
	std::unordered_set<unsigned int> unique;

	for (const auto& r : rulers_)
		unique.insert(r->Group());

	return { unique.begin(), unique.end() };
}

std::unordered_map<unsigned int, int>
RulerList::GroupCounts() const
{
	std::unordered_map<unsigned int, int> counts;

	for (const auto& r : rulers_)
		++counts[r->Group()];

	return counts;
}

const std::vector<std::shared_ptr<Ruler>>& RulerList::All() const
{
	return rulers_;
}