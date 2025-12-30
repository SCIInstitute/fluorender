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
#ifndef _BASE_CONV_VOL_MESH_H_
#define _BASE_CONV_VOL_MESH_H_

#include <Progress.h>
#include <string>
#include <memory>

class VolumeData;
class MeshData;
namespace flrd
{
	// Base class for converters
	class BaseConvVolMesh : public Progress
	{
	public:
		BaseConvVolMesh() : Progress() {}
		virtual ~BaseConvVolMesh() {}

		void SetVolumeData(const std::shared_ptr<VolumeData>& vd) { m_volume = vd; }
		std::shared_ptr<MeshData> GetMeshData() const { return m_mesh; }

		// Convert method to be implemented by derived classes
		virtual void Convert() = 0;
		virtual void Update(bool create_mesh) = 0;
		// merge vetices
		virtual void MergeVertices(bool avg_normals) = 0;
		//simplify
		virtual void Simplify(bool avg_normals) = 0;
		//smooth
		virtual void Smooth(bool avg_normals) = 0;

		//auto update
		virtual bool GetAutoUpdate() { return false; }

		//if auto threshold is calculated
		virtual bool GetAutoThreshold() { return false; }

		// Setters and getters for common properties
		virtual void SetUseTransfer(bool val) { m_use_transfer = val; }
		virtual bool GetUseTransfer() const { return m_use_transfer; }
		
		virtual void SetUseMask(bool val) { m_use_mask = val; }
		virtual bool GetUseMask() const { return m_use_mask; }

		virtual void SetIsoValue(double val) { m_iso = val; }
		virtual double GetIsoValue() const { return m_iso; }

		virtual void SetDownsample(int val) { m_downsample = val; }
		virtual int GetDownsample() const { return m_downsample; }
		virtual void SetDownsampleZ(int val) { m_downsample_z = val; }
		virtual int GetDownsampleZ() const { return m_downsample_z; }

		virtual void SetSimplify(double val) { m_simplify = val; }
		virtual double GetSimplify() const { return m_simplify; }

		virtual void SetSmooth(double strength, double scale) { m_smooth_strength = strength; m_smooth_scale = scale; }
		virtual void SetSmoothStrength(double val) { m_smooth_strength = val; }
		virtual void SetSmoothScale(double val) { m_smooth_scale = val; }
		virtual double GetSmoothStrength() const { return m_smooth_strength; }
		virtual double GetSmoothScale() const { return m_smooth_scale; }

		virtual std::string GetInfo() const { return m_info; }

		virtual bool GetMerged() const { return m_merged; }

	protected:
		std::weak_ptr<VolumeData> m_volume; // Pointer to the volume data
		std::shared_ptr<MeshData> m_mesh;
		bool m_merged = false;

		bool m_use_transfer = false; // Use transfer function
		bool m_use_mask = false; // Use mask for volume data
		double m_iso = 0.5; // Iso value for contouring
		int m_downsample = 1; // Downsampling factor in x and y
		int m_downsample_z = 1; // Downsampling factor in z
		double m_simplify = 0.0; // Simplification factor
		double m_smooth_strength = 0.0; // Smoothing factor in normal direction
		double m_smooth_scale = 0.0; // smoothing factor in tangential direction

		std::string m_info; // Information string
	};

}

#endif // _BASE_CONV_VOL_MESH_H_