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
#include <Clusterizer.h>
#include <Global.h>
#include <ComponentDefault.h>
#include <ClusterMethod.h>
#include <dbscan.h>
#include <kmeans.h>
#include <exmax.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <Texture.h>
#include <VolumeRenderer.h>
#include <Cell.h>

using namespace flrd;

Clusterizer::Clusterizer():
	Progress(),
	m_method(0),
	m_num(0),
	m_maxiter(0),
	m_tol(0.0f),
	m_size(0),
	m_eps(0.0)
{
	m_in_cells = std::make_unique<CelpList>();
	m_out_cells = std::make_unique<CelpList>();
}

Clusterizer::~Clusterizer()
{
}

void Clusterizer::Compute()
{
	SetProgress(0, "Computing clusters.");

	m_in_cells->clear();
	m_out_cells->clear();

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return;
	int bits = vd->GetBits();
	void* data_data = nrrd_data->data;
	if (!data_data)
		return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
	{
		vd->AddEmptyLabel();
		nrrd_label = tex->get_nrrd(tex->nlabel());
	}
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	double scale = vd->GetScalarScale();
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);

	flrd::ClusterMethod* method = 0;
	//switch method
	switch (m_method)
	{
	case 0:
	{
		flrd::ClusterExmax* method_exmax = new flrd::ClusterExmax();
		method_exmax->SetClnum(glbin_comp_def.m_cluster_clnum);
		method_exmax->SetMaxiter(glbin_comp_def.m_cluster_maxiter);
		method_exmax->SetProbTol(glbin_comp_def.m_cluster_tol);
		method = method_exmax;
	}
		break;
	case 1:
	{
		flrd::ClusterDbscan* method_dbscan = new flrd::ClusterDbscan();
		method_dbscan->SetSize(glbin_comp_def.m_cluster_size);
		method_dbscan->SetEps(static_cast<float>(glbin_comp_def.m_cluster_eps));
		method = method_dbscan;
	}
		break;
	case 2:
	{
		flrd::ClusterKmeans* method_kmeans = new flrd::ClusterKmeans();
		method_kmeans->SetClnum(glbin_comp_def.m_cluster_clnum);
		method_kmeans->SetMaxiter(glbin_comp_def.m_cluster_maxiter);
		method = method_kmeans;
	}
		break;
	}

	if (!method)
		return;

	method->SetSpacings(spcx, spcy, spcz);

	//add cluster points
	size_t i, j, k;
	size_t index;
	size_t nxyz = nx * ny * nz;
	unsigned char mask_value;
	float data_value;
	unsigned int label_value;
	bool use_init_cluster = false;
	struct CmpCnt
	{
		unsigned int id;
		unsigned int size;
		bool operator<(const CmpCnt& cc) const
		{
			return size > cc.size;
		}
	};
	std::unordered_map<unsigned int, CmpCnt> init_clusters;
	std::set<CmpCnt> ordered_clusters;

	size_t ticks = method == 0 ? nxyz / 500 : nxyz / 1000;
	size_t count = 0;
	if (m_method == 0)
	{
		SetRange(0, 20);
		for (index = 0; index < nxyz; ++index)
		{
			mask_value = data_mask[index];
			if (!mask_value)
				continue;
			label_value = data_label[index];
			if (!label_value)
				continue;
			auto it = init_clusters.find(label_value);
			if (it == init_clusters.end())
			{
				CmpCnt cc = { label_value, 1 };
				init_clusters.insert(std::pair<unsigned int, CmpCnt>(
					label_value, cc));
			}
			else
			{
				it->second.size++;
			}
			if (index % 1000 == 0)
			{
				SetProgress(static_cast<int>(100 * count / ticks),
					"Computing clusters");
				count++;
			}
		}
		if (init_clusters.size() >= glbin_comp_def.m_cluster_clnum)
		{
			for (auto it = init_clusters.begin();
				it != init_clusters.end(); ++it)
				ordered_clusters.insert(it->second);
			use_init_cluster = true;
		}
	}

	if (m_method == 0)
		SetRange(20, 40);
	else
		SetRange(0, 40);
	for (i = 0; i < nx; ++i) for (j = 0; j < ny; ++j) for (k = 0; k < nz; ++k)
	{
		index = nx * ny * k + nx * j + i;
		mask_value = data_mask[index];
		if (mask_value)
		{
			if (bits == 8)
				data_value = ((unsigned char*)data_data)[index] / 255.0f;
			else if (bits == 16)
				data_value = ((unsigned short*)data_data)[index] * static_cast<float>(scale) / 65535.0f;
			flrd::EmVec pnt = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k) };
			label_value = data_label[index];
			int cid = -1;
			if (use_init_cluster)
			{
				cid = 0;
				bool found = false;
				for (auto it = ordered_clusters.begin();
					it != ordered_clusters.end(); ++it)
				{
					if (label_value == it->id)
					{
						found = true;
						break;
					}
					cid++;
				}
				if (!found)
					cid = -1;
			}
			method->AddClusterPoint(
				pnt, data_value, cid);

			//add to list
			auto iter = m_in_cells->find(label_value);
			if (iter != m_in_cells->end())
			{
				iter->second->Inc(i, j, k, data_value);
			}
			else
			{
				flrd::Cell* cell = new flrd::Cell(label_value);
				cell->Inc(i, j, k, data_value);
				m_in_cells->insert(std::pair<unsigned int, flrd::Celp>
					(label_value, flrd::Celp(cell)));
			}
		}
		if (index % 1000 == 0)
		{
			SetProgress(static_cast<int>(100 * count / ticks),
				"Computing clusters");
			count++;
		}
	}

	method->SetProgressFunc(GetProgressFunc());
	method->SetRange(40, 90);

	if (method->Execute())
	{
		method->SetRange(90, 100);
		method->GenerateNewIDs(0, (void*)data_label, nx, ny, nz, true);
		m_out_cells = std::make_unique<CelpList>(method->GetCellList());
		vd->GetVR()->clear_tex_label();
		//m_view->RefreshGL(39);//refresh needs to be performed by caller
	}

	delete method;

	SetRange(0, 100);
	SetProgress(0, "");
}

//in and out cell lists
CelpList& Clusterizer::GetInCells()
{
	return *m_in_cells;
}
CelpList& Clusterizer::GetOutCells()
{
	return *m_out_cells;
}
