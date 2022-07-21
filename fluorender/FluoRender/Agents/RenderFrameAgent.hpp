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
#ifndef _RENDERFRAMEAGENT_H_
#define _RENDERFRAMEAGENT_H_

#include <InterfaceAgent.hpp>
#include <Root.hpp>
#include <base_reader.h>
#include <glm.h>
#include <wx/arrstr.h>

class RenderFrame;
class RenderviewPanel;
namespace fluo
{
	class RenderFrameAgent : public InterfaceAgent
	{
	public:
		enum VolumeLoadType
		{
			VLT_ImageJ = 0,
			VLT_NRRD,
			VLT_TIFF,
			VLT_OIB,
			VLT_OIF,
			VLT_LSM,
			VLT_PVXML,
			VLT_BRKXML,
			VLT_CZI,
			VLT_ND2,
			VLT_LIF,
			VLT_LOF,
		};

		RenderFrameAgent(RenderFrame &frame);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const RenderFrameAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "RenderFrameAgent"; }

		virtual void setObject(Root* obj);
		virtual Root* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual RenderFrameAgent* asRenderFrameAgent() { return this; }
		virtual const RenderFrameAgent* asRenderFrameAgent() const { return this; }

		friend class AgentFactory;

		std::vector<std::string> GetJvmArgs();

		void SaveProject(const std::wstring &filename);
		void OpenProject(const std::wstring &filename);
		void StartupLoad(wxArrayString files, bool run_mov, bool with_imagej);
		void LoadVolumes(wxArrayString files, bool withImageJ);
		void LoadMeshes(wxArrayString files);
		wxString SearchProjectPath(const wxString &filename);
		int LoadVolumeData(const wxString &filename, int type, bool withImageJ, int ch_num = -1, int t_num = -1);
		int LoadMeshData(const wxString &filename);
		int LoadMeshData(GLMmodel* mesh);
		void SetTextureUndos();
		void SetTextureRendererSettings();
		Color GetWavelengthColor(double wavelength);
		Color GetColor(int c);
		bool CheckNames(const std::string &name);

		void Select(const std::string &name);//name of a node
		void AddView(RenderviewPanel* vrv, bool set_gl);

		wxString ScriptDialog(const wxString& title,
			const wxString& wildcard, long style);

		void UpdateSettings();

	protected:
		RenderFrame &frame_;

		virtual void setupInputs();

	private:
		std::vector<BaseReader*> m_reader_list;
	};
}

#endif//_RENDERFRAMEAGENT_H_