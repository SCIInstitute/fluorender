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
#ifndef _RECORD_H_
#define _RECORD_H_

#include <Entry.h>
#include <fstream>

namespace flrd
{
	class File;
	class RecordHistParams;
	class Record
	{
		public:
			Record();
			Record(const Record& rec);
			virtual ~Record();

			virtual RecordHistParams* asRecordHistParams() { return 0; }
			virtual const RecordHistParams* asRecordHistParams() const { return 0; }

			virtual Entry* getInput() = 0;
			virtual void setInput(Entry* entry) = 0;
			virtual Entry* getOutput() = 0;
			virtual void setOutput(Entry* entry) = 0;

			virtual void open(File& file);
			virtual void save(File& file);

			virtual size_t getInputSize() = 0;
			virtual size_t getOutputSize() = 0;

			virtual void getInputData(float* data) = 0;
			virtual void getOutputData(float* data) = 0;

			virtual void getInputData(std::vector<float>& data) = 0;
			virtual void getOutputData(std::vector<float>& data) = 0;

		protected:
			Entry* m_input;
			Entry* m_output;
	};
}

#endif//_RECORD_H_