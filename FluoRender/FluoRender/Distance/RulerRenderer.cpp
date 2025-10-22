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

#include <RulerRenderer.h>
#include <Global.h>
#include <Names.h>
#include <RenderView.h>
#include <Vector.h>
#include <Quaternion.h>
#include <TextRenderer.h>
#include <ShaderProgram.h>
#include <TextureRenderer.h>
#include <RulerHandler.h>
#include <ImgShader.h>
#include <VertexArray.h>
#include <Ruler.h>
#include <Color.h>
#include <Point.h>
#include <Transform.h>
#include <glm/gtc/type_ptr.hpp>

using namespace flrd;

RulerRenderer::RulerRenderer() :
	m_view(0),
	m_ruler_list(0),
	m_line_size(3),
	m_sel_line_size(5),
	m_draw_text(true)
{

}

RulerRenderer::~RulerRenderer()
{

}


void RulerRenderer::Draw()
{
	if (!m_view || !m_ruler_list)
		return;
	if (m_ruler_list->empty())
		return;

	int nx = m_view->GetCanvasSize().w();
	int ny = m_view->GetCanvasSize().h();

	auto shader = glbin_shader_manager.shader(
		gstImgShader, flvr::ShaderParams::Img(IMG_SHDR_DRAW_THICK_LINES, 0));
	assert(shader);
	shader->bind();
	glm::mat4 matrix = glm::ortho(float(0), float(nx), float(0), float(ny));
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	auto va_rulers =
		glbin_vertex_array_manager.vertex_array(flvr::VAType::VA_Rulers);
	assert(va_rulers);
	std::vector<float> verts;
	unsigned int num = 0;
		
	//draw unselected first
	shader->setLocalParam(0, nx, ny, m_line_size, 0.0);
	num = DrawVerts(verts, 2);
	if (num)
	{
		va_rulers->buffer_data(flvr::VABufferType::VABuf_Coord,
			sizeof(float)*verts.size(),
			&verts[0], flvr::BufferUsage::StreamDraw);
		va_rulers->set_param(0, num);
		va_rulers->draw();
	}
	//draw selected next
	shader->setLocalParam(0, nx, ny, m_sel_line_size, 0.0);
	num = DrawVerts(verts, 1);
	if (num)
	{
		va_rulers->buffer_data(flvr::VABufferType::VABuf_Coord,
			sizeof(float)*verts.size(),
			&verts[0], flvr::BufferUsage::StreamDraw);
		va_rulers->set_param(0, num);
		va_rulers->draw();
	}

	shader->unbind();

	//draw text
	if (m_draw_text)
		DrawTextAt(nx, ny);
}

unsigned int RulerRenderer::DrawVerts(std::vector<float> &verts, int sel_mode)
{
	if (!m_view || !m_ruler_list)
		return 0;

	verts.clear();

	size_t tseq_cur_num;
	if (m_view->m_frame_num_type == 1)
		tseq_cur_num = m_view->m_param_cur_num;
	else
		tseq_cur_num = m_view->m_tseq_cur_num;
	bool persp = m_view->GetPersp();
	int nx = m_view->GetCanvasSize().w();
	int ny = m_view->GetCanvasSize().h();
	float w = glbin_text_tex_manager.GetSize() / 4.0f;
	float px, py;

	fluo::Transform mv, p;
	glm::mat4 mv_mat = m_view->GetModelView();
	mv.set(glm::value_ptr(mv_mat));
	glm::mat4 proj_mat = m_view->GetProjection();
	p.set(glm::value_ptr(proj_mat));

	std::set<int> sel_list = glbin_ruler_handler.GetSelRulers();
	//estimate
	int vert_num = 0;
	for (size_t i = 0; i < m_ruler_list->size(); ++i)
	{
		Ruler* ruler = (*m_ruler_list)[i];
		if (!ruler || !ruler->GetDisp())
			continue;
		//check condition
		if (sel_mode == 1)
		{
			if (sel_list.find(static_cast<int>(i)) == sel_list.end())
				continue;
		}
		else if (sel_mode == 2)
		{
			if (sel_list.find(static_cast<int>(i)) != sel_list.end())
				continue;
		}

		vert_num += (*m_ruler_list)[i]->GetNumPoint();
	}
	verts.reserve(vert_num * 10 * 3 * 2);

	unsigned int num = 0;
	fluo::Point p1, p2;
	RulerPoint *rp1, *rp2;
	fluo::Color c;
	fluo::Color text_color = m_view->GetTextColor();
	size_t rwt = tseq_cur_num;
	for (size_t i = 0; i < m_ruler_list->size(); i++)
	{
		Ruler* ruler = (*m_ruler_list)[i];
		if (!ruler) continue;
		//check condition
		if (sel_mode == 1)
		{
			if (sel_list.find(static_cast<int>(i)) == sel_list.end())
				continue;
		}
		else if (sel_mode == 2)
		{
			if (sel_list.find(static_cast<int>(i)) != sel_list.end())
				continue;
		}

		ruler->SetWorkTime(rwt);
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();
		bool draw_square = ruler->GetDisplay(0);
		bool draw_line = ruler->GetDisplay(1);

		if (!ruler->GetTransient() ||
			(ruler->GetTransient() &&
				ruler->GetTransTime() == tseq_cur_num))
		{
			if (ruler->GetUseColor())
				c = ruler->GetColor();
			else
				c = text_color;
			if (ruler->GetRulerMode() == RulerMode::Ellipse)
			{
				int np = ruler->GetNumPoint();
				if (np == 1 && draw_square)
				{
					//draw square
					rp1 = ruler->GetRulerPoint(0);
					p1 = rp1->GetPoint(rwt, interp);
					p1 = mv.transform(p1);
					p1 = p.transform(p1);
					if ((persp && (p1.z() <= 0.0 || p1.z() >= 1.0)) ||
						(!persp && (p1.z() >= 0.0 || p1.z() <= -1.0)))
						continue;
					px = static_cast<float>((p1.x() + 1.0)*nx / 2.0);
					py = static_cast<float>((p1.y() + 1.0)*ny / 2.0);
					if (rp1->GetLocked())
						DrawPoint(verts, 1, px, py, w, c);
					else
						DrawPoint(verts, 0, px, py, w, c);
					num += 8;
				}
				else if (np == 4)
				{
					//draw ellipse
					RulerPoint* rps[4];
					rps[0] = ruler->GetRulerPoint(0);
					rps[1] = ruler->GetRulerPoint(1);
					rps[2] = ruler->GetRulerPoint(2);
					rps[3] = ruler->GetRulerPoint(3);
					fluo::Point pps[4];
					pps[0] = rps[0]->GetPoint(rwt, interp);
					pps[1] = rps[1]->GetPoint(rwt, interp);
					pps[2] = rps[2]->GetPoint(rwt, interp);
					pps[3] = rps[3]->GetPoint(rwt, interp);
					fluo::Point ppc = ruler->GetCenter();
					double ra, rb;
					ra = (pps[0] - pps[1]).length() / 2.0;
					rb = (pps[2] - pps[3]).length() / 2.0;
					if (draw_square)
					{
						for (size_t j = 0; j < 4; ++j)
						{
							p1 = mv.transform(pps[j]);
							p1 = p.transform(p1);
							if ((persp && (p1.z() <= 0.0 || p1.z() >= 1.0)) ||
								(!persp && (p1.z() >= 0.0 || p1.z() <= -1.0)))
								continue;
							px = static_cast<float>((p1.x() + 1.0) * nx / 2.0);
							py = static_cast<float>((p1.y() + 1.0) * ny / 2.0);
							if (rps[j]->GetLocked())
								DrawPoint(verts, 1, px, py, w, c);
							else
								DrawPoint(verts, 0, px, py, w, c);
							num += 8;
						}
						//draw center
						p1 = mv.transform(ppc);
						p1 = p.transform(p1);
						if ((persp && (p1.z() <= 0.0 || p1.z() >= 1.0)) ||
							(!persp && (p1.z() >= 0.0 || p1.z() <= -1.0)))
							continue;
						px = static_cast<float>((p1.x() + 1.0)*nx / 2.0);
						py = static_cast<float>((p1.y() + 1.0)*ny / 2.0);
						verts.push_back(px - w); verts.push_back(py); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						verts.push_back(px + w); verts.push_back(py); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						verts.push_back(px); verts.push_back(py - w); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						verts.push_back(px); verts.push_back(py + w); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						num += 4;
					}
					if (draw_line)
					{
						//draw arcs
						DrawArc(ppc, pps[0], pps[2], c, mv, p, verts, num);
						DrawArc(ppc, pps[2], pps[1], c, mv, p, verts, num);
						DrawArc(ppc, pps[1], pps[3], c, mv, p, verts, num);
						DrawArc(ppc, pps[3], pps[0], c, mv, p, verts, num);
					}
				}
			}
			else if (ruler->GetRulerMode() == RulerMode::Polyline &&
				ruler->GetNumBranch() > 1)
			{
				//multiple branches
				for (int bi = 0; bi < ruler->GetNumBranch(); ++bi)
					for (size_t j = 0; j < ruler->GetNumBranchPoint(bi); ++j)
					{
						rp2 = ruler->GetRulerPoint(bi, static_cast<int>(j));
						p2 = rp2->GetPoint(rwt, interp);
						p2 = mv.transform(p2);
						p2 = p.transform(p2);
						if ((persp && (p2.z() <= 0.0 || p2.z() >= 1.0)) ||
							(!persp && (p2.z() >= 0.0 || p2.z() <= -1.0)))
							continue;
						px = static_cast<float>((p2.x() + 1.0)*nx / 2.0);
						py = static_cast<float>((p2.y() + 1.0)*ny / 2.0);
						if (draw_square)
						{
							if (bi == 0 && j == 0)
							{
								if (rp2->GetLocked())
									DrawPoint(verts, 3, px, py, w, c);
								else
									DrawPoint(verts, 2, px, py, w, c);
							}
							else
							{
								if (rp2->GetLocked())
									DrawPoint(verts, 1, px, py, w, c);
								else
									DrawPoint(verts, 0, px, py, w, c);
							}
							num += 8;
						}
						if (j > 0 && draw_line)
						{
							//draw line
							p1 = ruler->GetPoint(bi, static_cast<int>(j) - 1);
							p1 = mv.transform(p1);
							p1 = p.transform(p1);
							if ((persp && (p1.z() <= 0.0 || p1.z() >= 1.0)) ||
								(!persp && (p1.z() >= 0.0 || p1.z() <= -1.0)))
								continue;
							verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
							verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
							px = static_cast<float>((p1.x() + 1.0)*nx / 2.0);
							py = static_cast<float>((p1.y() + 1.0)*ny / 2.0);
							verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
							verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
							num += 2;
						}
					}
			}
			else
			{
				for (size_t j = 0; j < ruler->GetNumPoint(); ++j)
				{
					rp2 = ruler->GetRulerPoint(static_cast<int>(j));
					p2 = rp2->GetPoint(rwt, interp);
					p2 = mv.transform(p2);
					p2 = p.transform(p2);
					if ((persp && (p2.z() <= 0.0 || p2.z() >= 1.0)) ||
						(!persp && (p2.z() >= 0.0 || p2.z() <= -1.0)))
						continue;
					px = static_cast<float>((p2.x() + 1.0)*nx / 2.0);
					py = static_cast<float>((p2.y() + 1.0)*ny / 2.0);
					if (draw_square)
					{
						if (ruler->GetNumPoint() > 1 && j == 0)
						{
							if (rp2->GetLocked())
								DrawPoint(verts, 3, px, py, w, c);
							else
								DrawPoint(verts, 2, px, py, w, c);
						}
						else
						{
							if (rp2->GetLocked())
								DrawPoint(verts, 1, px, py, w, c);
							else
								DrawPoint(verts, 0, px, py, w, c);
						}
						num += 8;
					}
					if (j > 0 && draw_line)
					{
						p1 = ruler->GetPoint(static_cast<int>(j) - 1);
						p1 = mv.transform(p1);
						p1 = p.transform(p1);
						if ((persp && (p1.z() <= 0.0 || p1.z() >= 1.0)) ||
							(!persp && (p1.z() >= 0.0 || p1.z() <= -1.0)))
							continue;
						verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						px = static_cast<float>((p1.x() + 1.0)*nx / 2.0);
						py = static_cast<float>((p1.y() + 1.0)*ny / 2.0);
						verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						num += 2;
					}
				}
				if (ruler->GetRulerMode() == RulerMode::Protractor &&
					ruler->GetNumPoint() >= 3 &&
					draw_line)
				{
					fluo::Point center = ruler->GetPoint(1);
					fluo::Vector v1 = ruler->GetPoint(0) - center;
					fluo::Vector v2 = ruler->GetPoint(2) - center;
					double len = std::min(v1.length(), v2.length());
					if (len > w)
					{
						v1.normalize();
						v2.normalize();
						p1 = center + v1 * w;
						p1 = mv.transform(p1);
						p1 = p.transform(p1);
						px = static_cast<float>((p1.x() + 1.0)*nx / 2.0);
						py = static_cast<float>((p1.y() + 1.0)*ny / 2.0);
						verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						p1 = center + v2 * w;
						p1 = mv.transform(p1);
						p1 = p.transform(p1);
						px = static_cast<float>((p1.x() + 1.0)*nx / 2.0);
						py = static_cast<float>((p1.y() + 1.0)*ny / 2.0);
						verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
						verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
						num += 2;
					}
				}
			}
		}
	}

	return num;
}

void RulerRenderer::DrawPoint(std::vector<float> &verts, int type, float px, float py, float w, fluo::Color &c)
{
	float w2 = static_cast<float>(1.41421356 * w);
	switch (type)
	{
	case 0://square
		verts.push_back(px - w); verts.push_back(py - w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w); verts.push_back(py - w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w); verts.push_back(py - w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w); verts.push_back(py + w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w); verts.push_back(py + w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w); verts.push_back(py + w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w); verts.push_back(py + w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w); verts.push_back(py - w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		break;
	case 1://hash
		verts.push_back(px - w); verts.push_back(py - w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w); verts.push_back(py - w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w / 2); verts.push_back(py - w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w / 2); verts.push_back(py + w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w); verts.push_back(py + w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w); verts.push_back(py + w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w / 2); verts.push_back(py + w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w / 2); verts.push_back(py - w); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		break;
	case 2://diamond
		verts.push_back(px + w2); verts.push_back(py); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px); verts.push_back(py + w2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px); verts.push_back(py + w2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w2); verts.push_back(py); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w2); verts.push_back(py); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px); verts.push_back(py - w2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px); verts.push_back(py - w2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w2); verts.push_back(py); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		break;
	case 3://diamond hash
		verts.push_back(px + w2 - w / 2); verts.push_back(py - w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w / 2); verts.push_back(py + w2 - w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w / 2); verts.push_back(py + w2 - w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w2 + w / 2); verts.push_back(py - w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w2 + w / 2); verts.push_back(py + w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w / 2); verts.push_back(py - w2 + w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px - w / 2); verts.push_back(py - w2 + w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		verts.push_back(px + w2 - w / 2); verts.push_back(py + w / 2); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		break;
	}
}

void RulerRenderer::DrawArc(fluo::Point & ppc, fluo::Point& pp0, fluo::Point& pp1,
	fluo::Color &c, fluo::Transform& mv, fluo::Transform& p,
	std::vector<float> &verts, unsigned int& num)
{
	if (!m_view)
		return;

	bool persp = m_view->GetPersp();
	int nx = m_view->GetCanvasSize().w();
	int ny = m_view->GetCanvasSize().h();
	float px, py;
	fluo::Point p1, p2;
	int sec = 20;
	//arc 02
	fluo::Vector v0 = pp0 - ppc;
	double lv0 = v0.length();
	fluo::Vector v1 = pp1 - ppc;
	double lv1 = v1.length();
	fluo::Vector axis = Cross(v0, v1);
	axis.normalize();
	fluo::Quaternion q0(v0);
	p1 = pp0;
	for (int i = 1; i <= sec; ++i)
	{
		double theta = 90.0 * i / sec;
		double rth = d2r(theta);
		fluo::Quaternion q(theta, axis);
		q.Normalize();
		fluo::Quaternion q2 = (-q) * q0 * q;
		fluo::Vector vp2(q2.x, q2.y, q2.z);
		vp2.normalize();
		vp2 *= lv0 * lv1 / (sqrt(lv1*lv1*cos(rth)*cos(rth) + lv0 * lv0*sin(rth)*sin(rth)));
		p2 = fluo::Point(vp2);
		p2 = ppc + fluo::Vector(p2);
		if (i == 1)
		{
			p1 = mv.transform(p1);
			p1 = p.transform(p1);
		}
		if ((persp && (p1.z() <= 0.0 || p1.z() >= 1.0)) ||
			(!persp && (p1.z() >= 0.0 || p1.z() <= -1.0)))
			continue;
		p2 = mv.transform(p2);
		p2 = p.transform(p2);
		if ((persp && (p2.z() <= 0.0 || p2.z() >= 1.0)) ||
			(!persp && (p2.z() >= 0.0 || p2.z() <= -1.0)))
			continue;
		px = static_cast<float>((p1.x() + 1.0)*nx / 2.0);
		py = static_cast<float>((p1.y() + 1.0)*ny / 2.0);
		verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		px = static_cast<float>((p2.x() + 1.0)*nx / 2.0);
		py = static_cast<float>((p2.y() + 1.0)*ny / 2.0);
		verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
		verts.push_back(static_cast<float>(c.r())); verts.push_back(static_cast<float>(c.g())); verts.push_back(static_cast<float>(c.b()));
		num += 2;
		p1 = p2;
	}
}

void RulerRenderer::DrawTextAt(int nx, int ny)
{
	size_t tseq_cur_num;
	if (m_view->m_frame_num_type == 1)
		tseq_cur_num = m_view->m_param_cur_num;
	else
		tseq_cur_num = m_view->m_tseq_cur_num;
	float w = glbin_text_tex_manager.GetSize() / 4.0f;
	float sx, sy;
	sx = static_cast<float>(2.0 / nx);
	sy = static_cast<float>(2.0 / ny);
	fluo::Color c;
	fluo::Color text_color = m_view->GetTextColor();
	fluo::Point p2;
	float p2x, p2y;
	fluo::Transform mv, p;
	glm::mat4 mv_mat = m_view->GetModelView();
	mv.set(glm::value_ptr(mv_mat));
	glm::mat4 proj_mat = m_view->GetProjection();
	p.set(glm::value_ptr(proj_mat));
	flvr::TextRenderer* text_renderer = m_view->GetTextRenderer();
	if (!text_renderer)
		return;
	size_t rwt = tseq_cur_num;
	for (size_t i = 0; i < m_ruler_list->size(); i++)
	{
		Ruler* ruler = (*m_ruler_list)[i];
		if (!ruler) continue;
		ruler->SetWorkTime(rwt);
		if (!ruler->GetDisp()) continue;
		if (!ruler->GetDisplay(2)) continue;
		if (!ruler->GetTransient() ||
			(ruler->GetTransient() &&
				ruler->GetTransTime() == tseq_cur_num))
		{
			if (ruler->GetUseColor())
				c = ruler->GetColor();
			else
				c = text_color;
			size_t j = ruler->GetNumPoint() - 1;
			p2 = ruler->GetPoint(static_cast<int>(j));
			p2 = mv.transform(p2);
			p2 = p.transform(p2);
			p2x = static_cast<float>(p2.x()*nx / 2.0);
			p2y = static_cast<float>(p2.y()*ny / 2.0);
			text_renderer->RenderText(
				ruler->GetName(),
				c,
				(p2x + w)*sx, (p2y + w)*sy, sx, sy);
		}
	}
}