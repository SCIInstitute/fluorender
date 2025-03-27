﻿/*
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
#include <compatibility.h>
#include <tiny-json.h>
#include <fstream>
#include <sstream>

class JsonFile : public BaseTreeFile {
public:
	JsonFile() :
		json_(0),
		cur_obj_(0)
	{
		path_sep_ = "/";
		cd_sep_ = ".";
		pd_sep_ = "..";

		//root
		json_t mem[32];
		char empty_json[] = R"({})";
		json_ = const_cast<json_t*>(json_create(empty_json, mem, sizeof mem / sizeof * mem));
		cur_obj_ = json_;
	}

	~JsonFile()
	{
	}

	int LoadFile(const std::wstring& filename) override
	{
#ifdef _WIN32
		std::wstring long_name = L"\x5c\x5c\x3f\x5c" + filename;
#else
		std::wstring long_name = filename;
#endif
		std::ifstream file(long_name);
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string str = buffer.str();
		json_t mem[32];
		json_ = const_cast<json_t*>(json_create(str.data(), mem, sizeof mem / sizeof * mem));
		cur_obj_ = json_;

		return json_ == 0;
	}

	int LoadString(const std::string& ini_string) override
	{
		json_t mem[32];
		std::string str = ini_string;
		json_ = const_cast<json_t*>(json_create(str.data(), mem, sizeof mem / sizeof * mem));
		cur_obj_ = json_;

		return json_ == 0;
	}

	int LoadData(const std::unordered_map<std::string, std::string>& data) override
	{
		//dictionary_ = data;
		return 0;
	}

	std::unordered_map<std::string, std::string> GetData() override
	{
		std::unordered_map<std::string, std::string> data;
		return data;
	}

	int SaveFile(const std::wstring& filename) override
	{
		if (json_ == nullptr) {
			return 1; // Error: No JSON data to save
		}

#ifdef _WIN32
		std::wstring long_name = L"\x5c\x5c\x3f\x5c" + filename;
#else
		std::wstring long_name = filename;
#endif
		std::ofstream file(long_name);
		if (!file.is_open()) {
			return 6; // Error: Unable to open file
		}

		std::string jsonString = json_stringify(json_);
		file << jsonString;
		file.close();

		return 0; // Success
	}

	int SaveString(std::string& str) override
	{
		if (json_ == nullptr) {
			return 1; // Error: No JSON data to save
		}

		str = json_stringify(json_);
		return 0; // Success
	}

	// Implement group management methods
	bool Exists(const std::string& path) const override
	{
		// Normalize the path to handle relative and absolute paths
		std::string normalized_path = getFullPath(path);
		std::vector<std::string> components = splitPath(normalized_path);

		// Start from the root element or current element based on whether the path is absolute or relative
		json_t* element = const_cast<json_t*>((normalized_path.substr(0, path_sep_.length()) == path_sep_) ? json_ : cur_obj_);
		std::vector<json_t*> temp_stack = element_stack_;

		// Traverse the path components
		for (const auto& component : components) {
			if (component == cd_sep_) {
				// Current level, do nothing
				continue;
			}
			else if (component == pd_sep_) {
				// Parent level, move to parent element if possible
				if (!temp_stack.empty()) {
					element = temp_stack.back();
					temp_stack.pop_back();
				}
				else {
					return false; // No parent element, path is invalid
				}
			}
			else {
				// Move to the child element with the given name
				temp_stack.push_back(element);
				element = const_cast<json_t*>(json_getProperty(element, component.c_str()));
				if (!element) {
					return false; // Element not found, path is invalid
				}
			}
		}

		return true; // Path exists
	}

	bool SetPath(const std::string& path) override
	{
		// Normalize the path to handle relative and absolute paths
		std::string normalized_path = normalizePath(path);
		std::vector<std::string> components = splitPath(normalized_path);

		// Determine the starting element
		json_t* element = nullptr;
		std::string new_path;
		std::vector<json_t*> new_stack;

		if (normalized_path.substr(0, path_sep_.length()) == path_sep_) {
			// Absolute path
			element = const_cast<json_t*>(json_);
			new_path = path_sep_;
		}
		else {
			// Relative path
			element = const_cast<json_t*>(cur_obj_);
			new_path = cur_path_;
			new_stack = element_stack_;
		}

		// Traverse the path components
		for (const auto& component : components) {
			if (component == cd_sep_) {
				// Current level, do nothing
				continue;
			}
			else if (component == pd_sep_) {
				// Parent level, move to parent element if possible
				if (!new_stack.empty()) {
					element = new_stack.back();
					new_stack.pop_back();
					size_t pos = new_path.find_last_of(path_sep_);
					if (pos != std::string::npos) {
						new_path = new_path.substr(0, pos);
					}
				}
				else {
					return false; // No parent element, path is invalid
				}
			}
			else {
				// Move to the child element with the given name
				new_stack.push_back(element);
				json_t* child = const_cast<json_t*>(json_getProperty(element, component.c_str()));
				if (!child) {
					// Create a new child element if it doesn't exist
					child = (json_t*)malloc(sizeof(json_t));
					child->name = strdup(component.c_str());
					child->type = JSON_OBJ;
					child->u.c.child = nullptr;
					child->u.c.last_child = nullptr;
					child->sibling = nullptr;
					addProperty(element, component, child);
				}
				element = child;
				new_path += path_sep_ + component;
			}
		}

		// Update cur_obj_, cur_path_, and element_stack_
		cur_obj_ = element;
		cur_path_ = new_path;
		element_stack_ = new_stack;

		return true; // Path exists or was created, and cur_obj_, cur_path_, and element_stack_ are updated
	}

	std::string GetPath() const override
	{
		return cur_path_;
	}

	bool HasGroup(const std::string& group) const override
	{
		if (!cur_obj_) {
			return false;
		}
		const json_t* child = json_getProperty(cur_obj_, group.c_str());
		return child != nullptr && json_getType(child) == JSON_OBJ;
	}

	bool HasEntry(const std::string& entry) const override
	{
		if (!cur_obj_) {
			return false;
		}
		const json_t* child = json_getProperty(cur_obj_, entry.c_str());
		return child != nullptr;
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override
	{
		if (!cur_obj_) {
			return false;
		}
		const json_t* item = json_getChild(cur_obj_);
		while (item && json_getType(item) != JSON_OBJ) {
			item = json_getSibling(item);
		}
		if (item) {
			*group = json_getName(item);
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextGroup(std::string* group, long* index) const override
	{
		if (!cur_obj_) {
			return false;
		}
		const json_t* item = json_getChild(cur_obj_);
		for (long i = 0; i <= *index; ++i) {
			item = json_getSibling(item);
		}
		while (item && json_getType(item) != JSON_OBJ) {
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
		if (!cur_obj_) {
			return false;
		}
		const json_t* item = json_getChild(cur_obj_);
		if (item) {
			*entry = json_getName(item);
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextEntry(std::string* entry, long* index) const override
	{
		if (!cur_obj_) {
			return false;
		}
		const json_t* item = json_getChild(cur_obj_);
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
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* prev = nullptr;
		json_t* item = parent->u.c.child;
		while (item) {
			if (strcmp(item->name, key.c_str()) == 0) {
				if (prev) {
					prev->sibling = item->sibling;
				}
				else {
					parent->u.c.child = item->sibling;
				}
				if (item == parent->u.c.last_child) {
					parent->u.c.last_child = prev;
				}
				// Free the memory associated with the item
				free((void*)item->name);
				if (item->type == JSON_TEXT) {
					free((void*)item->u.value);
				}
				free(item);
				return true;
			}
			prev = item;
			item = item->sibling;
		}
		return false;
	}

	bool DeleteGroup(const std::string& group) override
	{
		return DeleteEntry(group); // Groups and entries are handled similarly
	}

protected:
	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value, const std::string& def = "") const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_TEXT) {
			*value = json_getValue(item);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadWstring(const std::string& key, std::wstring* value, const std::wstring& def = L"") const override
	{
		std::string str;
		if (ReadString(key, &str)) {
			*value = s2ws(str);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadBool(const std::string& key, bool* value, bool def = false) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_BOOLEAN) {
			*value = json_getBoolean(item);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadLong(const std::string& key, long* value, long def = 0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = static_cast<long>(json_getInteger(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadULong(const std::string& key, unsigned long* value, unsigned long def = 0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = static_cast<unsigned long>(json_getInteger(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadInt(const std::string& key, int* value, int def = 0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = static_cast<int>(json_getInteger(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadUInt(const std::string& key, unsigned int* value, unsigned int def = 0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = static_cast<unsigned int>(json_getInteger(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadSizeT(const std::string& key, size_t* value, size_t def = 0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = static_cast<size_t>(json_getInteger(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadShort(const std::string& key, short* value, short def = 0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = static_cast<short>(json_getInteger(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadUShort(const std::string& key, unsigned short* value, unsigned short def = 0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_INTEGER) {
			*value = static_cast<unsigned short>(json_getInteger(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadDouble(const std::string& key, double* value, double def = 0.0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_REAL) {
			*value = json_getReal(item);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadFloat(const std::string& key, float* value, float def = 0.0) const override
	{
		if (!cur_obj_) {
			*value = def;
			return false;
		}
		json_t const* item = json_getProperty(cur_obj_, key.c_str());
		if (item && json_getType(item) == JSON_REAL) {
			*value = static_cast<float>(json_getReal(item));
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadColor(const std::string& key, fluo::Color* value, const fluo::Color& def = fluo::Color(0.0)) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = fluo::Color(str);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadHSVColor(const std::string& key, fluo::HSVColor* value, const fluo::HSVColor& def = fluo::HSVColor()) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = fluo::HSVColor(str);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadPoint(const std::string& key, fluo::Point* value, const fluo::Point& def = fluo::Point(0.0)) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = fluo::Point(str);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadVector(const std::string& key, fluo::Vector* value, const fluo::Vector& def = fluo::Vector(0.0)) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = fluo::Vector(str);
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadQuaternion(const std::string& key, fluo::Quaternion* value, const fluo::Quaternion& def = fluo::Quaternion()) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = fluo::Quaternion(str);
			return true;
		}
		*value = def;
		return false;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override
	{
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* item = const_cast<json_t*>(json_getProperty(parent, key.c_str()));
		if (!item) {
			item = (json_t*)malloc(sizeof(json_t));
			item->type = JSON_TEXT;
			if (!addProperty(parent, key, item)) {
				free(item);
				return false;
			}
		}
		item->u.value = strdup(value.c_str());
		return true;
	}

	bool WriteWstring(const std::string& key, const std::wstring& value) override
	{
		return WriteString(key, ws2s(value));
	}

	bool WriteBool(const std::string& key, bool value) override
	{
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* item = const_cast<json_t*>(json_getProperty(parent, key.c_str()));
		if (!item) {
			item = (json_t*)malloc(sizeof(json_t));
			item->type = JSON_BOOLEAN;
			if (!addProperty(parent, key, item)) {
				free(item);
				return false;
			}
		}
		item->u.value = value ? "true" : "false";
		return true;
	}

	bool WriteLong(const std::string& key, long value) override
	{
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* item = const_cast<json_t*>(json_getProperty(parent, key.c_str()));
		if (!item) {
			item = (json_t*)malloc(sizeof(json_t));
			item->type = JSON_INTEGER;
			if (!addProperty(parent, key, item)) {
				free(item);
				return false;
			}
		}
		item->u.value = strdup(std::to_string(value).c_str());
		return true;
	}

	bool WriteULong(const std::string& key, unsigned long value) override
	{
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* item = const_cast<json_t*>(json_getProperty(parent, key.c_str()));
		if (!item) {
			item = (json_t*)malloc(sizeof(json_t));
			item->type = JSON_INTEGER;
			if (!addProperty(parent, key, item)) {
				free(item);
				return false;
			}
		}
		item->u.value = strdup(std::to_string(value).c_str());
		return true;
	}

	bool WriteInt(const std::string& key, int value) override
	{
		return WriteLong(key, static_cast<long>(value));
	}

	bool WriteUInt(const std::string& key, unsigned int value) override
	{
		return WriteULong(key, static_cast<unsigned long>(value));
	}

	bool WriteSizeT(const std::string& key, size_t value) override
	{
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* item = const_cast<json_t*>(json_getProperty(parent, key.c_str()));
		if (!item) {
			item = (json_t*)malloc(sizeof(json_t));
			item->type = JSON_INTEGER;
			if (!addProperty(parent, key, item)) {
				free(item);
				return false;
			}
		}
		item->u.value = strdup(std::to_string(value).c_str());
		return true;
	}

	bool WriteShort(const std::string& key, short value) override
	{
		return WriteLong(key, static_cast<long>(value));
	}

	bool WriteUShort(const std::string& key, unsigned short value) override
	{
		return WriteULong(key, static_cast<unsigned long>(value));
	}

	bool WriteDouble(const std::string& key, double value) override
	{
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* item = const_cast<json_t*>(json_getProperty(parent, key.c_str()));
		if (!item) {
			item = (json_t*)malloc(sizeof(json_t));
			item->type = JSON_REAL;
			if (!addProperty(parent, key, item)) {
				free(item);
				return false;
			}
		}
		item->u.value = strdup(std::to_string(value).c_str());
		return true;
	}

	bool WriteFloat(const std::string& key, float value) override
	{
		if (!cur_obj_) {
			return false;
		}
		json_t* parent = const_cast<json_t*>(cur_obj_);
		json_t* item = const_cast<json_t*>(json_getProperty(parent, key.c_str()));
		if (!item) {
			item = (json_t*)malloc(sizeof(json_t));
			item->type = JSON_REAL;
			if (!addProperty(parent, key, item)) {
				free(item);
				return false;
			}
		}
		item->u.value = strdup(std::to_string(value).c_str());
		return true;
	}

	bool WriteColor(const std::string& key, const fluo::Color& value) override
	{
		std::string str = value.to_string();
		return WriteString(key, str);
	}

	bool WriteHSVColor(const std::string& key, const fluo::HSVColor& value) override
	{
		std::string str = value.to_string();
		return WriteString(key, str);
	}

	bool WritePoint(const std::string& key, const fluo::Point& value) override
	{
		std::string str = value.to_string();
		return WriteString(key, str);
	}

	bool WriteVector(const std::string& key, const fluo::Vector& value) override
	{
		std::string str = value.to_string();
		return WriteString(key, str);
	}

	bool WriteQuaternion(const std::string& key, const fluo::Quaternion& value) override
	{
		std::string str = value.to_string();
		return WriteString(key, str);
	}

private:
	json_t* json_;
	json_t* cur_obj_;
	std::vector<json_t*> element_stack_;

private:
	std::string json_stringify(const json_t* json) {
		if (json == nullptr) {
			return "{}";
		}

		std::ostringstream oss;
		switch (json_getType(json)) {
		case JSON_OBJ:
			oss << "{";
			for (const json_t* child = json_getChild(json); child != nullptr; child = json_getSibling(child)) {
				oss << "\"" << json_getName(child) << "\": " << json_stringify(child);
				if (json_getSibling(child) != nullptr) {
					oss << ", ";
				}
			}
			oss << "}";
			break;
		case JSON_ARRAY:
			oss << "[";
			for (const json_t* child = json_getChild(json); child != nullptr; child = json_getSibling(child)) {
				oss << json_stringify(child);
				if (json_getSibling(child) != nullptr) {
					oss << ", ";
				}
			}
			oss << "]";
			break;
		case JSON_TEXT:
			oss << "\"" << json_getValue(json) << "\"";
			break;
		case JSON_BOOLEAN:
			oss << (json_getBoolean(json) ? "true" : "false");
			break;
		case JSON_INTEGER:
			oss << json_getInteger(json);
			break;
		case JSON_REAL:
			oss << json_getReal(json);
			break;
		case JSON_NULL:
			oss << "null";
			break;
		default:
			break;
		}

		return oss.str();
	}

	bool addProperty(json_t* parent, const std::string& key, json_t* item)
	{
		if (!parent || !item) {
			return false;
		}
		item->name = strdup(key.c_str());
		item->sibling = nullptr;

		if (!parent->u.c.child) {
			parent->u.c.child = item;
			parent->u.c.last_child = item;
		}
		else {
			parent->u.c.last_child->sibling = item;
			parent->u.c.last_child = item;
		}
		return true;
	}
};

#endif//_JSONFILE_H_