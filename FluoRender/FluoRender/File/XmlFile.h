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
#ifndef _XMLFILE_H_
#define _XMLFILE_H_

#include <BaseTreeFile.h>
#include <tinyxml2.h>
#include <compatibility.h>

class XmlFile : public BaseTreeFile
{
public:
	XmlFile() :
		cur_element_(0)
	{
	}

	~XmlFile() {}

	int LoadFile(const std::wstring& filename) override
	{
		std::string str = ws2s(filename);
		doc_.LoadFile(str.c_str());
		cur_element_ = doc_.RootElement();
		cur_path_ = path_sep_;
		return 0;
	}

	int LoadString(const std::string& ini_string) override
	{
		doc_.Parse(ini_string.c_str());
		cur_element_ = doc_.RootElement();
		cur_path_ = path_sep_;
		return 0;
	}

	int SaveFile(const std::wstring& filename) override
	{
		std::string str = ws2s(filename);
		if (doc_.SaveFile(str.c_str()) == tinyxml2::XML_SUCCESS)
		{
			return 0;
		}
		return 1; // Return an error code if saving fails
	}

	int SaveString(std::string& str) override
	{
		tinyxml2::XMLPrinter printer;
		doc_.Print(&printer);
		str = printer.CStr();
		return 0;
	}

	// Implement group management methods
	bool Exists(const std::string& path) const override
	{
		// Normalize the path to handle relative and absolute paths
		std::string normalized_path = getFullPath(path);
		std::vector<std::string> components = splitPath(normalized_path);

		// Start from the root element or current element based on whether the path is absolute or relative
		const tinyxml2::XMLElement* element = (normalized_path.substr(0, path_sep_.length()) == path_sep_) ? doc_.RootElement() : cur_element_;

		// Traverse the path components
		for (const auto& component : components) {
			if (component == cd_sep_) {
				// Current level, do nothing
				continue;
			}
			else if (component == pd_sep_) {
				// Parent level, move to parent element if possible
				if (element->Parent() && element->Parent()->ToElement()) {
					element = element->Parent()->ToElement();
				}
				else {
					return false; // No parent element, path is invalid
				}
			}
			else {
				// Move to the child element with the given name
				element = element->FirstChildElement(component.c_str());
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
		tinyxml2::XMLElement* element = nullptr;
		std::string new_path;

		if (normalized_path.substr(0, path_sep_.length()) == path_sep_) {
			// Absolute path
			element = doc_.RootElement();
			new_path = path_sep_;
		}
		else {
			// Relative path
			element = cur_element_;
			new_path = cur_path_;
		}

		// Traverse the path components
		for (const auto& component : components) {
			if (component == cd_sep_) {
				// Current level, do nothing
				continue;
			}
			else if (component == pd_sep_) {
				// Parent level, move to parent element if possible
				if (element->Parent() && element->Parent()->ToElement()) {
					element = element->Parent()->ToElement();
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
				tinyxml2::XMLElement* child = element->FirstChildElement(component.c_str());
				if (!child) {
					// Create a new child element if it doesn't exist
					child = doc_.NewElement(component.c_str());
					element->InsertEndChild(child);
				}
				element = child;
				new_path += path_sep_ + component;
			}
		}

		// Update cur_element_ and cur_path_
		cur_element_ = element;
		cur_path_ = new_path;

		return true; // Path exists or was created, and cur_element_ and cur_path_ are updated
	}

	std::string GetPath() const override
	{
		return cur_path_;
	}

	bool HasGroup(const std::string& group) const override
	{
		if (!cur_element_)
			return false;
		return cur_element_->FirstChildElement(group.c_str()) != nullptr;
	}

	bool HasEntry(const std::string& entry) const override
	{
		if (!cur_element_)
			return false;
		return cur_element_->FirstChildElement(entry.c_str()) != nullptr;
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override
	{
		if (!cur_element_)
			return false;
		const tinyxml2::XMLElement* element = cur_element_->FirstChildElement();
		if (element) {
			*group = element->Value();
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextGroup(std::string* group, long* index) const override
	{
		if (!cur_element_)
			return false;
		const tinyxml2::XMLElement* element = cur_element_->NextSiblingElement();
		if (element) {
			*group = element->Value();
			(*index)++;
			return true;
		}
		return false;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override
	{
		if (!cur_element_)
			return false;
		const tinyxml2::XMLElement* element = cur_element_->FirstChildElement();
		if (element) {
			*entry = element->Value();
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextEntry(std::string* entry, long* index) const override
	{
		if (!cur_element_)
			return false;
		const tinyxml2::XMLElement* element = cur_element_->NextSiblingElement();
		if (element) {
			*entry = element->Value();
			(*index)++;
			return true;
		}
		return false;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override
	{
		if (!cur_element_)
			return false;
		tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (element) {
			cur_element_->DeleteChild(element);
			return true;
		}
		return false;
	}

	bool DeleteGroup(const std::string& group) override
	{
		if (!cur_element_)
			return false;
		tinyxml2::XMLElement* element = cur_element_->FirstChildElement(group.c_str());
		if (element) {
			cur_element_->DeleteChild(element);
			return true;
		}
		return false;
	}

protected:
	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value, const std::string& def = "") const override
	{
		if (!cur_element_)
		{
			*value = def;
			return false;
		}
		const tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (element && element->GetText()) {
			*value = element->GetText();
			return true;
		}
		*value = def;
		return false;
	}

	bool ReadWstring(const std::string& key, std::wstring* value, const std::wstring& def = L"") const override
	{
		if (!cur_element_) {
			*value = def;
			return false;
		}
		const tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (element) {
			const char* text = element->GetText();
			if (text) {
				*value = std::wstring(text, text + strlen(text));
				return true;
			}
		}
		*value = def;
		return false;
	}

	bool ReadBool(const std::string& key, bool* value, bool def = false) const override
	{
		if (!cur_element_) {
			*value = def;
			return false;
		}
		const tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (element) {
			const char* text = element->GetText();
			if (text) {
				std::string textStr(text);
				std::transform(textStr.begin(), textStr.end(), textStr.begin(), ::tolower);
				if (textStr == "true" || textStr == "1") {
					*value = true;
					return true;
				}
				else if (textStr == "false" || textStr == "0") {
					*value = false;
					return true;
				}
			}
		}
		*value = def;
		return false;
	}

	bool ReadLong(const std::string& key, long* value, long def = 0) const override
	{
		if (!cur_element_)
		{
			*value = def;
			return false;
		}
		const tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (element) {
			int64_t tempValue;
			if (element->QueryInt64Text(&tempValue) == tinyxml2::XML_SUCCESS) {
				*value = static_cast<long>(tempValue);
				return true;
			}
		}
		*value = def;
		return false;
	}

	bool ReadDouble(const std::string& key, double* value, double def = 0.0) const override
	{
		if (!cur_element_)
		{
			*value = def;
			return false;
		}
		const tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (element) {
			element->QueryDoubleText(value);
			return true;
		}
		*value = def;
		return false;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override
	{
		if (!cur_element_)
			return false;
		tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (!element) {
			element = doc_.NewElement(key.c_str());
			cur_element_->InsertEndChild(element);
		}
		element->SetText(value.c_str());
		return true;
	}

	bool WriteWstring(const std::string& key, const std::wstring& value) override
	{
		if (!cur_element_) {
			return false;
		}
		tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (!element) {
			element = cur_element_->GetDocument()->NewElement(key.c_str());
			cur_element_->InsertEndChild(element);
		}
		std::string valueStr(value.begin(), value.end());
		element->SetText(valueStr.c_str());
		return true;
	}

	bool WriteBool(const std::string& key, bool value) override
	{
		if (!cur_element_) {
			return false;
		}
		tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (!element) {
			element = cur_element_->GetDocument()->NewElement(key.c_str());
			cur_element_->InsertEndChild(element);
		}
		element->SetText(value ? "1" : "0");
		return true;
	}

	bool WriteLong(const std::string& key, long value) override
	{
		if (!cur_element_)
			return false;
		tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (!element) {
			element = doc_.NewElement(key.c_str());
			cur_element_->InsertEndChild(element);
		}
		element->SetText(static_cast<int>(value));
		return true;
	}

	bool WriteDouble(const std::string& key, double value) override
	{
		if (!cur_element_)
			return false;
		tinyxml2::XMLElement* element = cur_element_->FirstChildElement(key.c_str());
		if (!element) {
			element = doc_.NewElement(key.c_str());
			cur_element_->InsertEndChild(element);
		}
		element->SetText(value);
		return true;
	}

private:
	tinyxml2::XMLDocument doc_;
	tinyxml2::XMLElement* cur_element_;
	std::string cur_path_;
};

#endif//_XMLFILE_H_