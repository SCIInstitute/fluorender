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
#include <compatibility.h>
#include <tiny-json.h>
#include <fstream>
#include <sstream>

class JsonFile : public BaseTreeFile
{
public:
	JsonFile() :
		json_(0),
		cur_obj_(0),
		cur_obj_index_(0),
		mem_size_(32),
		mem_used_(0)
	{
		path_sep_ = "/";
		cd_sep_ = ".";
		pd_sep_ = "..";

		//root
		mem_ = new json_t[mem_size_]();
		char empty_json[] = R"({})";
		json_ = const_cast<json_t*>(json_create(empty_json, mem_, static_cast<unsigned int>(mem_size_)));
		mem_used_ = 1; // Update mem_used_ to account for the root node
		cur_obj_ = json_;
		cur_obj_index_ = 0; // Root node index
	}

	~JsonFile()
	{
		FreeJson(json_);
		if (mem_)
		{
			delete[] mem_;
			mem_ = nullptr;
		}
	}

	int LoadFile(const std::wstring& filename) override
	{
#ifdef _WIN32
        std::wstring long_name = L"\x5c\x5c\x3f\x5c" + filename;
        std::ifstream file(long_name);
#else
        std::wstring long_name = filename;
        std::ifstream file(ws2s(long_name));
#endif
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string str = buffer.str();

		int result = LoadStringConf(str);

		file.close();

		return result;
	}

	int LoadStringConf(const std::string& ini_string) override
	{
		static std::string processed_string = ini_string;

		// Remove BOM if present
		if (processed_string.size() >= 3 &&
			processed_string[0] == '\xEF' &&
			processed_string[1] == '\xBB' &&
			processed_string[2] == '\xBF') {
			processed_string = processed_string.substr(3);
		}

		if (mem_)
		{
			delete[] mem_;
			mem_ = nullptr;
		}
		int node_count = count_nodes(processed_string.c_str());
		mem_size_ = node_count; // Start with an initial size
		mem_used_ = node_count;
		bool success = false;
		char* non_const_ini_string = processed_string.data();

		mem_ = new json_t[mem_size_]();
		json_ = const_cast<json_t*>(json_create(non_const_ini_string, mem_, static_cast<unsigned int>(mem_size_)));

		if (json_ != nullptr) {
			success = true;
			cur_obj_ = json_;
			cur_obj_index_ = 0; // Root node index
			//mem_used_ = 1; // Update mem_used_ to account for the root node
		}

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
        std::ofstream file(long_name);
#else
        std::wstring long_name = filename;
        std::ofstream file(ws2s(long_name));
#endif
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
					if (mem_used_ >= mem_size_) {
						size_t eid = element - mem_;
						if (!ReallocateMem()) {
							return false; // Failed to allocate memory
						}
						element = &mem_[eid];
					}
					child = &mem_[mem_used_++];
					AssignName(child, component.c_str());
					child->type = JSON_OBJ;
					child->u.c.child = nullptr;
					child->u.c.last_child = nullptr;
					child->sibling = nullptr;
					child->u.c.child_index = SIZE_MAX;
					child->u.c.last_child_index = SIZE_MAX;
					child->sibling_index = SIZE_MAX;
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

		// Update cur_obj_index_
		cur_obj_index_ = element - mem_;

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
	bool DeleteEntry(const std::string& key) override {
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
				// Free the memory associated with the item if dynamically allocated
				if (item->name_dynamically_allocated) {
					free((void*)item->name);
				}
				if (item->type == JSON_TEXT && item->value_dynamically_allocated) {
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

	std::string EncodeXml(const std::string& xml) override
	{
		std::string encoded;
		for (char c : xml) {
			switch (c) {
			case '\\': encoded += "\\\\"; break;
			case '\"': encoded += "\\\""; break;
			case '\n': encoded += "\\n"; break;
			case '\r': encoded += "\\r"; break;
			case '\t': encoded += "\\t"; break;
			default:   encoded += c; break;
			}
		}
		return encoded;
	}

	std::string DecodeXml(const std::string& encoded) override
	{
		std::string decoded;
		for (size_t i = 0; i < encoded.length(); ++i) {
			if (encoded[i] == '\\' && i + 1 < encoded.length()) {
				char next = encoded[i + 1];
				switch (next) {
				case 'n': decoded += '\n'; ++i; break;
				case 'r': decoded += '\r'; ++i; break;
				case 't': decoded += '\t'; ++i; break;
				case '\\': decoded += '\\'; ++i; break;
				case '\"': decoded += '\"'; ++i; break;
				default: decoded += encoded[i]; break;
				}
			}
			else {
				decoded += encoded[i];
			}
		}
		return decoded;
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
			if (mem_used_ >= mem_size_) {
				size_t pid = parent - mem_;
				if (!ReallocateMem()) {
					return false;
				}
				parent = &mem_[pid];
			}
			item = &mem_[mem_used_++];
			item->type = JSON_TEXT;
			if (!addProperty(parent, key, item)) {
				mem_used_--;
				return false;
			}
		}
		AssignValue(item, value.c_str());
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
			if (mem_used_ >= mem_size_) {
				size_t pid = parent - mem_;
				if (!ReallocateMem()) {
					return false;
				}
				parent = &mem_[pid];
			}
			item = &mem_[mem_used_++];
			item->type = JSON_BOOLEAN;
			if (!addProperty(parent, key, item)) {
				mem_used_--;
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
			if (mem_used_ >= mem_size_) {
				size_t pid = parent - mem_;
				if (!ReallocateMem()) {
					return false;
				}
				parent = &mem_[pid];
			}
			item = &mem_[mem_used_++];
			item->type = JSON_INTEGER;
			if (!addProperty(parent, key, item)) {
				mem_used_--;
				return false;
			}
		}
		AssignValue(item, std::to_string(value).c_str());
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
			if (mem_used_ >= mem_size_) {
				size_t pid = parent - mem_;
				if (!ReallocateMem()) {
					return false;
				}
				parent = &mem_[pid];
			}
			item = &mem_[mem_used_++];
			item->type = JSON_INTEGER;
			if (!addProperty(parent, key, item)) {
				mem_used_--;
				return false;
			}
		}
		AssignValue(item, std::to_string(value).c_str());
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
			if (mem_used_ >= mem_size_) {
				size_t pid = parent - mem_;
				if (!ReallocateMem()) {
					return false;
				}
				parent = &mem_[pid];
			}
			item = &mem_[mem_used_++];
			item->type = JSON_INTEGER;
			if (!addProperty(parent, key, item)) {
				mem_used_--;
				return false;
			}
		}
		AssignValue(item, std::to_string(value).c_str());
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
			if (mem_used_ >= mem_size_) {
				size_t pid = parent - mem_;
				if (!ReallocateMem()) {
					return false;
				}
				parent = &mem_[pid];
			}
			item = &mem_[mem_used_++];
			item->type = JSON_REAL;
			if (!addProperty(parent, key, item)) {
				mem_used_--;
				return false;
			}
		}
		AssignValue(item, std::to_string(value).c_str());
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
			if (mem_used_ >= mem_size_) {
				size_t pid = parent - mem_;
				if (!ReallocateMem()) {
					return false;
				}
				parent = &mem_[pid];
			}
			item = &mem_[mem_used_++];
			item->type = JSON_REAL;
			if (!addProperty(parent, key, item)) {
				mem_used_--;
				return false;
			}
		}
		AssignValue(item, std::to_string(value).c_str());
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
	json_t* mem_;
	size_t mem_size_;
	size_t mem_used_;
	json_t* cur_obj_;
	size_t cur_obj_index_;
	std::vector<json_t*> element_stack_;

private:
	bool ReallocateMem() {
		size_t new_size = mem_size_ * 2;
		json_t* new_mem = new json_t[new_size]();
		if (!new_mem) {
			return false;
		}
		std::copy(mem_, mem_ + mem_size_, new_mem);
		delete[] mem_;
		mem_ = new_mem;
		mem_size_ = new_size;

		// Update json_ and cur_obj_ to point to the new memory block
		json_ = &mem_[0];
		cur_obj_ = &mem_[cur_obj_index_]; // Update cur_obj_ to the correct index

		// Update all indices in the new memory block
		UpdateIndices();

		return true;
	}

	void UpdateIndices() {
		for (size_t i = 0; i < mem_used_; ++i) {
			json_t* item = &mem_[i];
			if (item->type == JSON_OBJ) {
				if (item->u.c.child_index != SIZE_MAX) {
					item->u.c.child = &mem_[item->u.c.child_index];
				}
				if (item->u.c.last_child_index != SIZE_MAX) {
					item->u.c.last_child = &mem_[item->u.c.last_child_index];
				}
			}
			if (item->sibling_index != SIZE_MAX) {
				item->sibling = &mem_[item->sibling_index];
			}
		}
	}

	bool addProperty(json_t* parent, const std::string& key, json_t* item) {
		if (!parent || !item) {
			return false;
		}

		// Ensure parent is of type JSON_OBJ
		if (parent->type != JSON_OBJ) {
			return false;
		}

		// Allocate memory for the name and check for success
		AssignName(item, key.c_str());
		if (!item->name) {
			return false;
		}

		item->sibling = nullptr;
		item->sibling_index = SIZE_MAX;

		if (!parent->u.c.child) {
			parent->u.c.child = item;
			parent->u.c.child_index = item - mem_;
			parent->u.c.last_child = item;
			parent->u.c.last_child_index = item - mem_;
		}
		else {
			parent->u.c.last_child->sibling = item;
			parent->u.c.last_child->sibling_index = item - mem_;
			parent->u.c.last_child = item;
			parent->u.c.last_child_index = item - mem_;
		}

		return true;
	}

	std::string json_stringify(const json_t* json, bool formatted = true, int indent = 0) {
		if (json == nullptr) {
			return "{}";
		}

		std::ostringstream oss;
		std::string indent_str = formatted ? std::string(indent, '\t') : "";

		switch (json_getType(json)) {
		case JSON_OBJ:
			oss << "{";
			if (formatted) oss << "\n";
			for (const json_t* child = json_getChild(json); child != nullptr; child = json_getSibling(child)) {
				if (formatted) oss << indent_str << "\t";
				oss << "\"" << json_getName(child) << "\": " << json_stringify(child, formatted, indent + 1);
				if (json_getSibling(child) != nullptr) {
					oss << ",";
					if (formatted) oss << "\n";
				}
				else if (formatted) {
					oss << "\n";
				}
			}
			if (formatted) oss << indent_str;
			oss << "}";
			break;
		case JSON_ARRAY:
			oss << "[";
			if (formatted) oss << "\n";
			for (const json_t* child = json_getChild(json); child != nullptr; child = json_getSibling(child)) {
				if (formatted) oss << indent_str << "\t";
				oss << json_stringify(child, formatted, indent + 1);
				if (json_getSibling(child) != nullptr) {
					oss << ",";
					if (formatted) oss << "\n";
				}
				else if (formatted) {
					oss << "\n";
				}
			}
			if (formatted) oss << indent_str;
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

	int count_nodes(const char* json_data) {
		int count = 0;
		const char* p = json_data;
		bool in_string = false;
		char prev_char = '\0';

		while (*p) {
			if (*p == '\"') {
				in_string = !in_string;
			}
			if (!in_string) {
				if (*p == '{' || *p == '[') {
					if (prev_char != ':') {
						count++; // Count opening braces/brackets only if not preceded by ':'
					}
				}
				else if (*p == ':') {
					count++; // Count key-value pairs
				}
				if (!isspace(*p) && *p != '\n' && *p != '\r' && *p != '\t') {
					prev_char = *p; // Update previous valid character
				}
			}
			p++;
		}
		return count;
	}

	void AssignName(json_t* item, const char* name) {
		if (item->name_dynamically_allocated) {
			free((void*)item->name);
		}
		item->name = strdup(name);
		item->name_dynamically_allocated = true;
	}

	void AssignValue(json_t* item, const char* value) {
		if (item->value_dynamically_allocated) {
			free((void*)item->u.value);
		}
		item->u.value = strdup(value);
		item->value_dynamically_allocated = true;
	}

	void FreeJson(json_t* obj) {
		if (!obj) return;

		// Free the name if dynamically allocated
		if (obj->name_dynamically_allocated) {
			free((void*)obj->name);
		}

		// Free the value if dynamically allocated and the type is JSON_TEXT
		if (obj->type == JSON_TEXT && obj->value_dynamically_allocated) {
			free((void*)obj->u.value);
		}

		// Recursively free children if the node is a JSON object
		if (obj->type == JSON_OBJ) {
			for (json_t* item = obj->u.c.child; item; item = item->sibling) {
				FreeJson(item);
			}
		}
	}
};

#endif//_JSONFILE_H_
