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

#define DOT_REPLACEMENT '\\'
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
		this->dictionary.clear();
		this->_format_ = __INI_MAP_DEFAULT_FORMAT__;
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

		for (const auto& pair : dictionary) {
			size_t pos = pair.first.find('\\');
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

		for (const auto& pair : dictionary) {
			size_t pos = pair.first.find('\\');
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
		return dictionary.count(path) ? true : false;
	}

	bool SetPath(const std::string& path) override {
		currentSection = ini_get_section(ini, path.c_str());
		return currentSection != nullptr;
	}

	std::string GetPath() const override {
		return currentSection->name;
	}

	bool HasGroup(const std::string& group) const override {
		return ini_get_section(ini, group.c_str()) != nullptr;
	}

	bool HasEntry(const std::string& entry) const override {
		return ini_get_entry(currentSection, entry.c_str()) != nullptr;
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override {
		if (ini->sections) {
			*group = ini->sections->name;
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextGroup(std::string* group, long* index) const override {
		const ini_section_t* section = ini->sections;
		for (long i = 0; i <= *index; ++i) {
			section = section->next;
		}
		if (section) {
			*group = section->name;
			(*index)++;
			return true;
		}
		return false;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override {
		if (currentSection->entries) {
			*entry = currentSection->entries->name;
			*index = 0;
			return true;
		}
		return false;
	}

	bool GetNextEntry(std::string* entry, long* index) const override {
		const ini_entry_t* entryPtr = currentSection->entries;
		for (long i = 0; i <= *index; ++i) {
			entryPtr = entryPtr->next;
		}
		if (entryPtr) {
			*entry = entryPtr->name;
			(*index)++;
			return true;
		}
		return false;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override {
		ini_remove_entry(currentSection, key.c_str());
		return true;
	}

	bool DeleteGroup(const std::string& group) override {
		ini_remove_section(ini, group.c_str());
		return true;
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
	bool WriteString(const std::string& key, const std::string& value) override {
		ini_set_entry(currentSection, key.c_str(), value.c_str());
		return true;
	}

	bool WriteLong(const std::string& key, long value) override {
		ini_set_entry(currentSection, key.c_str(), std::to_string(value).c_str());
		return true;
	}

	bool WriteDouble(const std::string& key, double value) override {
		ini_set_entry(currentSection, key.c_str(), std::to_string(value).c_str());
		return true;
	}

private:
	IniFormat _format_;

	std::unordered_map<std::string, std::string> dictionary;
	std::string current_section;

	static inline void chrarr_tolower(char* const str)
	{
		for (char* chrptr = str; *chrptr; chrptr++) {
			*chrptr = *chrptr > 0x40 && *chrptr < 0x5b ? *chrptr | 0x60 : *chrptr;
		}
	}

	std::string getSource(const std::string& key) const
	{
		std::string full_key = current_section.empty() ? key : current_section + "\\" + key;
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
					newptr1[idx] = oldptr1[idx] == '.' ? DOT_REPLACEMENT : oldptr1[idx];

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
			newptr2[idx] = disp->data[idx] == '.' ? DOT_REPLACEMENT : disp->data[idx];

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