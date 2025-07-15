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
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <memory>
#include <unordered_map>

class TreeFileFactory
{
public:
	std::shared_ptr<BaseTreeFile> createTreeFile(int type, const std::string& id) {
		std::shared_ptr<BaseTreeFile> handler;
		switch (type)
		{
		case 0:
		default:
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
		}

		handlers[id] = handler;
		return handler;
	}

	std::shared_ptr<BaseTreeFile> createTreeFile(const std::wstring& filename, const std::string& id) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			return nullptr;
		}

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		int type = determineFileType(content);
		return createTreeFile(type, id);
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
	std::string getFirstNonEmptyLine(const std::string& content) {
		std::istringstream stream(content);
		std::string line;
		while (std::getline(stream, line)) {
			// Trim whitespace
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			line.erase(line.find_last_not_of(" \t\r\n") + 1);
			if (!line.empty() && line[0] != '#') {
				return line;
			}
		}
		return "";
	}

	//0: ini, 1: xml, 2: json, 3: pole
	int determineFileType(const std::string& content)
	{
		std::string processed_content = content;

		// Remove BOM if present
		if (processed_content.size() >= 3 &&
			processed_content[0] == '\xEF' &&
			processed_content[1] == '\xBB' &&
			processed_content[2] == '\xBF') {
			processed_content = processed_content.substr(3);
		}

		std::istringstream stream(processed_content);
		std::string line;
		bool hasIniSection = false;
		bool hasIniKeyValue = false;

		while (std::getline(stream, line)) {
			// Trim whitespace
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			line.erase(line.find_last_not_of(" \t\r\n") + 1);

			if (line.empty() || line[0] == ';' || line[0] == '#') continue; // Skip comments

			// INI section header
			if (std::regex_match(line, std::regex(R"(\[\s*[^\]\r\n]+\s*\])")))
			{
				hasIniSection = true;
				continue;
			}

			// INI key-value pair
			if (std::regex_match(line, std::regex(R"(^\s*([^#;\s][^=]*[^#;\s])\s*=\s*([^\r\n]*)$)")))
			{
				hasIniKeyValue = true;
				continue;
			}

			// POLE detection
			if (std::regex_search(line, std::regex(R"(POLE\s+Structure\s+Start)"))) {
				return 3;
			}

			// XML detection
			if (line.find("<?xml") == 0 || line.find('<') == 0) {
				return 1;
			}
		}

		// Only check for JSON after scanning all lines
		std::string firstLine = getFirstNonEmptyLine(processed_content);
		if (!hasIniSection && !hasIniKeyValue &&
			!firstLine.empty() && (firstLine[0] == '{' || firstLine[0] == '[')) {
			return 2; // JSON
		}

		if (hasIniSection || hasIniKeyValue) {
			return 0; // INI
		}

		return -1; // Unknown
	}
};

#endif//_TREEFILEFACTORY_H_