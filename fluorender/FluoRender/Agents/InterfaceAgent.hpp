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
#ifndef _INTERFACEAGENT_H_
#define _INTERFACEAGENT_H_

#include <Names.hpp>
#include <Object.hpp>
#include <Node.hpp>
#include <ValueUpdateVisitor.hpp>

#define FOUND_VALUE(v) names.find(v) != names.end()

namespace fluo
{
	class AnnotationPropAgent;
	class BrushToolAgent;
	class CalculationAgent;
	class ClipPlaneAgent;
	class ClKernelAgent;
	class ColocalAgent;
	class ComponentAgent;
	class ConvertAgent;
	class CountingAgent;
	class ListModel;
	class MeasureAgent;
	class MeshPropAgent;
	class MeshTransAgent;
	class MovieAgent;
	class NoiseReduceAgent;
	class OutAdjustAgent;
	class RecorderAgent;
	class RenderCanvasAgent;
	class RenderFrameAgent;
	class RenderviewAgent;
	class SettingAgent;
	class TrackAgent;
	class TreeModel;
	class VolumePropAgent;
	class InterfaceAgent : public Object
	{
	public:
		InterfaceAgent()
		{
			addRefValue(gstAgentAsset, (Referenced*)0);
		}

		virtual InterfaceAgent* clone(const CopyOp& copyop) const { return 0; }

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const InterfaceAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "InterfaceAgent"; }

		//observer
		virtual unsigned int getPriority() const { return 200; }

		virtual void processNotification(Event& event)
		{
			if (event.getNotifyFlags() & Event::NOTIFY_AGENT ||
				event.getNotifyFlags() & Event::NOTIFY_SELF)
				Object::processNotification(event);
		}

		virtual void setObject(Object* obj)
		{
			Referenced* ref;
			getRefValue(gstAgentAsset, &ref);
			Object* old_obj = dynamic_cast<Object*>(ref);
			if (old_obj  &&
				old_obj == obj)
				return;

			if (old_obj)
				old_obj->removeObserver(this);
			clearValues(1);
			addSetRefValue(gstAgentAsset, obj);
			if (obj)
			{
				copyValues(*obj);//shallow copy to share values
				UpdateFui();
				obj->addObserver(this);
			}
		}
		virtual Object* getObject()
		{
			Referenced* ref;
			getRefValue(gstAgentAsset, &ref);
			return dynamic_cast<Object*>(ref);
		}

		virtual Node* getFirstParent()
		{
			Object* obj = getObject();
			if (obj)
			{
				Node* node = dynamic_cast<Node*>(obj);
				if (node)
					return node->getParent(0);
			}
			return 0;
		}

		virtual Node* getParent()
		{
			Object* obj = getObject();
			if (obj)
			{
				Node* node = dynamic_cast<Node*>(obj);
				if (node)
				{
					ParentList list = node->getParents();
					for (auto i : list)
					{
						if (i->asVolumeGroup() ||
							i->asMeshGroup() ||
							i->asRenderview())
							return i;
					}
				}
			}
			return 0;
		}

		virtual bool testSyncParentValue(const std::string& name)
		{
			Object* obj = getObject();
			Node* parent = getParent();
			if (obj && parent)
			{
				Value* value1 = obj->getValuePointer(name);
				Value* value2 = parent->getValuePointer(name);
				return value1->hasObserver(value2) &&
					value2->hasObserver(value1);
			}
			return false;
		}

		virtual bool testSyncParentValues(const ValueCollection &names)
		{
			Object* obj = getObject();
			Node* parent = getParent();
			if (obj && parent)
			{
				for (auto it = names.begin();
					it != names.end(); ++it)
				{
					Value* value1 = obj->getValuePointer(*it);
					Value* value2 = parent->getValuePointer(*it);
					if (!value1->hasObserver(value2) ||
						!value2->hasObserver(value1))
						return false;
				}
				return true;
			}
			return false;
		}

		virtual void syncParentValue(const std::string& name)
		{
			//get obj parent
			Node* parent = getParent();
			if (parent)
			{
				ValueUpdateVisitor update;
				update.setType(ValueUpdateVisitor::ValueUpdateVisitType::SYNC_VALUE);
				update.setValueName(name);
				parent->accept(update);
			}
		}

		virtual void unsyncParentValue(const std::string& name)
		{
			//get obj parent
			Node* parent = getParent();
			if (parent)
			{
				ValueUpdateVisitor update;
				update.setType(ValueUpdateVisitor::ValueUpdateVisitType::UNSYNC_VALUE);
				update.setValueName(name);
				parent->accept(update);
			}
		}

		virtual void syncParentValues(const ValueCollection &names)
		{
			//get obj parent
			Node* parent = getParent();
			if (parent)
			{
				ValueUpdateVisitor update;
				update.setType(ValueUpdateVisitor::ValueUpdateVisitType::SYNC_VALUES);
				update.setValueNames(names);
				parent->accept(update);
			}
		}

		virtual void unsyncParentValues(const ValueCollection &names)
		{
			//get obj parent
			Node* parent = getParent();
			if (parent)
			{
				ValueUpdateVisitor update;
				update.setType(ValueUpdateVisitor::ValueUpdateVisitType::UNSYNC_VALUES);
				update.setValueNames(names);
				parent->accept(update);
			}
		}

		virtual void propParentValue(const std::string& name)
		{
			//get obj parent
			Node* parent = getParent();
			if (parent)
			{
				ValueUpdateVisitor update;
				update.setType(ValueUpdateVisitor::ValueUpdateVisitType::PROP_VALUE);
				update.setValueName(name);
				update.setObject(this);
				parent->accept(update);
			}
		}

		virtual void propParentValues(const ValueCollection &names)
		{
			//get obj parent
			Node* parent = getParent();
			if (parent)
			{
				ValueUpdateVisitor update;
				update.setType(ValueUpdateVisitor::ValueUpdateVisitType::PROP_VALUES);
				update.setValueNames(names);
				update.setObject(this);
				parent->accept(update);
			}
		}

		virtual void UpdateFui(const ValueCollection &names = {}) = 0;
		virtual void resumeObserverNotificationAndUpdate()
		{
			UpdateFui();
			Referenced::resumeObserverNotification();
		}

		//convenient conversions
		virtual AnnotationPropAgent* asAnnotationPropAgent() { return 0; }
		virtual const AnnotationPropAgent* asAnnotationPropAgent() const { return 0; }
		virtual BrushToolAgent* asBrushToolAgent() { return 0; }
		virtual const BrushToolAgent* asBrushToolAgent() const { return 0; }
		virtual CalculationAgent* asCalculationAgent() { return 0; }
		virtual const CalculationAgent* asCalculationAgent() const { return 0; }
		virtual ClipPlaneAgent* asClipPlaneAgent() { return 0; }
		virtual const ClipPlaneAgent* asClipPlaneAgent() const { return 0; }
		virtual ClKernelAgent* asClKernelAgent() { return 0; }
		virtual const ClKernelAgent* asClKernelAgent() const { return 0; }
		virtual ColocalAgent* asColocalAgent() { return 0; }
		virtual const ColocalAgent* asColocalAgent() const { return 0; }
		virtual ComponentAgent* asComponentAgent() { return 0; }
		virtual const ComponentAgent* asComponentAgent() const { return 0; }
		virtual ConvertAgent* asConvertAgent() { return 0; }
		virtual const ConvertAgent* asConvertAgent() const { return 0; }
		virtual CountingAgent* asCountingAgent() { return 0; }
		virtual const CountingAgent* asCountingAgent() const { return 0; }
		virtual ListModel* asListModel() { return 0; }
		virtual const ListModel* asListModel() const { return 0; }
		virtual MeasureAgent* asMeasureAgent() { return 0; }
		virtual const MeasureAgent* asMeasureAgent() const { return 0; }
		virtual MeshPropAgent* asMeshPropAgent() { return 0; }
		virtual const MeshPropAgent* asMeshPropAgent() const { return 0; }
		virtual MeshTransAgent* asMeshTransAgent() { return 0; }
		virtual const MeshTransAgent* asMeshTransAgent() const { return 0; }
		virtual MovieAgent* asMovieAgent() { return 0; }
		virtual const MovieAgent* asMovieAgent() const { return 0; }
		virtual NoiseReduceAgent* asNoiseReduceAgent() { return 0; }
		virtual const NoiseReduceAgent* asNoiseReduceAgent() const { return 0; }
		virtual OutAdjustAgent* asOutAdjustAgent() { return 0; }
		virtual const OutAdjustAgent* asOutAdjustAgent() const { return 0; }
		virtual RecorderAgent* asRecorderAgent() { return 0; }
		virtual const RecorderAgent* asRecorderAgent() const { return 0; }
		virtual RenderCanvasAgent* asRenderCanvasAgent() { return 0; }
		virtual const RenderCanvasAgent* asRenderCanvasAgent() const { return 0; }
		virtual RenderFrameAgent* asRenderFrameAgent() { return 0; }
		virtual const RenderFrameAgent* asRenderFrameAgent() const { return 0; }
		virtual RenderviewAgent* asRenderviewAgent() { return 0; }
		virtual const RenderviewAgent* asRenderviewAgent() const { return 0; }
		virtual SettingAgent* asSettingAgent() { return 0; }
		virtual const SettingAgent* asSettingAgent() const { return 0; }
		virtual TrackAgent* asTrackAgent() { return 0; }
		virtual const TrackAgent* asTrackAgent() const { return 0; }
		virtual TreeModel* asTreeModel() { return 0; }
		virtual const TreeModel* asTreeModel() const { return 0; }
		virtual VolumePropAgent* asVolumePropAgent() { return 0; }
		virtual const VolumePropAgent* asVolumePropAgent() const { return 0; }

	protected:
		virtual void handleValueChanged(Event& event)
		{
			Referenced* refd = event.sender;
			Value* value = dynamic_cast<Value*>(refd);
			if (value)
			{
				ValueCollection names{ value->getName() };
				UpdateFui(names);
			}
		}
	};
}

#endif//_INTERFACEAGENT_H_