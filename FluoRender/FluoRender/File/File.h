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
#ifndef _FILE_H_
#define _FILE_H_

#include <string>
#include <fstream>
#include <vector>

namespace flrd
{
	class File
	{
	public:
		File();
		virtual ~File();

		std::ofstream& getOfs()
		{
			return ofs_;
		}
		std::ifstream& getIfs()
		{
			return ifs_;
		}

		virtual void beginWrite(const std::string& filename);
		virtual void endWrite();
		virtual void beginRead(const std::string& filename);
		virtual void endRead();
		virtual void getPos();
		virtual void setPos();

		template<typename T>
		bool check(const T& t)
		{
			getPos();
			T tt;
			readValue(tt);
			if (t != tt)
			{
				setPos();
				return false;
			}
			return true;
		}

		//output
		void writeString(const std::string& s)
		{
			if (ofs_.bad()) return;
			ofs_.write(s.c_str(), s.size());
		}
		template<typename T>
		void writeVector(const std::vector<T>& v)
		{
			if (v.empty() || ofs_.bad())
				return;
			ofs_.write(reinterpret_cast<const char*>(&v[0]),
				v.size() * sizeof(T));
		}
		template<typename T>
		void writeValue(const T v)
		{
			if (ofs_.bad()) return;
			ofs_.write(reinterpret_cast<const char*>(&v), sizeof(v));
		}

		//input
		std::string readString(size_t l)
		{
			if (ifs_.bad()) return "";
			char* buf = new char[l + 1]();
			ifs_.read(buf, l);
			std::string str(buf);
			delete[] buf;
			return str;
		}
		template<typename T>
		void readVector(std::vector<T>& v)
		{
			if (ifs_.bad()) return;
			const size_t l = v.size();
			ifs_.read(reinterpret_cast<char*>(&v[0]), l * sizeof(T));
		}
		template<typename T>
		void readValue(T& v)
		{
			if (ifs_.bad()) return;
			ifs_.read(reinterpret_cast<char*>(&v), sizeof(T));
		}

	protected:
		int mode_;//0 invalid; 1 write; 2 read
		std::ofstream ofs_;
		std::ifstream ifs_;
		std::streampos pos_;
	};
}

#endif//_FILE_H_