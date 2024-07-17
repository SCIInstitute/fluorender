/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _TABLE_H_
#define _TABLE_H_

#include <Record.h>
#include <vector>
#include <string>
#include <ctime>
#include <functional>
#include <Params.h>

namespace flrd
{
	typedef std::function<void(int)> TableUpdateFunc;
	class Table
	{
		public:
			enum TableTags
			{
				TAG_TABLE_REC_NUM = 1,
				TAG_TABLE_REC_HISTPARAM,
				TAG_TABLE_ENT_IN,
				TAG_TABLE_ENT_OUT,
				TAG_TABLE_ENT_HIST,
				TAG_TABLE_ENT_PARAMS,
				TAG_TABLE_TIME_CREATE,
				TAG_TABLE_TIME_MODIFY,
				TAG_TABLE_NOTES,
				TAG_TABLE_NOTE_SIZE,
				TAG_TABLE_NAME,
				TAG_TABLE_NAME_SIZE,
				TAG_TABLE_TRAINED_NUM
			};

			Table();
			Table(const Table& table);
			virtual ~Table();

			virtual void clear();
			virtual void addRecord(Record* rec);
			virtual void readRecord(Record* rec);
			virtual void delRecord(size_t i);
			virtual void delRecords(std::vector<size_t>& vi);
			virtual void setCreateTime(const std::time_t& t);
			virtual std::time_t* getCreateTime()
			{
				return &m_create_time;
			}
			virtual std::time_t* getModifyTime()
			{
				return &m_modify_time;
			}
			virtual void setNotes(const std::string& text)
			{
				m_notes = text;
				setModified();
			}
			virtual std::string getNotes()
			{
				return m_notes;
			}
			virtual void setModified()
			{
				m_modify_time = std::time(0);
				m_modified = true;
				if (m_update_func)
					m_update_func(3);
			}
			virtual bool getModified()
			{
				return m_modified;
			}
			virtual void setName(const std::string& name)
			{
				m_name = name;
				setModified();
			}
			virtual std::string getName()
			{
				return m_name;
			}

			virtual void open(const std::string& filename, bool info = false);
			virtual void save(const std::string& filename);

			virtual size_t getRecSize()
			{
				return m_data.size();
			}
			virtual size_t getRecNum()
			{
				return m_recnum;
			}
			virtual void getRecInput(float* data)
			{
				float* p = data;
				for (auto i : m_data)
				{
					i->getInputData(p);
					p += i->getInputSize();
				}
			}
			virtual void getRecOutput(float* data)
			{
				float* p = data;
				for (auto i : m_data)
				{
					i->getOutputData(p);
					p += i->getOutputSize();
				}
			}
			virtual void getOneInput(size_t i, std::vector<float>& data)
			{
				if (i < m_data.size())
					m_data[i]->getInputData(data);
			}
			virtual void getOneOutput(size_t i, std::vector<float>& data)
			{
				if (i < m_data.size())
					m_data[i]->getOutputData(data);
			}

			void setUpdateFunc(TableUpdateFunc func)
			{
				m_update_func = func;
			}

			void setParams(Params* params)
			{
				m_params = params;
			}

		protected:
			bool m_modified;
			std::time_t m_create_time;
			std::time_t m_modify_time;
			std::string m_name;
			std::string m_notes;
			size_t m_recnum;
			std::vector<Record*> m_data;
			Params* m_params;//type of records
			//trainer
			size_t m_trained_rec_num;

			//update
			TableUpdateFunc m_update_func;
	};
}

#endif//_TABLE_H_