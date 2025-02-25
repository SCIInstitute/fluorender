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

	virtual int LoadFile(const std::wstring& filename) = 0;
	virtual int LoadString(const std::string& str) = 0;
	virtual int SaveFile(const std::wstring& filename) = 0;
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

	template<typename T>
	bool Read(const std::string& key, T* value, const T& defaultVal) const {
		if constexpr (std::is_same_v<T, std::string>) {
			return ReadString(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, std::wstring>) {
			return ReadWstring(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, bool>) {
			return ReadBool(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, long>) {
			return ReadLong(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return ReadDouble(key, value, defaultVal);
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
	std::string cur_path_;
	std::string path_sep_;//separator for path
	std::string cd_sep_;//symbol for current dir
	std::string pd_sep_;//symbol for parent dir

protected:
	// Type-specific read methods
	virtual bool ReadString(const std::string& key, std::string* value, const std::string& def = "") const = 0;
	virtual bool ReadWstring(const std::string& key, std::wstring* value, const std::wstring& def = L"") const = 0;
	virtual bool ReadBool(const std::string& key, bool* value, bool def = false) const = 0;
	virtual bool ReadLong(const std::string& key, long* value, long def = 0) const = 0;
	virtual bool ReadDouble(const std::string& key, double* value, double def = 0.0) const = 0;

	// Type-specific write methods
	virtual bool WriteString(const std::string& key, const std::string& value) = 0;
	virtual bool WriteWstring(const std::string& key, const std::wstring& value) = 0;
	virtual bool WriteBool(const std::string& key, bool value) = 0;
	virtual bool WriteLong(const std::string& key, long value) = 0;
	virtual bool WriteDouble(const std::string& key, double value) = 0;

	//path processing
	std::vector<std::string> splitPath(const std::string& path) const {
		std::vector<std::string> parts;
		size_t start = 0, end = 0;
		while ((end = path.find(path_sep_, start)) != std::string::npos) {
			std::string part = path.substr(start, end - start);
			if (!part.empty()) {
				parts.push_back(part);
			}
			start = end + path_sep_.length();
		}
		std::string part = path.substr(start);
		if (!part.empty()) {
			parts.push_back(part);
		}
		return parts;
	}

	std::string joinPath(const std::vector<std::string>& parts) const {
		std::ostringstream oss;
		for (const auto& part : parts) {
			if (!oss.str().empty()) {
				oss << path_sep_;
			}
			oss << part;
		}
		return oss.str();
	}

	std::string normalizePath(const std::string& path) const {
		std::vector<std::string> parts = splitPath(path);
		std::vector<std::string> normalized_parts;
		for (const auto& part : parts) {
			if (part == "..") {
				if (!normalized_parts.empty()) {
					normalized_parts.pop_back();
				}
			}
			else if (part != ".") {
				normalized_parts.push_back(part);
			}
		}
		return joinPath(normalized_parts);
	}

	std::string getFullPath(const std::string& path) const {
		if (path.empty()) {
			return cur_path_ + path_sep_;
		}

		if (path.substr(0, path_sep_.length()) == path_sep_) {
			// Absolute path
			return normalizePath(path) + path_sep_;
		}

		std::vector<std::string> parts;
		if (path.substr(0, pd_sep_.length()) == pd_sep_) {
			// Handle relative path with ".."
			parts = splitPath(cur_path_);
			parts.pop_back(); // Go up one level
			parts.push_back(path.substr(pd_sep_.length()));
		}
		else if (path.substr(0, cd_sep_.length()) == cd_sep_) {
			// Handle relative path with "."
			parts = splitPath(cur_path_);
			parts.push_back(path.substr(cd_sep_.length()));
		}
		else {
			// Relative path
			parts = splitPath(cur_path_);
			parts.push_back(path);
		}

		return normalizePath(joinPath(parts)) + path_sep_;
	}

	std::string getFullKey(const std::string& key) const
	{
		return cur_path_.empty() ? key : cur_path_ + path_sep_ + key;
	}

private:
	template<class T> struct always_false : std::false_type {};
};

#endif