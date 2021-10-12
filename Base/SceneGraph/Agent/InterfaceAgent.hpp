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

namespace fluo
{
class InterfaceAgent : public Object
{
  public:
    InterfaceAgent(){}

    virtual InterfaceAgent* clone(const CopyOp& copyop) const { return nullptr; }

    virtual bool isSameKindAs(const Object* obj) const
    {
      return dynamic_cast<const InterfaceAgent*>(obj) != nullptr;
    }

    virtual const char* className() const { return "InterfaceAgent"; }

    //observer
    virtual unsigned int getPriority() const { return 200; }

    virtual void processNotification(Event& event)
    {
      if (event.getNotifyFlags() & Event::NOTIFY_AGENT)
        Object::processNotification(event);
    }

    virtual void setObject(Object* obj)
    {
      Object* old_obj = nullptr;
	  Referenced* ref;
	  if (getRvalu("asset", &ref))
	  {
		  old_obj = dynamic_cast<Object*>(ref);
		  if (old_obj == obj)
		  return;
	  }

      if (old_obj)
        old_obj->removeObserver(this);

      clearValues();
      addValue("asset", obj);

      if (obj)
      {
        copyValues(*obj);//shallow copy to share values
        //UpdateAllSettings();
        obj->addObserver(this);
      }
    }

    virtual Object* getObject()
    {
	  Referenced* ref;
      if (getRvalu("asset", &ref))
		return dynamic_cast<Object*>(ref);
	  return nullptr;
    }

    virtual Node* getObjParent()
    {
      Object* obj = getObject();
      if (obj)
      {
        Node* node = dynamic_cast<Node*>(obj);

        if (node)
          return node->getParent(0);
      }
      return nullptr;
    }

    /*
    virtual bool testSyncParentValue(const std::string& name)
    {
      Object* obj = getObject();
      Node* parent = getObjParent();

      if (obj && parent)
      {
        Value* value1 = obj->getValue(name);
        Value* value2 = parent->getValue(name);
        return value1->hasObserver(value2) && value2->hasObserver(value1);
      }
      return false;
    }

    virtual bool testSyncParentValues(const ValueCollection &names)
    {
      Object* obj = getObject();
      Node* parent = getObjParent();

      if (obj && parent)
      {
        for (auto it = names.begin(); it != names.end(); ++it)
        {
          Value* value1 = obj->getValue(*it);
          Value* value2 = parent->getValue(*it);

          if (!value1->hasObserver(value2) || !value2->hasObserver(value1))
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
      Node* parent = getObjParent();

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
      Node* parent = getObjParent();

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
      Node* parent = getObjParent();

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
      Node* parent = getObjParent();
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
      Node* parent = getObjParent();

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
      Node* parent = getObjParent();

      if (parent)
      {
        ValueUpdateVisitor update;
        update.setType(ValueUpdateVisitor::ValueUpdateVisitType::PROP_VALUES);
        update.setValueNames(names);
        update.setObject(this);
        parent->accept(update);
      }
    }

    virtual void UpdateAllSettings() {}
};
}

#endif//_INTERFACEAGENT_H_
