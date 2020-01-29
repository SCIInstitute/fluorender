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
#ifndef INTERFACE_AGENT_HPP
#define INTERFACE_AGENT_HPP

#include <Object.hpp>
#include <Node.hpp>
#include <ValueUpdateVisitor.hpp>

    class InterfaceAgent : public fluo::Object
	{
	public:
		InterfaceAgent()
		{
		}

        virtual InterfaceAgent* clone(const fluo::CopyOp& copyop) const { return nullptr; }

        virtual bool isSameKindAs(const fluo::Object* obj) const
		{
            return dynamic_cast<const InterfaceAgent*>(obj) != nullptr;
		}

		virtual const char* className() const { return "InterfaceAgent"; }

		//observer
		virtual unsigned int getPriority() const { return 200; }

        virtual void processNotification(fluo::Event& event)
		{
            if (event.getNotifyFlags() & fluo::Event::NOTIFY_AGENT)
                fluo::Object::processNotification(event);
		}

        virtual void setObject(fluo::Object* obj)
		{
            fluo::Object* old_obj = nullptr;
            if (getValue("asset", (fluo::Referenced**)&old_obj) &&
				old_obj == obj)
				return;

			if (old_obj)
				old_obj->removeObserver(this);
			clearValues();
			addValue("asset", obj);
			if (obj)
			{
				copyValues(*obj);//shallow copy to share values
				UpdateAllSettings();
				obj->addObserver(this);
			}
		}
        virtual fluo::Object* getObject()
		{
            fluo::Object* obj = nullptr;
            getValue("asset", (fluo::Referenced**)&obj);
			return obj;
		}

        virtual fluo::Node* getObjParent()
		{
            fluo::Object* obj = getObject();
			if (obj)
			{
                fluo::Node* node = dynamic_cast<fluo::Node*>(obj);
				if (node)
					return node->getParent(0);
			}
            return nullptr;
		}
/*
		virtual bool testSyncParentValue(const std::string& name)
		{
            fluo::Object* obj = getObject();
            fluo::Node* parent = getObjParent();
			if (obj && parent)
			{
                fluo::Value* value1 = obj->getValue(name);
                fluo::Value* value2 = parent->getValue(name);
				return value1->hasObserver(value2) &&
					value2->hasObserver(value1);
			}
			return false;
		}

        virtual bool testSyncParentValues(const fluo::ValueCollection &names)
		{
            fluo::Object* obj = getObject();
            fluo::Node* parent = getObjParent();
			if (obj && parent)
			{
				for (auto it = names.begin();
					it != names.end(); ++it)
				{
                    fluo::Value* value1 = obj->getValue(*it);
                    fluo::Value* value2 = parent->getValue(*it);
					if (!value1->hasObserver(value2) ||
						!value2->hasObserver(value1))
						return false;
				}
				return true;
			}
			return false;
		}
*/
		virtual void syncParentValue(const std::string& name)
		{
			//get obj parent
            fluo::Node* parent = getObjParent();
			if (parent)
			{
                fluo::ValueUpdateVisitor update;
                update.setType(fluo::ValueUpdateVisitor::ValueUpdateVisitType::SYNC_VALUE);
				update.setValueName(name);
				parent->accept(update);
			}
		}

		virtual void unsyncParentValue(const std::string& name)
		{
			//get obj parent
            fluo::Node* parent = getObjParent();
			if (parent)
			{
                fluo::ValueUpdateVisitor update;
                update.setType(fluo::ValueUpdateVisitor::ValueUpdateVisitType::UNSYNC_VALUE);
				update.setValueName(name);
				parent->accept(update);
			}
		}

        virtual void syncParentValues(const fluo::ValueCollection &names)
		{
			//get obj parent
            fluo::Node* parent = getObjParent();
			if (parent)
			{
                fluo::ValueUpdateVisitor update;
                update.setType(fluo::ValueUpdateVisitor::ValueUpdateVisitType::SYNC_VALUES);
				update.setValueNames(names);
				parent->accept(update);
			}
		}

        virtual void unsyncParentValues(const fluo::ValueCollection &names)
		{
			//get obj parent
            fluo::Node* parent = getObjParent();
			if (parent)
			{
                fluo::ValueUpdateVisitor update;
                update.setType(fluo::ValueUpdateVisitor::ValueUpdateVisitType::UNSYNC_VALUES);
				update.setValueNames(names);
				parent->accept(update);
			}
		}

		virtual void propParentValue(const std::string& name)
		{
			//get obj parent
            fluo::Node* parent = getObjParent();
			if (parent)
			{
                fluo::ValueUpdateVisitor update;
                update.setType(fluo::ValueUpdateVisitor::ValueUpdateVisitType::PROP_VALUE);
				update.setValueName(name);
				update.setObject(this);
				parent->accept(update);
			}
		}

        virtual void propParentValues(const fluo::ValueCollection &names)
		{
			//get obj parent
            fluo::Node* parent = getObjParent();
			if (parent)
			{
                fluo::ValueUpdateVisitor update;
                update.setType(fluo::ValueUpdateVisitor::ValueUpdateVisitType::PROP_VALUES);
				update.setValueNames(names);
				update.setObject(this);
				parent->accept(update);
			}
		}

        virtual void UpdateAllSettings() {}

	protected:
	};

#endif//_INTERFACEAGENT_H_
