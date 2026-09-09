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
#ifndef ManipPropPanelAgent_h
#define ManipPropPanelAgent_h

#include <Agent.h>
#include <Names.h>
#include <memory>

class ManipPropPanel;
class MeshData;

class ManipPropPanelAgent : public Agent
{
public:
	ManipPropPanelAgent(
		ManipPropPanel* panel,
		const std::shared_ptr<MeshData>& md);

	virtual ~ManipPropPanelAgent() = default;

	ManipPropPanel* GetPanel() const;

	std::shared_ptr<MeshData> GetData() const
	{
		return m_md.lock();
	}

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstCurrentSelect,
	};

	std::weak_ptr<MeshData> m_md;
};

#endif // ManipPropPanelAgent_h
