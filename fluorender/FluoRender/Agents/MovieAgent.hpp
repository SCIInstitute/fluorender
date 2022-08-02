/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef _MOVIEAGENT_H_
#define _MOVIEAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>
#include <QVideoEncoder.h>

class MoviePanel;
namespace fluo
{
	class MovieAgent : public InterfaceAgent
	{
	public:
		MovieAgent(MoviePanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const MovieAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "MovieAgent"; }

		virtual void setObject(Renderview* view);
		virtual Renderview* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual MovieAgent* asMovieAgent() { return this; }
		virtual const MovieAgent* asMovieAgent() const { return this; }

		friend class AgentFactory;

		void Select(const std::string& name);
		int GetScriptFiles(std::vector<std::string>& list);
		void AddScriptToList();
		void GetScriptSettings();
		void SetProgress(double pcnt);
		void SetRendering(double pcnt, bool rewind);
		void WriteFrameToFile(int total_frames);
		void Prev();
		void Stop();
		void Run();
		void Rewind();
		void UpFrame();
		void DownFrame();
		void AutoKey();

		//timer
		void TimerRun();
		void ResumeRun();
		void HoldRun();

	protected:
		MoviePanel &panel_;

		virtual void setupInputs();

	private:
		QVideoEncoder encoder_;

	private:
		void OnTimer();
		void OnMovTimeSeqEnable(Event& event);
		void OnMovSeqMode(Event& event);
		void OnMovRotEnable(Event& event);
		void OnMovRotAxis(Event& event);
		void OnMovRotAng(Event& event);
		void OnCurrentFrame(Event& event);
		void OnDrawCropFrame(Event& event);
		void OnMovCurrentPage(Event& event);
		void OnMovLength(Event& event);
		void OnMovFps(Event& event);
		void OnBeginFrame(Event& event);
		void OnEndFrame(Event& event);
		void OnScriptFile(Event& event);
		void OnRunScript(Event& event);
		void OnMovCurTime(Event& event);
	};
}

#endif//_MOVIEAGENT_H_