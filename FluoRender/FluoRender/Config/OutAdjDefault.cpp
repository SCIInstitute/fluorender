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

#include <OutAdjDefault.h>
#include <Names.h>
#include <Global.h>
#include <RenderView.h>
#include <TreeLayer.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>

OutAdjDefault::OutAdjDefault()
{
	m_split = true;
	m_sync_r = false;
	m_sync_g = false;
	m_sync_b = false;
	m_gamma_r = 1.0;
	m_gamma_g = 1.0;
	m_gamma_b = 1.0;
	m_brightness_r = 1.0;
	m_brightness_g = 1.0;
	m_brightness_b = 1.0;
	m_hdr_r = 0;
	m_hdr_g = 0;
	m_hdr_b = 0;
}

OutAdjDefault::~OutAdjDefault()
{

}

void OutAdjDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	if (f->Exists("/outadj default"))
		f->SetPath("/outadj default");

	f->Read("split", &m_split, true);

	f->Read("sync_r", &m_sync_r, false);
	f->Read("sync_g", &m_sync_g, false);
	f->Read("sync_b", &m_sync_b, false);

	f->Read("gamma_r", &m_gamma_r, 1.0);
	f->Read("gamma_g", &m_gamma_g, 1.0);
	f->Read("gamma_b", &m_gamma_b, 1.0);

	f->Read("brightness_r", &m_brightness_r, 1.0);
	f->Read("brightness_g", &m_brightness_g, 1.0);
	f->Read("brightness_b", &m_brightness_b, 1.0);

	f->Read("hdr_r", &m_hdr_r, 0.0);
	f->Read("hdr_g", &m_hdr_g, 0.0);
	f->Read("hdr_b", &m_hdr_b, 0.0);
}

void OutAdjDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/outadj default");

	f->Write("split", m_split);

	f->Write("sync_r", m_sync_r);
	f->Write("sync_g", m_sync_g);
	f->Write("sync_b", m_sync_b);

	f->Write("gamma_r", m_gamma_r);
	f->Write("gamma_g", m_gamma_g);
	f->Write("gamma_b", m_gamma_b);

	f->Write("brightness_r", m_brightness_r);
	f->Write("brightness_g", m_brightness_g);
	f->Write("brightness_b", m_brightness_b);

	f->Write("hdr_r", m_hdr_r);
	f->Write("hdr_g", m_hdr_g);
	f->Write("hdr_b", m_hdr_b);
}

void OutAdjDefault::Set(RenderView* view)
{
	if (!view)
		return;

	m_sync_r = view->GetSync(0);
	m_sync_g = view->GetSync(1);
	m_sync_b = view->GetSync(2);

	fluo::Color c;

	c = view->GetGammaColor();
	m_gamma_r = c.r();
	m_gamma_g = c.g();
	m_gamma_b = c.b();

	c = view->GetBrightness();
	m_brightness_r = c.r();
	m_brightness_g = c.g();
	m_brightness_b = c.b();

	c = view->GetHdr();
	m_hdr_r = c.r();
	m_hdr_g = c.g();
	m_hdr_b = c.b();
}

void OutAdjDefault::Apply(RenderView* view)
{
	if (!view)
		return;

	view->SetSync(0, true);
	view->SetSync(1, true);
	view->SetSync(2, true);

	view->SetGammaColor(fluo::Color(1, 1, 1));
	view->SetBrightness(fluo::Color(1, 1, 1));
	view->SetHdr(fluo::Color(0, 0, 0));
}

void OutAdjDefault::Set(TreeLayer* layer)
{
	if (!layer)
		return;

	m_sync_r = layer->GetSync(0);
	m_sync_g = layer->GetSync(1);
	m_sync_b = layer->GetSync(2);

	fluo::Color c;

	c = layer->GetGammaColor();
	m_gamma_r = c.r();
	m_gamma_g = c.g();
	m_gamma_b = c.b();

	c = layer->GetBrightness();
	m_brightness_r = c.r();
	m_brightness_g = c.g();
	m_brightness_b = c.b();

	c = layer->GetHdr();
	m_hdr_r = c.r();
	m_hdr_g = c.g();
	m_hdr_b = c.b();
}

void OutAdjDefault::Apply(TreeLayer* layer)
{
	if (!layer)
		return;

	layer->SetSync(0, m_sync_r);
	layer->SetSync(1, m_sync_g);
	layer->SetSync(2, m_sync_b);

	layer->SetGammaColor(fluo::Color(m_gamma_r, m_gamma_g, m_gamma_b));
	layer->SetBrightness(fluo::Color(m_brightness_r, m_brightness_g, m_brightness_b));
	layer->SetHdr(fluo::Color(m_hdr_r, m_hdr_g, m_hdr_b));
}

