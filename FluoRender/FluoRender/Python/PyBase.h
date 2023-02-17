/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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
#ifndef PYBASE_H
#define PYBASE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <compatibility.h>
#include <future>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#ifdef __linux__
#include <dlfcn.h>
#endif

namespace flrd
{
	// Thread-safe queue
	template <typename T>
	class PyQueue
	{
	private:
		// Underlying queue
		std::queue<T> m_queue;

		// mutex for thread synchronization
		std::mutex m_mutex;

		// Condition variable for signaling
		std::condition_variable m_cond;

	public:
		// Pushes an element to the queue
		void push(T item)
		{

			// Acquire lock
			std::unique_lock<std::mutex> lock(m_mutex);

			// Add item
			m_queue.push(item);

			// Notify one thread that
			// is waiting
			m_cond.notify_one();
		}

		// Pops an element off the queue
		T pop()
		{

			// acquire lock
			std::unique_lock<std::mutex> lock(m_mutex);

			// wait until queue is not empty
			m_cond.wait(lock,
				[this]() { return !m_queue.empty(); });

			// retrieve item
			T item = m_queue.front();
			m_queue.pop();

			// return item
			return item;
		}
	};

	class PyBase
	{
	public:
		PyBase();
		~PyBase();

		enum OpType
		{
			ot_Initialize,
			ot_Run_SimpleString,
			ot_Quit
		};

		void SetInterval(int t)
		{
			m_interval = std::chrono::milliseconds(t);
		}
		int GetState()
		{
			return m_state;
		}
		virtual bool Init();
		virtual void Run(OpType func, const std::string& par = "");

	protected:
		//thread for running
		std::future<void> m_thread;
		std::atomic<int> m_state;//0-idle;1-busy;
		std::chrono::milliseconds m_interval;//intereval for query
		//message queue
		PyQueue <std::pair<OpType, std::string>> m_queue;

		static bool m_valid;//if get python
#ifdef _WIN32
		static HMODULE python_dll;//lib
		//functions
		static decltype(&Py_SetProgramName) SetProgramName;
		static decltype(&Py_Initialize) Initialize;
		static decltype(&PyDict_New) Dict_New;
		static decltype(&PyRun_SimpleString) Run_SimpleString;
		static decltype(&PyRun_String) Run_String;
		static decltype(&PyObject_Repr) Object_Repr;
		static decltype(&Py_FinalizeEx) FinalizeEx;
#else
		static void* python_dll;
		//functions
#endif
		virtual void ThreadFunc();
	private:
		bool SetValid(void* val)
		{
			m_valid = val != nullptr;
			return m_valid;
		}
	};
}

#endif//PYBASE_H
