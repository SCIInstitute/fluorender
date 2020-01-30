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
#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <Annotations/AnnotationFactory.hpp>
#include <Mesh/MeshFactory.hpp>
#include <Volume/VolumeFactory.hpp>
#include <FUI/Panels/Base_Agent/AgentFactory.hpp>
#include <ProcessorFactory.hpp>

//#include <Fui/AgentFactory.h> // Will have to figure this out
//#include <Fui/IconList.h>     // Same here
//#include <../../Processor/ProcessorFactory.hpp>

namespace fluo
{
	//automatically creates the factories and provide global access
	class Global
	{
	public:
		static Global& instance() { return instance_; }

		inline size_t getNum()
		{
			size_t volume_num = volume_factory_->getNum();
			size_t mesh_num = mesh_factory_->getNum();
			size_t annotations_num = annotation_factory_->getNum();

			return volume_num + mesh_num + annotations_num;
		}

		inline Object* get(size_t i)
		{
			size_t volume_num = volume_factory_->getNum();
			size_t mesh_num = mesh_factory_->getNum();
			size_t annotations_num = annotation_factory_->getNum();

			if (i < volume_num)
				return volume_factory_->get(i);
			else if (i < volume_num + mesh_num)
				return mesh_factory_->get(i - volume_num);
			else if (i < volume_num + mesh_num + annotations_num)
				return annotation_factory_->get(i - volume_num - mesh_num);
			return 0;
		}

		inline size_t getIndex(const Object* object) const
		{
			size_t volume_num = volume_factory_->getNum();
			size_t mesh_num = mesh_factory_->getNum();
			size_t annotations_num = annotation_factory_->getNum();

			for (size_t i = 0; i < volume_num; ++i)
			{
				if (volume_factory_->get(i) == object)
					return i;
			}
			for (size_t i = 0; i < mesh_num; ++i)
			{
				if (mesh_factory_->get(i) == object)
					return i + volume_num;
			}
			for (size_t i = 0; i < annotations_num; ++i)
			{
				if (annotation_factory_->get(i) == object)
					return i + volume_num + mesh_num;
			}
			return volume_num + mesh_num + annotations_num;
		}

		VolumeFactory& getVolumeFactory()
		{ return *volume_factory_; }

		MeshFactory& getMeshFactory()
		{ return *mesh_factory_; }

		AnnotationFactory& getAnnotationFactory()
		{ return *annotation_factory_;}

        AgentFactory& getAgentFactory()
		{ return *agent_factory_; }
/*
		FUI::IconList& getIconList(bool shown)
		{ return shown?shown_icon_list_:hidden_icon_list_; }
*/
		void addListObserver(Observer* obsrvr)
		{
			volume_factory_->addObserver(obsrvr);
			mesh_factory_->addObserver(obsrvr);
			annotation_factory_->addObserver(obsrvr);
		}

		ProcessorFactory& getProcessorFactory()
		{ return *processor_factory_; }

	private:
		Global();

		static Global instance_;

		//objects in these will be shown in the list panel
		ref_ptr<VolumeFactory> volume_factory_;
		ref_ptr<MeshFactory> mesh_factory_;
		ref_ptr<AnnotationFactory> annotation_factory_;

		//list of agent
        ref_ptr<AgentFactory> agent_factory_;

		//list of processors
		ref_ptr<ProcessorFactory> processor_factory_;

		//icon list
//		FUI::IconList shown_icon_list_;
    //	FUI::IconList hidden_icon_list_;
	};
}

#endif
