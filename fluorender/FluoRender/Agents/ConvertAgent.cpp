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

#include <ConvertAgent.hpp>
#include <ConvertDlg.h>
#include <Global.hpp>
#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <MeshData.hpp>
#include <MeshFactory.hpp>
#include <FLIVR/Texture.h>
#include <FLIVR/VolumeRenderer.h>
#include <Converters/VolumeMeshConv.h>
#include <wx/progdlg.h>

using namespace fluo;

ConvertAgent::ConvertAgent(ConvertDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void ConvertAgent::setupInputs()
{

}

void ConvertAgent::setObject(VolumeData* obj)
{
	InterfaceAgent::setObject(obj);
}

VolumeData* ConvertAgent::getObject()
{
	return dynamic_cast<VolumeData*>(InterfaceAgent::getObject());
}

void ConvertAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
	bool bval;
	long lval;
	double dval;
	if (update_all || FOUND_VALUE(gstVolMeshThresh))
	{
		getValue(gstVolMeshThresh, dval);
		dlg_.m_cnv_vol_mesh_thresh_text->SetValue(wxString::Format("%.2f", dval));
	}
	if (update_all || FOUND_VALUE(gstUseTransferFunc))
	{
		getValue(gstUseTransferFunc, bval);
		dlg_.m_cnv_vol_mesh_usetransf_chk->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		getValue(gstUseSelection, bval);
		dlg_.m_cnv_vol_mesh_selected_chk->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(gstVolMeshDownXY))
	{
		getValue(gstVolMeshDownXY, lval);
		dlg_.m_cnv_vol_mesh_downsample_text->SetValue(wxString::Format("%d", lval));
	}
	if (update_all || FOUND_VALUE(gstVolMeshDownZ))
	{
		getValue(gstVolMeshDownZ, lval);
		dlg_.m_cnv_vol_mesh_downsample_z_text->SetValue(wxString::Format("%d", lval));
	}
	if (update_all || FOUND_VALUE(gstVolMeshWeld))
	{
		getValue(gstVolMeshWeld, bval);
		dlg_.m_cnv_vol_mesh_weld_chk->SetValue(bval);
	}
}

void ConvertAgent::Convert()
{
	VolumeData* vd = getObject();
	if (!vd) return;

	wxProgressDialog *prog_diag = new wxProgressDialog(
		"FluoRender: Convert volume to polygon data",
		"Converting... Please wait.",
		100, 0,
		wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE);
	int progress = 0;

	progress = 50;
	prog_diag->Update(progress);

	VolumeMeshConv converter;
	converter.SetVolume(vd->GetTexture()->get_nrrd(0));
	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	converter.SetVolumeSpacings(spcx, spcy, spcz);
	double int_max;
	vd->getValue(gstMaxInt, int_max);
	converter.SetMaxValue(int_max);
	bool bval;
	long lval;
	double dval;
	//get iso value
	getValue(gstVolMeshThresh, dval);
	converter.SetIsoValue(dval);
	//get downsampling
	getValue(gstVolMeshDownXY, lval);
	converter.SetDownsample(lval);
	//get downsampling Z
	getValue(gstVolMeshDownZ, lval);
	converter.SetDownsampleZ(lval);
	//get use transfer function
	getValue(gstUseTransferFunc, bval);
	converter.SetVolumeUseTrans(bval);
	if (bval)
	{
		double gamma, lo_thresh, hi_thresh, offset, gm_thresh;
		vd->getValue(gstGamma3d, gamma);
		vd->getValue(gstLowThreshold, lo_thresh);
		vd->getValue(gstHighThreshold, hi_thresh);
		vd->getValue(gstSaturation, offset);
		vd->getValue(gstExtractBoundary, gm_thresh);
		converter.SetVolumeTransfer(gamma, lo_thresh, hi_thresh, offset, gm_thresh);
	}
	//get use selection
	getValue(gstUseSelection, bval);
	converter.SetVolumeUseMask(bval);
	if (bval)
	{
		vd->GetRenderer()->return_mask();
		converter.SetVolumeMask(vd->GetTexture()->get_nrrd(vd->GetTexture()->nmask()));
	}
	//start converting
	converter.Convert();
	GLMmodel* mesh = converter.GetMesh();

	progress = 90;
	prog_diag->Update(progress);

	if (mesh)
	{
		getValue(gstVolMeshWeld, bval);
		if (bval)
			glmWeld(mesh, 0.001 * Min(spcx, spcy, spcz));
		float area;
		float scale[3] = { 1.0f, 1.0f, 1.0f };
		glmArea(mesh, scale, &area);
		//DataManager* mgr = m_frame->GetDataManager();
		//mgr->LoadMeshData(mesh);
		MeshData* md = glbin_mshf->build();
		md->LoadData(mesh);
		Renderview* view = glbin_root->getCurrentRenderview();
		if (md && view)
		{
			view->addMeshData(md, 0);
			//view->RefreshGL(39);
		}
		(*(dlg_.m_stat_text)) <<
			"The surface area of mesh object " <<
			md->getName() << " is " <<
			wxString::Format("%f", area) << "\n";
	}

	delete prog_diag;

}