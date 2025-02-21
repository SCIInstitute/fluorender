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
#ifndef _BASETREEFILE_H_
#define _BASETREEFILE_H_

#include <string>
#include <vector>
#include <type_traits>

class BaseTreeFile
{
public:
	virtual ~BaseTreeFile() {}

	virtual int LoadFile(const std::string& filename) = 0;
	virtual int LoadString(const std::string& str) = 0;
	virtual int SaveFile(const std::string& filename) = 0;
	virtual int SaveString(std::string& str) = 0;

	// Template methods to read values
	template<typename T>
	bool Read(const std::string& key, T* value) const {
		if constexpr (std::is_same_v<T, std::string>) {
			return ReadString(key, value);
		}
		else if constexpr (std::is_same_v<T, std::wstring>) {
			return ReadWstring(key, value);
		}
		else if constexpr (std::is_same_v<T, bool>) {
			return ReadBool(key, value);
		}
		else if constexpr (std::is_same_v<T, long>) {
			return ReadLong(key, value);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return ReadDouble(key, value);
		}
		else {
			static_assert(always_false<T>::value, "Unsupported type");
		}
	}

	// Template methods to write values
	template<typename T>
	bool Write(const std::string& key, const T& value) {
		if constexpr (std::is_same_v<T, std::string>) {
			return WriteString(key, value);
		}
		else if constexpr (std::is_same_v<T, std::wstring>) {
			return WriteWstring(key, value);
		}
		else if constexpr (std::is_same_v<T, bool>) {
			return WriteBool(key, value);
		}
		else if constexpr (std::is_same_v<T, long>) {
			return WriteLong(key, value);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return WriteDouble(key, value);
		}
		else {
			static_assert(always_false<T>::value, "Unsupported type");
		}
	}

	// Methods to manage groups
	virtual bool Exists(const std::string& path) const = 0;
	virtual bool SetPath(const std::string& path) = 0;
	virtual std::string GetPath() const = 0;
	virtual bool HasGroup(const std::string& group) const = 0;
	virtual bool HasEntry(const std::string& entry) const = 0;

	// Methods to enumerate entries and groups
	virtual bool GetFirstGroup(std::string* group, long* index) const = 0;
	virtual bool GetNextGroup(std::string* group, long* index) const = 0;
	virtual bool GetFirstEntry(std::string* entry, long* index) const = 0;
	virtual bool GetNextEntry(std::string* entry, long* index) const = 0;

	// Method to delete entries and groups
	virtual bool DeleteEntry(const std::string& key) = 0;
	virtual bool DeleteGroup(const std::string& group) = 0;

protected:
	// Type-specific read methods
	virtual bool ReadString(const std::string& key, std::string* value) const = 0;
	virtual bool ReadWstring(const std::string& key, std::wstring* value) const = 0;
	virtual bool ReadBool(const std::string& key, bool* value) const = 0;
	virtual bool ReadLong(const std::string& key, long* value) const = 0;
	virtual bool ReadDouble(const std::string& key, double* value) const = 0;

	// Type-specific write methods
	virtual bool WriteString(const std::string& key, const std::string& value) = 0;
	virtual bool WriteWstring(const std::string& key, const std::wstring& value) = 0;
	virtual bool WriteBool(const std::string& key, bool value) = 0;
	virtual bool WriteLong(const std::string& key, long value) = 0;
	virtual bool WriteDouble(const std::string& key, double value) = 0;

private:
	template<class T> struct always_false : std::false_type {};
};

#endif