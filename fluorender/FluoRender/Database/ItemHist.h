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
#ifndef _ITEMHIST_H_
#define _ITEMHIST_H_

#include <Item.h>
#include <vector>

namespace flrd
{
	class ItemHist : public Item
	{
		public:
			ItemHist();
			~ItemHist();

			void setRange(float min, float max)
			{
				m_min = min;
				m_max = max;
			}

			void setPopulation(unsigned int pop)
			{
				m_population = pop;
			}

			void setData(float* d)
			{
				m_data.assign(d, d + m_bins);
			}

			void setData(unsigned int* d)
			{
				//need normalization
				if (!m_population)
					return;
				for (size_t i = 0; i < m_bins; ++i)
					m_data[i] = float(d[i]) / float(m_population);
			}

			float* getData()
			{
				if (m_data.empty())
					return 0;
				return &(m_data[0]);
			}

		private:
			static unsigned int m_bins;//bin size
			float m_min;//min value
			float m_max;//max value
			unsigned int m_population;//sample size
			std::vector<float> m_data;//histogram, normalized
	};
}

#endif//_ITEMHIST_H_