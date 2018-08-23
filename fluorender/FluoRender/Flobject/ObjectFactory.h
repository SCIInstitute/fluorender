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
#ifndef FL_OBJECTFACTORY
#define FL_OBJECTFACTORY

#include <Flobject/Object.h>
#include <vector>
#include <string>

namespace FL
{
	class ObjectFactory : public Object
	{
	public:
		ObjectFactory();

		//don't copy itself
		virtual Object* clone(const CopyOp& copyop) const { return 0; }

		virtual bool isSameKindAs(const Object* obj) const
		{ return dynamic_cast<const ObjectFactory*>(obj) != NULL; }

		virtual const char* className() const { return "ObjectFactory"; }

		virtual Object* build();

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

				objects_.erase(objects_.begin() + pos, objects_.begin() + end);
			}
			return false;
		}

		inline size_t getNum() const { return objects_.size(); }

		inline ObjectList getAll()
		{
			ObjectList result;
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
				result.push_back((*it).get());
			return result;
		}

		inline Object* get(size_t i) { return objects_[i].get(); }

		inline const Object* get(size_t i) const { return objects_[i].get(); }

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

		inline Object* find(const unsigned int id)
		{
			for (auto it = objects_.begin();
				it != objects_.end(); ++it)
			{
				if ((*it)->getId() == id)
					return (*it).get();
			}
			return 0;
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

		inline bool rename(Object* obj, const std::string &name)
		{ if (obj) obj->setName(name); }

		virtual void clear()
		{ objects_.clear(); }

		inline void incCounter()
		{ ++global_id_; ++local_id_; }

	protected:
		virtual ~ObjectFactory();

		static unsigned int global_id_;
		unsigned int local_id_;

		std::vector<ref_ptr<Object>> objects_;
	};

}


#endif