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
#ifndef MoviePanelAgent_h
#define MoviePanelAgent_h

#include <Agent.h>
#include <Names.h>
#include <memory>
#include <vector>
#include <string>

class MoviePanel;

class MoviePanelAgent : public Agent
{
public:
	MoviePanelAgent(
		MoviePanel* panel);

	virtual ~MoviePanelAgent() = default;

	MoviePanel* GetPanel() const;

	void SelectKeyframe(int id);
	void DeleteKeyframe(int id);
	void DeleteAllKeyframes();
	void SetKeyframeTime(int id, double time);
	void SetKeyframeDuration(int id, double duration);
	void SetKeyframeInterpolation(int id, int type);
	void SetKeyframeDescription(int id, const std::wstring& description);
	void MoveKeyframe(int sourceId, int targetId, bool before);

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

	size_t GetScriptFiles(std::vector<std::wstring>& list);

	//common
	void SetFps();
	void SetMovieLength();
	void SetViewIndex();
	void SetSliderStyle();
	//frames
	void SetScrollFrame();
	void SetStartFrame();
	void SetEndFrame();
	void SetCurrentFrame();
	void SetCurrentTime();
	void SetFullFrame();
	void Play();
	void PlayInv();
	void Rewind();
	void Forward();
	void Loop();
	void IncFrame();
	void DecFrame();
	void Save();

	void SetRotateEnable();
	void SetRotateAxis();
	void SetRotateDeg();
	void SetRotateInterp();
	void SetSeqMode();
	void SetSeqDec();
	void SetSeqInc();
	void SetSeqNum();

	//keyframe movie
	void SetKeyframeNum();
	void SetKeyframeMovie();
	void SetKeyDuration();
	void SetKeyInterpolation();
	void InsertKey();

	//lock
	void SetCameraLock();
	void SetCameraLockType();
	void SetCameraLockCenter();

	//preset
	void GenerateKeys();

	//crop
	void SetCropEnable();
	void SetCropValues();
	void SetScalebarPos();
	void SetScalebarValues();

	//script
	void EnableScript();
	void SetScriptFile();
	void LoadScriptFile();
	void SelectScriptFile();
};

#endif // MoviePanelAgent_h
