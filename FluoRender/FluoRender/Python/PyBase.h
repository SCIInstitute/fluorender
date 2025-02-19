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
#ifndef PYBASE_H
#define PYBASE_H

#define PY_SSIZE_T_CLEAN
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
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

		bool empty()
		{
			// acquire lock
			std::unique_lock<std::mutex> lock(m_mutex);
			return m_queue.empty();
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
			ot_Run_SimpleStringEx,
			ot_Quit
		};

		static void SetHighVer(int ver)
		{
			m_high_ver = ver;
		}
		static void Free()
		{
#ifdef _WIN32
			if (python_dll)
				FreeLibrary(python_dll);
#else
			if (python_dll)
				dlclose(python_dll);
#endif
			m_valid = false;
		}
		void SetInterval(int t)
		{
			m_interval = std::chrono::milliseconds(t);
		}
		int GetState()
		{
			return m_state;
		}
		std::string GetOutput()
		{
			return m_output;
		}
		virtual bool Init();
		virtual void Run(OpType func, const std::string& par = "");
		virtual void Exit();

	protected:
		//thread for running
		std::future<void> m_thread;
		std::atomic<int> m_state;//0-idle;1-just created;2-busy
		std::string m_output;//console output from run string ex
		std::chrono::milliseconds m_interval;//intereval for query
		//message queue
		PyQueue <std::pair<OpType, std::string>> m_queue;

		static bool m_valid;//if get python
		static int m_high_ver;//highest version of python to search
#ifdef _WIN32
		static HMODULE python_dll;//lib
		//functions
		static decltype(&Py_Initialize) Initialize;
		static decltype(&PyRun_SimpleString) Run_SimpleString;
		static decltype(&Py_FinalizeEx) FinalizeEx;
		static decltype(&PyImport_AddModule) Import_AddModule;
		static decltype(&PyObject_GetAttrString) Object_GetAttrString;
		static decltype(&PyUnicode_AsEncodedString) Unicode_AsEncodedString;
		static decltype(&PyBytes_AsString) Bytes_AsString;
#else
		static void* python_dll;
		//functions
		typedef void(Initialize_tp)();
		static Initialize_tp* Initialize;
		typedef int(PyRun_SimpleString_tp)(const char *s);
		static PyRun_SimpleString_tp* Run_SimpleString;
		typedef int(Py_FinalizeEx_tp)();
		static Py_FinalizeEx_tp* FinalizeEx;
		typedef PyObject*(PyImport_AddModule_tp)(const char *name);
		static PyImport_AddModule_tp* Import_AddModule;
		typedef PyObject*(PyObject_GetAttrString_tp)(PyObject*, const char*);
		static PyObject_GetAttrString_tp* Object_GetAttrString;
		typedef PyObject* (PyUnicode_AsEncodedString_tp)(PyObject *unicode, const char *encoding, const char *errors);
		static PyUnicode_AsEncodedString_tp* Unicode_AsEncodedString;
		typedef char*(PyBytes_AsString_tp)(PyObject*);
		static PyBytes_AsString_tp* Bytes_AsString;
#endif

		virtual void ThreadFunc();

		void Run_SimpleStringEx(const std::string& str);

	private:
		bool SetValid(void* val)
		{
			m_valid = val != nullptr;
			return m_valid;
		}
		std::string GetPythonPath();
	};
}

#endif//PYBASE_H
