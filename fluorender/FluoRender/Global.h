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
#ifndef GLOBAL_H
#define GLOBAL_H

#include <Tracking/VolCache.h>
#include <Database/TableHistParams.h>

#define glbin fluo::Global::instance()
#define glbin_cache_queue fluo::Global::instance().get_cache_queue()
#define glbin_reg_cache_queue_func(obj, read_func, del_func) \
	fluo::Global::instance().get_cache_queue().\
	RegisterCacheQueueFuncs(\
	std::bind(&read_func, obj, std::placeholders::_1),\
	std::bind(&del_func, obj, std::placeholders::_1))

namespace fluo
{
	class Global
	{
	public:
		static Global& instance() { return instance_; }

		flrd::CacheQueue& get_cache_queue() { return cache_queue_; }

		flrd::TableHistParams& get_ca_table() { return comp_analyzer_table_; }

	private:
		Global();
		static Global instance_;

		flrd::CacheQueue cache_queue_;

		flrd::TableHistParams comp_analyzer_table_;//records for learning comp analyzer settings
	};

}
#endif
