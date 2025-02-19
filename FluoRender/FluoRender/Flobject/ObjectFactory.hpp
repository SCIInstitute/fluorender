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
#ifndef FL_OBJECTFACTORY_HPP
#define FL_OBJECTFACTORY_HPP

#include <Node.hpp>
#include <deque>
#include <string>
#include <iostream>

// TODO: Replace with QT XML Parser
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace fluo
{
	class ObjectFactory : public Node
	{
	public:
		ObjectFactory();

		//don't copy itself
        virtual Object* clone(const CopyOp& copyop) const { return nullptr; }

		virtual bool isSameKindAs(const Object* obj) const
        { return dynamic_cast<const ObjectFactory*>(obj) != nullptr; }

		virtual const char* className() const { return "ObjectFactory"; }

		virtual void createDefault();

        virtual void setEventHandler(Object* obj) {}

		virtual unsigned int getPriority() const { return 300; }

		//as observer
        virtual void processNotification(Event& event)
		{
            if (event.getNotifyFlags() & Event::NOTIFY_FACTORY)
                Object::processNotification(event);
		}

		//propagate values from object to the default
		virtual void propValuesToDefault(Object*, const ValueCollection &names = {});
		virtual void propValuesFromDefault(Object*, const ValueCollection &names = {});
		//read default settings for object
		//to take advantage of the value management system,
		//create a default object and use it to save settings
		bool readDefault(std::istream &is, const ValueCollection &names = {});
		bool writeDefault(std::ostream &os, const ValueCollection &names = {}, int indent = 1);
		bool readDefault(const ValueCollection &names = {});
		bool writeDefault(const ValueCollection &names = {});

		virtual Object* getDefault()
		{ return findFirst(default_object_name_); }

		//to control how an object is built, provide a "prototype" object with values
		//otherwise, it's default to null object and an object is cloned from the default object
        virtual Object* build(Object* obj = nullptr);

		virtual Object* clone(Object*);

		virtual Object* clone(const unsigned int);

		inline bool remove(Object* object)
		{
			size_t pos = getIndex(object);
			if (pos < objects_.size())
				return remove(pos, 1);
			else
				return false;
		}

		inline bool remove(size_t pos, size_t num = 1)
		{
			if (pos < objects_.size() && num > 0)
			{
				size_t end = pos + num;
				if (end > objects_.size())
					end = objects_.size();

				//notify observers of change
				for (auto it = objects_.begin() + pos;
					it != objects_.begin() + end; ++it)
				{
					Event event;
					event.init(Event::EVENT_NODE_REMOVED,
						this, it->get());
					notifyObservers(event);
				}

				objects_.erase(objects_.begin() + pos, objects_.begin() + end);
			}
			return false;
		}

		inline size_t getNum(bool include_default = false) const
		{
			if (include_default)
				return objects_.size();
			else
				return objects_.empty() ? 0 : objects_.size() - 1;
		}

		inline ObjectList getAll(bool include_default = false)
		{
			ObjectList result;
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if ((*it)->getName() != default_object_name_)
					result.push_back((*it).get());
			}
			return result;
		}

		inline virtual Object* get(size_t i) { return objects_[i].get(); }

		inline virtual const Object* get(size_t i) const { return objects_[i].get(); }

		inline virtual Object* getLast()
		{
			return objects_.front().get();
		}

		inline bool contains(const Object* object) const
		{
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if (*it == object)
					return true;
			}
			return false;
		}

		inline size_t getIndex(const Object* object) const
		{
			for (size_t i = 0; i < objects_.size(); ++i)
			{
				if (objects_[i] == object)
					return i;
			}
			return objects_.size();
		}

		inline virtual Object* find(const unsigned int id)
		{
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if ((*it)->getId() == id)
					return (*it).get();
			}
            return nullptr;
		}

		inline virtual Object* findFirst(const std::string &name)
		{
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if ((*it)->getName() == name)
					return (*it).get();
			}
            return nullptr;
		}

		inline virtual Object* findLast(const std::string &name)
		{
			for (auto it = objects_.rbegin();
				it != objects_.rend(); ++it)
			{
				if ((*it)->getName() == name)
					return (*it).get();
			}
            return nullptr;
		}

		inline ObjectList find(const std::string &name)
		{
			ObjectList result;
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if ((*it)->getName() == name)
					result.push_back((*it).get());
			}
			return result;
		}

		//find by matching values of two objects
		inline ObjectList findByValues(const Object& obj)
		{
			ObjectList result;
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if (it->get()->cmpValues(obj))
					result.push_back(it->get());
			}
			return result;
		}

		inline virtual Object* findFirstByValues(const Object& obj)
		{
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if (it->get()->cmpValues(obj))
					return it->get();
			}
            return nullptr;
		}

		inline virtual Object* findLastByValues(const Object& obj)
		{
			for (auto it = objects_.rbegin();
				it != objects_.rend(); ++it)
			{
				if (it->get()->cmpValues(obj))
					return it->get();
			}
            return nullptr;
		}

		//if we want to resolve naming conflicts, more code is needed
		inline bool rename(Object* obj, const std::string &name)
		{ if (obj) obj->setName(name); }

		virtual void clear(bool clear_default = false)
		{
			if (clear_default)
				objects_.clear();
			else
			{
				for (auto it = objects_.begin();
					it != objects_.end();)
				{
					if ((*it)->getName() != default_object_name_)
						it = objects_.erase(it);
					else
						++it;
				}
			}
		}

		inline void incCounter()
		{ ++global_id_; ++local_id_; }

	protected:
		virtual ~ObjectFactory();

		//update the values of the default volume
		bool setDefaultValues(boost::property_tree::ptree &pt, const ValueCollection &names);
		//convert the values of the default volume to ptree
		bool convDefaultValues(boost::property_tree::ptree &pt, const ValueCollection &names);
		//replace values in an existing tree, assuming values are immediate children of the tree
		bool replaceDefaultValues(boost::property_tree::ptree &pt, const ValueCollection &names);

		std::string default_object_name_;
		std::string default_setting_filename_value_name_;
		static unsigned int global_id_;
		unsigned int local_id_;

		//reserve the LAST object for default
		//objects created later can be cloned from the default
		std::deque<ref_ptr<Object>> objects_;
	};

}


#endif
