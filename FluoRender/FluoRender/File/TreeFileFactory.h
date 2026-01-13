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
#ifdef _WIN32
		std::ifstream file(filename);
#else
		std::ifstream file(ws2s(filename));
#endif
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
	// Helper function to get the first non-empty, non-comment line
	inline std::string getFirstNonEmptyLine(const std::string& content) {
		std::istringstream stream(content);
		std::string line;
		while (std::getline(stream, line)) {
			// Trim whitespace
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			line.erase(line.find_last_not_of(" \t\r\n") + 1);
			if (!line.empty() && line[0] != ';' && line[0] != '#') {
				return line;
			}
		}
		return "";
	}

	inline void trim(std::string& x)
	{
		auto beg = x.find_first_not_of(" \t\r\n");
		if (beg == std::string::npos) { x.clear(); return; }
		auto end = x.find_last_not_of(" \t\r\n");
		x = x.substr(beg, end - beg + 1);
	}

	inline std::string removeWhitespace(const std::string& s)
	{
		std::string out;
		out.reserve(s.size());
		for (unsigned char c : s) {
			if (!std::isspace(c)) out.push_back(static_cast<char>(c));
		}
		return out;
	}

	// 0: ini, 1: xml, 2: json, 3: pole, -1: unknown
	int determineFileType(const std::string& content) {
		std::string processed_content = content;

		// Remove UTF-8 BOM if present
		if (processed_content.size() >= 3 &&
			static_cast<unsigned char>(processed_content[0]) == 0xEF &&
			static_cast<unsigned char>(processed_content[1]) == 0xBB &&
			static_cast<unsigned char>(processed_content[2]) == 0xBF) {
			processed_content = processed_content.substr(3);
		}

		// Quick XML declaration check
		std::string firstLine = getFirstNonEmptyLine(processed_content);
		if (!firstLine.empty() && firstLine.rfind("<?xml", 0) == 0) {
			return 1; // XML
		}

		std::istringstream stream(processed_content);
		std::string line;

		bool hasIniSection = false;
		bool hasIniKeyValue = false;
		bool sawAngleBracketTagLike = false; // potential XML
		bool sawJsonIndicators = false;      // keys/colons/braces/brackets consistent with JSON
		bool sawJsonBracesOrBrackets = false;

		// Stricter INI section regex: only identifiers as section names
		// e.g., [Section], [section.name], [section-1] but NOT [1,2,3] or [ {"a":1} ]
		std::regex iniSectionStrict(R"(^\[\s*[A-Za-z0-9_.\-]+\s*\]$)");
		// INI key-value: key = value (key cannot start with ; or #)
		std::regex iniKeyValue(R"(^\s*([^#;\s][^=]*?)\s*=\s*(.*?)\s*$)");

		// Light JSON indicators:
		// - presence of colon ':' in a context that looks like key:value
		// - braces/brackets balance & top-level start
		// - lines that look like "key": value or 'key': value
		std::regex jsonKVQuoted(R"(^\s*["'][^"']+["']\s*:\s*.+$)");
		std::regex jsonKVUnquoted(R"(^\s*[A-Za-z0-9_.\-]+\s*:\s*.+$)");

		// First/last non-space characters of entire content (for top-level structure)
		std::string noWS = removeWhitespace(processed_content);
		char firstNonWS = noWS.empty() ? '\0' : noWS.front();
		char lastNonWS = noWS.empty() ? '\0' : noWS.back();
		if (firstNonWS == '{' || firstNonWS == '[') sawJsonBracesOrBrackets = true;

		// Iterate lines to collect hints
		stream.clear();
		stream.seekg(0);
		while (std::getline(stream, line)) {
			trim(line);
			if (line.empty()) continue;
			// Skip INI-style comments
			if (line[0] == ';' || line[0] == '#') continue;

			// POLE detection
			if (std::regex_search(line, std::regex(R"(POLE\s+Structure\s+Start)"))) {
				return 3; // POLE
			}

			// Potential XML tag?
			if (line.front() == '<' && line.back() == '>') {
				// Heuristic: looks like an XML element
				sawAngleBracketTagLike = true;
			}

			// Collect JSON hints
			if (std::regex_match(line, jsonKVQuoted) || std::regex_match(line, jsonKVUnquoted)) {
				sawJsonIndicators = true;
			}
			// Also count braces/brackets presence in lines
			if (line.find('{') != std::string::npos || line.find('[') != std::string::npos) {
				sawJsonBracesOrBrackets = true;
			}

			// INI section header (strict)
			if (std::regex_match(line, iniSectionStrict)) {
				hasIniSection = true;
				continue;
			}

			// INI key-value
			if (std::regex_match(line, iniKeyValue)) {
				hasIniKeyValue = true;
				continue;
			}
		}

		// If content clearly forms JSON at top-level ({...} or [...]) and ends appropriately
		// and we saw JSON indicators, prefer JSON over INI.
		bool topLevelJsonShape =
			((firstNonWS == '{' && lastNonWS == '}') ||
				(firstNonWS == '[' && lastNonWS == ']'));

		if (topLevelJsonShape) {
			// If it looks like JSON, return 2 unless it clearly looks like XML
			// This avoids misclassifying bracket-only JSON arrays as INI sections.
			return 2; // JSON
		}

		// If we saw multiple JSON indicators (key:value patterns) and no INI key-values, prefer JSON
		if (sawJsonIndicators && !hasIniKeyValue && !hasIniSection) {
			return 2; // JSON
		}

		// Final INI check: require at least one key-value pair (not just a section)
		// This prevents array-like [1,2,3] from being mistaken as INI.
		if (hasIniKeyValue || (hasIniSection && sawAngleBracketTagLike == false && sawJsonIndicators == false)) {
			return 0; // INI
		}

		// XML fallback if angle-bracket tags were seen and not INI
		if (sawAngleBracketTagLike) {
			return 1; // XML
		}

		// Unknown
		return -1;
	}
};

#endif//_TREEFILEFACTORY_H_
