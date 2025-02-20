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

class IniFile : public BaseTreeFile
{
public:
	IniFile(const std::string& filename) {
		ini = ini_load(filename.c_str(), NULL);
		currentSection = ini->sections;
	}

	~IniFile() {
		ini_free(ini);
	}

	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value) const override {
		const ini_entry_t* entry = ini_get_entry(currentSection, key.c_str());
		if (entry) {
			*value = entry->value;
			return true;
		}
		return false;
	}

	bool ReadLong(const std::string& key, long* value) const override {
		const ini_entry_t* entry = ini_get_entry(currentSection, key.c_str());
		if (entry) {
			*value = std::stol(entry->value);
			return true;
		}
		return false;
	}

	bool ReadDouble(const std::string& key, double* value) const override {
		const ini_entry_t* entry = ini_get_entry(currentSection, key.c_str());
		if (entry) {
			*value = std::stod(entry->value);
			return true;
		}
		return false;
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

	// Implement group management methods
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

private:
	ini_t* ini;
	ini_section_t* currentSection;
};

#endif//_INIFILE_H_