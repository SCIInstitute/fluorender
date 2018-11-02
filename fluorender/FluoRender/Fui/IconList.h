/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef _ICONLIST_H_
#define _ICONLIST_H_

#include <unordered_map>
#include <Formats/png_resource.h>
#include <img/icons.h>

namespace FUI
{
	class IconList
	{
	public:
		IconList() {}

		void init(bool shown = true)
		{
			wxIcon icon;
			if (shown)
			{
				icon.CopyFromBitmap(wxGetBitmapFromMemory(volume_data_shown));
				list_.insert(std::pair<std::string, wxIcon>(
					"VolumeData", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(mesh_data_shown));
				list_.insert(std::pair<std::string, wxIcon>(
					"MeshData", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(volume_group_shown));
				list_.insert(std::pair<std::string, wxIcon>(
					"VolumeGroup", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(mesh_group_shown));
				list_.insert(std::pair<std::string, wxIcon>(
					"MeshGroup", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(render_view_shown));
				list_.insert(std::pair<std::string, wxIcon>(
					"RenderView", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(default_shown));
				list_.insert(std::pair<std::string, wxIcon>(
					"default", icon));
			}
			else
			{
				icon.CopyFromBitmap(wxGetBitmapFromMemory(volume_data_hidden));
				list_.insert(std::pair<std::string, wxIcon>(
					"VolumeData", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(mesh_data_hidden));
				list_.insert(std::pair<std::string, wxIcon>(
					"MeshData", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(volume_group_hidden));
				list_.insert(std::pair<std::string, wxIcon>(
					"VolumeGroup", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(mesh_group_hidden));
				list_.insert(std::pair<std::string, wxIcon>(
					"MeshGroup", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(render_view_hidden));
				list_.insert(std::pair<std::string, wxIcon>(
					"RenderView", icon));
				icon.CopyFromBitmap(wxGetBitmapFromMemory(default_hidden));
				list_.insert(std::pair<std::string, wxIcon>(
					"default", icon));
			}
		}

		wxIcon get(const std::string &name) const
		{
			auto it = list_.find(name);
			if (it != list_.end())
				return it->second;
			else
			{
				it = list_.find("default");
				if (it != list_.end())
					return it->second;
			}
			return wxNullIcon;
		}

	private:
		std::unordered_map<std::string, wxIcon> list_;
	};
}

#endif//_ICONLIST_H_