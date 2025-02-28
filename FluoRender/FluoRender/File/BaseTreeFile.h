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

#include <Color.h>
#include <Point.h>
#include <Vector.h>
#include <string>
#include <vector>
#include <type_traits>
#include <unordered_map>

class BaseTreeFile
{
public:
	virtual ~BaseTreeFile() {}

	virtual int LoadFile(const std::wstring& filename) = 0;
	virtual int LoadString(const std::string& str) = 0;
	virtual int LoadData(const std::unordered_map<std::string, std::string>& data) = 0;
	virtual std::unordered_map<std::string, std::string> GetData() = 0;
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
		else if constexpr (std::is_same_v<T, unsigned long>) {
			return ReadULong(key, value);
		}
		else if constexpr (std::is_same_v<T, int>) {
			return ReadInt(key, value);
		}
		else if constexpr (std::is_same_v<T, unsigned int>) {
			return ReadUInt(key, value);
		}
		else if constexpr (std::is_same_v<T, size_t>) {
			return ReadSizeT(key, value);
		}
		else if constexpr (std::is_same_v<T, short>) {
			return ReadShort(key, value);
		}
		else if constexpr (std::is_same_v<T, unsigned short>) {
			return ReadUShort(key, value);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return ReadDouble(key, value);
		}
		else if constexpr (std::is_same_v<T, float>) {
			return ReadFloat(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::Color>) {
			return ReadColor(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::HSVColor>) {
			return ReadHSVColor(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::Point>) {
			return ReadPoint(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::Vector>) {
			return ReadVector(key, value);
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
		else if constexpr (std::is_same_v<T, unsigned long>) {
			return ReadULong(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, int>) {
			return ReadInt(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, unsigned int>) {
			return ReadUInt(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, size_t>) {
			return ReadSizeT(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, short>) {
			return ReadShort(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, unsigned short>) {
			return ReadUShort(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return ReadDouble(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, float>) {
			return ReadFloat(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, fluo::Color>) {
			return ReadColor(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, fluo::HSVColor>) {
			return ReadHSVColor(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, fluo::Point>) {
			return ReadPoint(key, value, defaultVal);
		}
		else if constexpr (std::is_same_v<T, fluo::Vector>) {
			return ReadVector(key, value, defaultVal);
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
		else if constexpr (std::is_same_v<T, unsigned long>) {
			return WriteULong(key, value);
		}
		else if constexpr (std::is_same_v<T, int>) {
			return WriteInt(key, value);
		}
		else if constexpr (std::is_same_v<T, unsigned int>) {
			return WriteUInt(key, value);
		}
		else if constexpr (std::is_same_v<T, size_t>) {
			return WriteSizeT(key, value);
		}
		else if constexpr (std::is_same_v<T, short>) {
			return WriteShort(key, value);
		}
		else if constexpr (std::is_same_v<T, unsigned short>) {
			return WriteUShort(key, value);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return WriteDouble(key, value);
		}
		else if constexpr (std::is_same_v<T, float>) {
			return WriteFloat(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::Color>) {
			return WriteColor(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::HSVColor>) {
			return WriteHSVColor(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::Point>) {
			return WritePoint(key, value);
		}
		else if constexpr (std::is_same_v<T, fluo::Vector>) {
			return WriteVector(key, value);
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
	virtual bool ReadULong(const std::string& key, unsigned long* value, unsigned long def = 0) const = 0;
	virtual bool ReadInt(const std::string& key, int* value, int def = 0) const = 0;
	virtual bool ReadUInt(const std::string& key, unsigned int* value, unsigned int def = 0) const = 0;
	virtual bool ReadSizeT(const std::string& key, size_t* value, size_t def = 0) const = 0;
	virtual bool ReadShort(const std::string& key, short* value, short def = 0) const = 0;
	virtual bool ReadUShort(const std::string& key, unsigned short* value, unsigned short def = 0) const = 0;
	virtual bool ReadDouble(const std::string& key, double* value, double def = 0.0) const = 0;
	virtual bool ReadFloat(const std::string& key, float* value, float def = 0.0f) const = 0;
	virtual bool ReadColor(const std::string& key, fluo::Color* value, const fluo::Color& def = fluo::Color(0.0)) const = 0;
	virtual bool ReadHSVColor(const std::string& key, fluo::HSVColor* value, const fluo::HSVColor& def = fluo::HSVColor()) const = 0;
	virtual bool ReadPoint(const std::string& key, fluo::Point* value, const fluo::Point& def = fluo::Point(0.0)) const = 0;
	virtual bool ReadVector(const std::string& key, fluo::Vector* value, const fluo::Vector& def = fluo::Vector(0.0)) const = 0;

	// Type-specific write methods
	virtual bool WriteString(const std::string& key, const std::string& value) = 0;
	virtual bool WriteWstring(const std::string& key, const std::wstring& value) = 0;
	virtual bool WriteBool(const std::string& key, bool value) = 0;
	virtual bool WriteLong(const std::string& key, long value) = 0;
	virtual bool WriteULong(const std::string& key, unsigned long value) = 0;
	virtual bool WriteInt(const std::string& key, int value) = 0;
	virtual bool WriteUInt(const std::string& key, unsigned int value) = 0;
	virtual bool WriteSizeT(const std::string& key, size_t value) = 0;
	virtual bool WriteShort(const std::string& key, short value) = 0;
	virtual bool WriteUShort(const std::string& key, unsigned short value) = 0;
	virtual bool WriteDouble(const std::string& key, double value) = 0;
	virtual bool WriteFloat(const std::string& key, float value) = 0;
	virtual bool WriteColor(const std::string& key, const fluo::Color& value) = 0;
	virtual bool WriteHSVColor(const std::string& key, const fluo::HSVColor& value) = 0;
	virtual bool WritePoint(const std::string& key, const fluo::Point& value) = 0;
	virtual bool WriteVector(const std::string& key, const fluo::Vector& value) = 0;

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
		return cur_path_.empty() ? path_sep_ + key : cur_path_ + key;
	}

private:
	template<class T> struct always_false : std::false_type {};
};

#endif