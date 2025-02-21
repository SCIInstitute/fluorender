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
#ifndef _JSONFILE_H_
#define _JSONFILE_H_

#include <BaseTreeFile.h>
#include <tiny-json.h>
#include <fstream>
#include <sstream>

class JsonFile : public BaseTreeFile {
public:
	JsonFile()
	{
	}

	~JsonFile()
	{
		//json_delete(json);
	}

	int LoadFile(const std::string& filename) override
	{
		//std::ifstream file(filename);
		//std::stringstream buffer;
		//buffer << file.rdbuf();
		//std::string str = buffer.str();
		//std::vector<char> mutableBuffer(str.begin(), str.end());
		//mutableBuffer.push_back('\0'); // Ensure null-termination
		//json = json_create(mutableBuffer.data(), mutableBuffer.size() - 1);
		//currentObject = json_getObject(json);

		return 1;
	}

	int LoadString(const std::string& ini_string) override
	{
		return 1;
	}

	int SaveFile(const std::string& filename) override
	{
		return 1;
	}

	int SaveString(std::string& str) override
	{
		return 1;
	}

	// Implement group management methods
	bool Exists(const std::string& path) const override
	{
		return false;
	}

	bool SetPath(const std::string& path) override
	{
		//currentObject = json_getObject(json_getProperty(json_getObject(json), path.c_str()));
		//return currentObject != nullptr;
		return false;
	}

	std::string GetPath() const override
	{
		return ""; // tiny-json does not support getting the current path directly
	}

	bool HasGroup(const std::string& group) const override
	{
		return json_getProperty(currentObject, group.c_str()) != nullptr;
	}

	bool HasEntry(const std::string& entry) const override
	{
		return json_getProperty(currentObject, entry.c_str()) != nullptr;
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override
	{
		json_t const* item = json_getChild(currentObject);
		if (item) {
			*group = json_getName(item);
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextGroup(std::string* group, long* index) const override
	{
		json_t const* item = json_getSibling(json_getChild(currentObject));
		for (long i = 0; i <= *index; ++i) {
			item = json_getSibling(item);
		}
		if (item) {
			*group = json_getName(item);
			(*index)++;
			return true;
		}
		return false;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override
	{
		json_t const* item = json_getChild(currentObject);
		if (item) {
			*entry = json_getName(item);
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextEntry(std::string* entry, long* index) const override
	{
		json_t const* item = json_getSibling(json_getChild(currentObject));
		for (long i = 0; i <= *index; ++i) {
			item = json_getSibling(item);
		}
		if (item) {
			*entry = json_getName(item);
			(*index)++;
			return true;
		}
		return false;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override
	{
		//json_t* item = json_getProperty(currentObject, key.c_str());
		//if (item) {
		//	json_removeProperty(currentObject, key.c_str());
		//	return true;
		//}
		return false;
	}

	bool DeleteGroup(const std::string& group) override
	{
		//json_t* item = json_getProperty(currentObject, group.c_str());
		//if (item) {
		//	json_removeProperty(currentObject, group.c_str());
		//	return true;
		//}
		return false;
	}

protected:
	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value) const override
	{
		json_t const* item = json_getProperty(currentObject, key.c_str());
		if (item && json_getType(item) == JSON_TEXT) {
			*value = json_getValue(item);
			return true;
		}
		return false;
	}

	bool ReadWstring(const std::string& key, std::wstring* value) const override
	{
		return false;
	}

	bool ReadBool(const std::string& key, bool* value) const override
	{
		return false;
	}

	bool ReadLong(const std::string& key, long* value) const override
	{
		json_t const* item = json_getProperty(currentObject, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = std::stol(json_getValue(item));
			return true;
		}
		return false;
	}

	bool ReadDouble(const std::string& key, double* value) const override
	{
		json_t const* item = json_getProperty(currentObject, key.c_str());
		if (item && json_getType(item) == JSON_REAL) {
			*value = std::stod(json_getValue(item));
			return true;
		}
		return false;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override
	{
		//json_t* item = json_getProperty(currentObject, key.c_str());
		//if (!item) {
		//	item = json_create(JSON_TEXT);
		//	json_addProperty(currentObject, key.c_str(), item);
		//}
		//json_setValue(item, value.c_str());
		return true;
	}

	bool WriteWstring(const std::string& key, const std::wstring& value) override
	{
		return true;
	}

	bool WriteBool(const std::string& key, bool value) override
	{
		return true;
	}

	bool WriteLong(const std::string& key, long value) override
	{
		//json_t* item = json_getProperty(currentObject, key.c_str());
		//if (!item) {
		//	item = json_create(JSON_INTEGER);
		//	json_addProperty(currentObject, key.c_str(), item);
		//}
		//json_setInteger(item, value);
		return true;
	}

	bool WriteDouble(const std::string& key, double value) override
	{
		//json_t* item = json_getProperty(currentObject, key.c_str());
		//if (!item) {
		//	item = json_create(JSON_REAL);
		//	json_addProperty(currentObject, key.c_str(), item);
		//}
		//json_setReal(item, value);
		return true;
	}

private:
	json_t* json;
	json_t* currentObject;
};

#endif//_JSONFILE_H_