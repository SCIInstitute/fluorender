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
#ifndef _POLEFILE_H_
#define _POLEFILE_H_

#include <BaseTreeFile.h>
#include <pole.h>
#include <sstream>

class PoleFile : public BaseTreeFile
{
public:
	PoleFile(const std::string& filename) : storage(filename) {
		currentNode = storage.root();
	}

	~PoleFile() {}

	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value) const override {
		auto node = currentNode->find(key);
		if (node != currentNode->end()) {
			std::stringstream ss;
			node->second->read(ss);
			*value = ss.str();
			return true;
		}
		return false;
	}

	bool ReadLong(const std::string& key, long* value) const override {
		auto node = currentNode->find(key);
		if (node != currentNode->end()) {
			std::stringstream ss;
			node->second->read(ss);
			ss >> *value;
			return true;
		}
		return false;
	}

	bool ReadDouble(const std::string& key, double* value) const override {
		auto node = currentNode->find(key);
		if (node != currentNode->end()) {
			std::stringstream ss;
			node->second->read(ss);
			ss >> *value;
			return true;
		}
		return false;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override {
		auto node = currentNode->find(key);
		if (node == currentNode->end()) {
			node = currentNode->insert(key);
		}
		std::stringstream ss(value);
		node->second->write(ss);
		return true;
	}

	bool WriteLong(const std::string& key, long value) override {
		auto node = currentNode->find(key);
		if (node == currentNode->end()) {
			node = currentNode->insert(key);
		}
		std::stringstream ss;
		ss << value;
		node->second->write(ss);
		return true;
	}

	bool WriteDouble(const std::string& key, double value) override {
		auto node = currentNode->find(key);
		if (node == currentNode->end()) {
			node = currentNode->insert(key);
		}
		std::stringstream ss;
		ss << value;
		node->second->write(ss);
		return true;
	}

	// Implement group management methods
	bool SetPath(const std::string& path) override {
		currentNode = storage.find(path);
		return currentNode != nullptr;
	}

	std::string GetPath() const override {
		return currentNode->name();
	}

	bool HasGroup(const std::string& group) const override {
		return currentNode->find(group) != currentNode->end();
	}

	bool HasEntry(const std::string& entry) const override {
		return currentNode->find(entry) != currentNode->end();
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override {
		if (!currentNode->empty()) {
			*group = currentNode->begin()->first;
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextGroup(std::string* group, long* index) const override {
		auto it = currentNode->begin();
		std::advance(it, *index + 1);
		if (it != currentNode->end()) {
			*group = it->first;
			(*index)++;
			return true;
		}
		return false;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override {
		if (!currentNode->empty()) {
			*entry = currentNode->begin()->first;
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextEntry(std::string* entry, long* index) const override {
		auto it = currentNode->begin();
		std::advance(it, *index + 1);
		if (it != currentNode->end()) {
			*entry = it->first;
			(*index)++;
			return true;
		}
		return false;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override {
		auto node = currentNode->find(key);
		if (node != currentNode->end()) {
			currentNode->erase(node);
			return true;
		}
		return false;
	}

	bool DeleteGroup(const std::string& group) override {
		auto node = currentNode->find(group);
		if (node != currentNode->end()) {
			currentNode->erase(node);
			return true;
		}
		return false;
	}

private:
	POLE::Storage storage;
	POLE::Node* currentNode;
};

#endif//_POLEFILE_H_