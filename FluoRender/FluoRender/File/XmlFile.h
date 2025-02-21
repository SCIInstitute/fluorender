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

class XmlFile : public BaseTreeFile
{
public:
	XmlFile(const std::string& filename) {
		doc.LoadFile(filename.c_str());
		currentElement = doc.RootElement();
	}

	~XmlFile() {}

	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value) const override {
		const tinyxml2::XMLElement* element = currentElement->FirstChildElement(key.c_str());
		if (element && element->GetText()) {
			*value = element->GetText();
			return true;
		}
		return false;
	}

	bool ReadLong(const std::string& key, long* value) const override {
		const tinyxml2::XMLElement* element = currentElement->FirstChildElement(key.c_str());
		if (element) {
			int64_t tempValue;
			if (element->QueryInt64Text(&tempValue) == tinyxml2::XML_SUCCESS) {
				*value = static_cast<long>(tempValue);
				return true;
			}
		}
		return false;
	}

	bool ReadDouble(const std::string& key, double* value) const override {
		const tinyxml2::XMLElement* element = currentElement->FirstChildElement(key.c_str());
		if (element) {
			element->QueryDoubleText(value);
			return true;
		}
		return false;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override {
		tinyxml2::XMLElement* element = currentElement->FirstChildElement(key.c_str());
		if (!element) {
			element = doc.NewElement(key.c_str());
			currentElement->InsertEndChild(element);
		}
		element->SetText(value.c_str());
		return true;
	}

	bool WriteLong(const std::string& key, long value) override {
		tinyxml2::XMLElement* element = currentElement->FirstChildElement(key.c_str());
		if (!element) {
			element = doc.NewElement(key.c_str());
			currentElement->InsertEndChild(element);
		}
		element->SetText(static_cast<int>(value));
		return true;
	}

	bool WriteDouble(const std::string& key, double value) override {
		tinyxml2::XMLElement* element = currentElement->FirstChildElement(key.c_str());
		if (!element) {
			element = doc.NewElement(key.c_str());
			currentElement->InsertEndChild(element);
		}
		element->SetText(value);
		return true;
	}

	// Implement group management methods
	bool SetPath(const std::string& path) override {
		currentElement = doc.RootElement()->FirstChildElement(path.c_str());
		return currentElement != nullptr;
	}

	std::string GetPath() const override {
		return currentElement->Value();
	}

	bool HasGroup(const std::string& group) const override {
		return currentElement->FirstChildElement(group.c_str()) != nullptr;
	}

	bool HasEntry(const std::string& entry) const override {
		return currentElement->FirstChildElement(entry.c_str()) != nullptr;
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override {
		const tinyxml2::XMLElement* element = currentElement->FirstChildElement();
		if (element) {
			*group = element->Value();
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextGroup(std::string* group, long* index) const override {
		const tinyxml2::XMLElement* element = currentElement->NextSiblingElement();
		if (element) {
			*group = element->Value();
			(*index)++;
			return true;
		}
		return false;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override {
		const tinyxml2::XMLElement* element = currentElement->FirstChildElement();
		if (element) {
			*entry = element->Value();
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextEntry(std::string* entry, long* index) const override {
		const tinyxml2::XMLElement* element = currentElement->NextSiblingElement();
		if (element) {
			*entry = element->Value();
			(*index)++;
			return true;
		}
		return false;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override {
		tinyxml2::XMLElement* element = currentElement->FirstChildElement(key.c_str());
		if (element) {
			currentElement->DeleteChild(element);
			return true;
		}
		return false;
	}

	bool DeleteGroup(const std::string& group) override {
		tinyxml2::XMLElement* element = currentElement->FirstChildElement(group.c_str());
		if (element) {
			currentElement->DeleteChild(element);
			return true;
		}
		return false;
	}

private:
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement* currentElement;
};

#endif//_XMLFILE_H_