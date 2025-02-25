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
#ifndef _TREEFILEFACTORY_H_
#define _TREEFILEFACTORY_H_

#include <IniFile.h>
#include <JsonFile.h>
#include <PoleFile.h>
#include <XmlFile.h>
#include <PoleFile.h>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <memory>
#include <unordered_map>

class TreeFileFactory
{
public:
	std::shared_ptr<BaseTreeFile> createTreeFile(const std::wstring& filename, const std::string& id) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			throw std::runtime_error("Unable to open file");
		}

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		int type = determineFileType(content);
		std::shared_ptr<BaseTreeFile> handler;
		switch (type)
		{
		case 0:
			handler = std::make_shared<IniFile>();
			break;
		case 1:
			handler = std::make_shared<XmlFile>();
			break;
		case 2:
			handler = std::make_shared<JsonFile>();
			break;
		case 3:
			handler = std::make_shared<PoleFile>();
			break;
		default:
			throw std::runtime_error("Unknown file type");
		}

		handlers[id] = handler;
		return handler;
	}

	std::shared_ptr<BaseTreeFile> getTreeFile(const std::string& id) {
		auto it = handlers.find(id);
		if (it != handlers.end()) {
			return it->second;
		}
		return nullptr;
	}

	void releaseFileHandler(const std::string& id) {
		handlers.erase(id);
	}

private:
	std::unordered_map<std::string, std::shared_ptr<BaseTreeFile>> handlers;

private:
	//0: ini, 1: xml, 2: json, 3: pole
	int determineFileType(const std::string& content)
	{
		std::regex iniRegex(R"(\[.*\]\s*.*=.*)");
		if (std::regex_search(content, iniRegex)) {
			return 0;
		}

		std::regex xmlRegex(R"(<\?xml.*\?>|<.*>.*<\/.*>)");
		if (std::regex_search(content, xmlRegex)) {
			return 1;
		}

		std::regex jsonRegex(R"(\{.*:.*\})");
		if (std::regex_search(content, jsonRegex)) {
			return 2;
		}

		// Add regex for POLE file structure
		std::regex poleRegex(R"(POLE\s+Structure\s+Start)");
		if (std::regex_search(content, poleRegex)) {
			return 3;
		}

		return -1;
	}
};

#endif//_TREEFILEFACTORY_H_