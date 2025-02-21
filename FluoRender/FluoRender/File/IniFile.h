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
#ifndef _INIFILE_H_
#define _INIFILE_H_

#include <BaseTreeFile.h>
#include <confini.h>
#include <compatibility.h>
#include <map>
#include <set>

#define PATH_SEPARATOR '\\'
#define CURR_DIR '.'
#define PAR_DIR ".."
#define __INI_MAP_DEFAULT_FORMAT__ { \
    /* .delimiter_symbol = */ INI_EQUALS, \
    /* .case_sensitive = */ false, \
    \
    /*   We ignore comments and disabled entries...  */ \
    /* .semicolon_marker = */ INI_IGNORE, \
    /* .hash_marker = */ INI_IGNORE, \
    \
    /* .section_paths = */ INI_ABSOLUTE_AND_RELATIVE, \
    /* .multiline_nodes = */ INI_MULTILINE_EVERYWHERE, \
    /* .no_single_quotes = */ false, \
    /* .no_double_quotes = */ false, \
    /* .no_spaces_in_names = */ false, \
    /* .implicit_is_not_empty = */ false, \
    /* .do_not_collapse_values = */ false, \
    /* .preserve_empty_quotes = */ false, \
    /* .disabled_after_space = */ false, \
    /* .disabled_can_be_implicit = */ false \
  }

class IniFile : public BaseTreeFile
{
public:
	IniFile() :
		dictionary()
	{
	}

	int LoadFile(const std::string& filename) override
	{
		dictionary.clear();
		_format_ = __INI_MAP_DEFAULT_FORMAT__;
		return load_ini_path(filename.c_str(), __INI_MAP_DEFAULT_FORMAT__, NULL, _push_dispatch_, &dictionary);
	}

	int LoadString(const std::string& ini_string) override
	{
		dictionary.clear();
		_format_ = __INI_MAP_DEFAULT_FORMAT__;
		size_t len = ini_string.length();
		char* tmp = new char[len + 1];
		memcpy(tmp, ini_string.c_str(), len + 1);
		int retval = strip_ini_cache(tmp, len, __INI_MAP_DEFAULT_FORMAT__, NULL, _push_dispatch_, &dictionary);
		delete[] tmp;
		return retval;
	}

	int SaveFile(const std::string& filename) override
	{
		std::ofstream file(filename);
		if (!file.is_open()) {
			return CONFINI_EIO;
		}

		std::map<std::string, std::string> sorted_dict(dictionary.begin(), dictionary.end());

		for (const auto& pair : sorted_dict) {
			size_t pos = pair.first.find(PATH_SEPARATOR);
			std::string section = pair.first.substr(0, pos);
			std::string key = pair.first.substr(pos + 1);

			file << "[" << section << "]\n";
			file << key << " = " << pair.second << "\n";
		}

		file.close();
		return CONFINI_SUCCESS;
	}

	int SaveString(std::string& str) override
	{
		std::ostringstream oss;

		std::map<std::string, std::string> sorted_dict(dictionary.begin(), dictionary.end());

		for (const auto& pair : sorted_dict) {
			size_t pos = pair.first.find(PATH_SEPARATOR);
			std::string section = pair.first.substr(0, pos);
			std::string key = pair.first.substr(pos + 1);

			oss << "[" << section << "]\n";
			oss << key << " = " << pair.second << "\n";
		}

		str = oss.str();
		return CONFINI_SUCCESS;
	}


	// Implement group management methods
	bool Exists(const std::string& path) const override
	{
		std::string full_key = getFullKey(path);
		return dictionary.count(full_key) ? true : false;
	}

	bool SetPath(const std::string& path) override
	{
		current_section = getFullPath(path);
		return !current_section.empty();
	}

	std::string GetPath() const override
	{
		return current_section;
	}

	bool HasGroup(const std::string& group) const override
	{
		std::string prefix = current_section + group + PATH_SEPARATOR;
		for (const auto& pair : dictionary)
		{
			if (pair.first.find(prefix) == 0 && pair.first != group) {
				return true;
			}
		}
		return false;
	}

	bool HasEntry(const std::string& entry) const override
	{
		std::string full_key = getFullKey(entry);
		return dictionary.find(full_key) != dictionary.end();
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override
	{
		if (dictionary.empty()) {
			return false;
		}

		std::set<std::string> unique_sections;
		for (const auto& pair : dictionary) {
			size_t pos = pair.first.find(PATH_SEPARATOR);
			std::string section = (pos == std::string::npos) ? pair.first : pair.first.substr(0, pos);
			unique_sections.insert(section);
		}

		if (unique_sections.empty()) {
			return false;
		}

		*group = *unique_sections.begin();
		*index = 0;
		sections_vector.assign(unique_sections.begin(), unique_sections.end());
		return true;
	}

	bool GetNextGroup(std::string* group, long* index) const override
	{
		if (*index + 1 >= sections_vector.size()) {
			return false;
		}

		*index += 1;
		*group = sections_vector[*index];
		return true;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override
	{
		if (dictionary.empty() || current_section.empty()) {
			return false;
		}

		entries_vector.clear();
		std::string prefix = current_section;
		for (const auto& pair : dictionary) {
			if (pair.first.find(prefix) == 0) {
				entries_vector.push_back(pair.first.substr(prefix.length()));
			}
		}

		if (entries_vector.empty()) {
			return false;
		}

		*entry = entries_vector[0];
		*index = 0;
		return true;
	}

	bool GetNextEntry(std::string* entry, long* index) const override
	{
		if (*index + 1 >= entries_vector.size()) {
			return false;
		}

		*index += 1;
		*entry = entries_vector[*index];
		return true;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override
	{
		std::string full_key = getFullKey(key);
		auto it = dictionary.find(full_key);
		if (it != dictionary.end()) {
			dictionary.erase(it);
			return true;
		}
		return false;
	}

	bool DeleteGroup(const std::string& group) override
	{
		std::string prefix = getFullPath(group);
		bool found = false;
		for (auto it = dictionary.begin(); it != dictionary.end(); ) {
			if (it->first.find(prefix) == 0 || it->first == group) {
				it = dictionary.erase(it);
				found = true;
			}
			else {
				++it;
			}
		}
		return found;
	}

protected:
	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value) const override
	{
		std::string str;
		if (!extractString(key, str))
			return false;
		*value = str;
		return true;
	}

	bool ReadWstring(const std::string& key, std::wstring* value) const override
	{
		std::string str;
		if (!extractString(key, str))
			return false;
		*value = s2ws(str);
		return true;
	}

	bool ReadBool(const std::string& key, bool* value) const override
	{
		unsigned int uintbool =
			static_cast<unsigned int>(ini_get_bool_i(getSource(key).c_str(), 2, _format_));
		*value = !(uintbool & 2);
		return static_cast<bool>(uintbool & 1);
	}

	bool ReadLong(const std::string& key, long* value) const override
	{
		std::string str;
		if (!extractString(key, str))
			return false;
		*value = std::stol(str);
		return true;
	}

	bool ReadDouble(const std::string& key, double* value) const override
	{
		std::string str;
		if (!extractString(key, str))
			return false;
		*value = std::stod(str);
		return true;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override
	{
		std::string full_key = getFullKey(key);
		dictionary.insert(std::pair<std::string, std::string>(full_key, value));
		return true;
	}

	bool WriteWstring(const std::string& key, const std::wstring& value) override
	{
		return WriteString(key, ws2s(value));
	}

	bool WriteBool(const std::string& key, bool value) override
	{
		return WriteString(key, value ? "1" : "0");
	}

	bool WriteLong(const std::string& key, long value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteDouble(const std::string& key, double value) override
	{
		return WriteString(key, std::to_string(value));
	}

private:
	IniFormat _format_;

	std::unordered_map<std::string, std::string> dictionary;
	std::string current_section;
	mutable std::vector<std::string> sections_vector;
	mutable std::vector<std::string> entries_vector;

	static inline void chrarr_tolower(char* const str)
	{
		for (char* chrptr = str; *chrptr; chrptr++) {
			*chrptr = *chrptr > 0x40 && *chrptr < 0x5b ? *chrptr | 0x60 : *chrptr;
		}
	}

	std::vector<std::string> splitPath(const std::string& path) const {
		std::vector<std::string> parts;
		std::istringstream iss(path);
		std::string part;
		while (std::getline(iss, part, PATH_SEPARATOR)) {
			if (!part.empty()) {
				parts.push_back(part);
			}
		}
		return parts;
	}

	std::string joinPath(const std::vector<std::string>& parts) const {
		std::ostringstream oss;
		for (const auto& part : parts) {
			if (!oss.str().empty()) {
				oss << PATH_SEPARATOR;
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
			return current_section + PATH_SEPARATOR;
		}

		if (path[0] == PATH_SEPARATOR) {
			// Absolute path
			return normalizePath(path) + PATH_SEPARATOR;
		}

		std::vector<std::string> parts;
		if (path.substr(0, 2) == PAR_DIR) {
			// Handle relative path with ".."
			parts = splitPath(current_section);
			parts.pop_back(); // Go up one level
			parts.push_back(path.substr(2));
		}
		else if (path[0] == CURR_DIR) {
			// Handle relative path with "."
			parts = splitPath(current_section);
			parts.push_back(path.substr(1));
		}
		else {
			// Relative path
			parts = splitPath(current_section);
			parts.push_back(path);
		}

		return normalizePath(joinPath(parts)) + PATH_SEPARATOR;
	}

	std::string getFullKey(const std::string& key) const
	{
		std::string full_key = current_section.empty() ? key : current_section + PATH_SEPARATOR + key;
	}

	std::string getSource(const std::string& key) const
	{
		std::string full_key = getFullKey(key);
		if (!dictionary.count(full_key))
		{
			return "";
			//throw std::runtime_error("Key not present.");
		}
		return dictionary.at(full_key);
	}

	bool extractString(const std::string& key, std::string& result) const
	{
		const std::string srcstr = getSource(key);
		if (srcstr.empty()) {
			return false;
		}
		size_t len = srcstr.length();
		char* tmp = new char[len + 1];
		memcpy(tmp, srcstr.c_str(), len + 1);
		len = ini_string_parse(tmp, _format_);
		result = std::string(tmp, len);
		delete[] tmp;
		return true;
	}

	static int _push_dispatch_(IniDispatch* const disp, void* const v_dictionary)
	{
#define thismap (reinterpret_cast<std::unordered_map<std::string, std::string> *>(v_dictionary))

		if (disp->type != INI_KEY) {

			return 0;

		}

		size_t idx;
		char* newptr1, * newptr2, * oldptr1, * oldptr2;
		disp->d_len = ini_unquote(disp->data, disp->format);

		/*  remove quoted dots from parent  */
		if (disp->at_len) {

			/*  has parent  */
			newptr1 = newptr2 = new char[disp->at_len + 1];
			*((const char**)&oldptr2) = disp->append_to;

			while ((oldptr1 = oldptr2)) {

				idx = ini_array_shift((const char**)&oldptr2, '.', disp->format);
				newptr1[idx] = '\0';

				while (idx > 0) {

					--idx;
					newptr1[idx] = oldptr1[idx] == '.' ? PATH_SEPARATOR : oldptr1[idx];

				}

				newptr1 += ini_unquote(newptr1, disp->format);
				*newptr1++ = '.';

			}

			idx = newptr1 - newptr2;
			newptr1 = new char[idx + disp->d_len + 1];
			memcpy(newptr1, newptr2, idx);
			delete[] newptr2;
			newptr2 = newptr1 + idx;

		}
		else {

			/*  parent is root  */
			newptr1 = newptr2 = new char[disp->d_len + 1];

		}

		/*  remove dots from key name  */
		idx = disp->d_len + 1;

		do {

			--idx;
			newptr2[idx] = disp->data[idx] == '.' ? PATH_SEPARATOR : disp->data[idx];

		} while (idx > 0);

		if (!disp->format.case_sensitive) {

			chrarr_tolower(newptr1);

		}

		std::string key = std::string(newptr1, newptr2 - newptr1 + disp->d_len);
		delete newptr1;

		/*  check for duplicate keys  */
		if (thismap->count(key)) {

			std::cerr << "`" << key << "` will be overwritten (duplicate key found)\n";
			thismap->erase(key);

		}

		thismap->insert(
			std::pair<std::string, std::string>(
				key,
				disp->value ? std::string(disp->value, disp->v_len) : ""
			)
		);

		return 0;

#undef thismap
	}

};

#endif//_INIFILE_H_