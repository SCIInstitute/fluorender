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
		dictionary_()
	{
		path_sep_ = "/";
		path_sep_s_ = path_sep_;
		cd_sep_ = ".";
		pd_sep_ = "..";
	}

	int LoadFile(const std::wstring& filename) override
	{
		dictionary_.clear();
		cur_path_ = "";
		_format_ = __INI_MAP_DEFAULT_FORMAT__;
#ifdef _WIN32
		std::wstring long_name = L"\x5c\x5c\x3f\x5c" + filename;
#else
		std::wstring long_name = filename;
#endif
		std::string str = ws2s(long_name);
		return load_ini_path(str.c_str(), __INI_MAP_DEFAULT_FORMAT__, NULL, _push_dispatch_, &dictionary_);
	}

	int LoadStringConf(const std::string& ini_string) override
	{
		dictionary_.clear();
		cur_path_ = "";
		_format_ = __INI_MAP_DEFAULT_FORMAT__;
		size_t len = ini_string.length();
		char* tmp = new char[len + 1];
		memcpy(tmp, ini_string.c_str(), len + 1);
		int retval = strip_ini_cache(tmp, len, __INI_MAP_DEFAULT_FORMAT__, NULL, _push_dispatch_, &dictionary_);
		delete[] tmp;
		return retval;
	}

	int LoadData(const std::unordered_map<std::string, std::string>& data) override
	{
		dictionary_ = data;
		return 0;
	}

	std::unordered_map<std::string, std::string> GetData() override
	{
		return dictionary_;
	}

	int SaveFile(const std::wstring& filename) override
	{
#ifdef _WIN32
		std::wstring long_name = L"\x5c\x5c\x3f\x5c" + filename;
        std::ofstream file(long_name);
#else
		std::wstring long_name = filename;
        std::ofstream file(ws2s(long_name));
#endif
		if (!file.is_open()) {
			return CONFINI_EIO;
		}

		auto sorted_dict = SortingUtility::getSortedMap(dictionary_);

		std::string current_section;
		for (const auto& pair : sorted_dict) {
			std::string full_key = pair.first;
			size_t pos = 0;
			std::string section;
			std::string key = full_key;

			// Determine the section and key
			while ((pos = key.find(path_sep_)) != std::string::npos) {
				section += (section.empty() ? "" : path_sep_) + key.substr(0, pos);
				key = key.substr(pos + 1);
			}

			// Print the section if it's different from the current section
			if (section != current_section) {
				current_section = section;
				file << "[" << current_section << "]\n";
			}

			file << key << " = " << pair.second << "\n";
		}

		file.close();
		return CONFINI_SUCCESS;
	}

	int SaveString(std::string& str) override
	{
		std::ostringstream oss;

		std::map<std::string, std::string> sorted_dict(dictionary_.begin(), dictionary_.end());

		for (const auto& pair : sorted_dict) {
			size_t pos = pair.first.find(path_sep_);
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
		std::string str = getFullPath(path);
		for (const auto& it : dictionary_)
		{
			if (it.first.find(str) == 0)
				return true;
		}
		return false;
	}

	bool SetPath(const std::string& path) override
	{
		cur_path_ = getFullPath(path);
		return !cur_path_.empty();
	}

	std::string GetPath() const override
	{
		return cur_path_;
	}

	bool HasGroup(const std::string& group) const override
	{
		std::string prefix = cur_path_ + group + path_sep_;
		for (const auto& pair : dictionary_)
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
		return dictionary_.find(full_key) != dictionary_.end();
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override
	{
		if (dictionary_.empty()) {
			return false;
		}

		std::set<std::string> unique_sections;
		for (const auto& pair : dictionary_) {
			size_t pos = pair.first.find(path_sep_);
			std::string section = (pos == std::string::npos) ? pair.first : pair.first.substr(0, pos);
			unique_sections.insert(section);
		}

		if (unique_sections.empty()) {
			return false;
		}

		*group = *unique_sections.begin();
		*index = 0;
		sections_vector_.assign(unique_sections.begin(), unique_sections.end());
		return true;
	}

	bool GetNextGroup(std::string* group, long* index) const override
	{
		if (*index + 1 >= sections_vector_.size()) {
			return false;
		}

		*index += 1;
		*group = sections_vector_[*index];
		return true;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override
	{
		if (dictionary_.empty() || cur_path_.empty()) {
			return false;
		}

		entries_vector_.clear();
		std::string prefix = cur_path_;
		for (const auto& pair : dictionary_) {
			if (pair.first.find(prefix) == 0) {
				entries_vector_.push_back(pair.first.substr(prefix.length()));
			}
		}

		if (entries_vector_.empty()) {
			return false;
		}

		*entry = entries_vector_[0];
		*index = 0;
		return true;
	}

	bool GetNextEntry(std::string* entry, long* index) const override
	{
		if (*index + 1 >= entries_vector_.size()) {
			return false;
		}

		*index += 1;
		*entry = entries_vector_[*index];
		return true;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override
	{
		std::string full_key = getFullKey(key);
		auto it = dictionary_.find(full_key);
		if (it != dictionary_.end()) {
			dictionary_.erase(it);
			return true;
		}
		return false;
	}

	bool DeleteGroup(const std::string& group) override
	{
		std::string prefix = getFullPath(group);
		bool found = false;
		for (auto it = dictionary_.begin(); it != dictionary_.end(); ) {
			if (it->first.find(prefix) == 0 || it->first == group) {
				it = dictionary_.erase(it);
				found = true;
			}
			else {
				++it;
			}
		}
		return found;
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
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = str;
		return true;
	}

	bool ReadWstring(const std::string& key, std::wstring* value, const std::wstring& def = L"") const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = s2ws(str);
		return true;
	}

	bool ReadBool(const std::string& key, bool* value, bool def = false) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}

		// Convert the string to lowercase for case-insensitive comparison
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		if (str == "1" || str == "true")
		{
			*value = true;
		}
		else if (str == "0" || str == "false")
		{
			*value = false;
		}
		else
		{
			*value = def;
			return false;
		}

		return true;
	}

	bool ReadLong(const std::string& key, long* value, long def = 0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = STOL(str);
		return true;
	}

	bool ReadULong(const std::string& key, unsigned long* value, unsigned long def = 0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = STOUL(str);
		return true;
	}

	bool ReadInt(const std::string& key, int* value, int def = 0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = STOI(str);
		return true;
	}

	bool ReadUInt(const std::string& key, unsigned int* value, unsigned int def = 0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = STOUL(str);
		return true;
	}

	bool ReadSizeT(const std::string& key, size_t* value, size_t def = 0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = STOULL(str);
		return true;
	}

	bool ReadShort(const std::string& key, short* value, short def = 0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = static_cast<short>(STOI(str));
		return true;
	}

	bool ReadUShort(const std::string& key, unsigned short* value, unsigned short def = 0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = static_cast<unsigned short>(STOUL(str));
		return true;
	}

	bool ReadDouble(const std::string& key, double* value, double def = 0.0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = STOD(str);
		return true;
	}

	bool ReadFloat(const std::string& key, float* value, float def = 0.0) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = STOF(str);
		return true;
	}

	bool ReadColor(const std::string& key, fluo::Color* value, const fluo::Color& def = fluo::Color(0.0)) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = fluo::Color(str);
		return true;
	}

	bool ReadHSVColor(const std::string& key, fluo::HSVColor* value, const fluo::HSVColor& def = fluo::HSVColor()) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = fluo::HSVColor(str);
		return true;
	}

	bool ReadPoint(const std::string& key, fluo::Point* value, const fluo::Point& def = fluo::Point(0.0)) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = fluo::Point(str);
		return true;
	}

	bool ReadVector(const std::string& key, fluo::Vector* value, const fluo::Vector& def = fluo::Vector(0.0)) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = fluo::Vector(str);
		return true;
	}

	bool ReadQuaternion(const std::string& key, fluo::Quaternion* value, const fluo::Quaternion& def = fluo::Quaternion()) const override
	{
		std::string str;
		if (!extractString(key, str))
		{
			*value = def;
			return false;
		}
		*value = fluo::Quaternion(str);
		return true;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override
	{
		std::string full_key = getFullKey(key);
		dictionary_.insert(std::pair<std::string, std::string>(full_key, value));
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

	bool WriteULong(const std::string& key, unsigned long value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteInt(const std::string& key, int value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteUInt(const std::string& key, unsigned int value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteSizeT(const std::string& key, size_t value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteShort(const std::string& key, short value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteUShort(const std::string& key, unsigned short value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteDouble(const std::string& key, double value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteFloat(const std::string& key, float value) override
	{
		return WriteString(key, std::to_string(value));
	}

	bool WriteColor(const std::string& key, const fluo::Color& value) override
	{
		return WriteString(key, value.to_string());
	}

	bool WriteHSVColor(const std::string& key, const fluo::HSVColor& value) override
	{
		return WriteString(key, value.to_string());
	}

	bool WritePoint(const std::string& key, const fluo::Point& value) override
	{
		return WriteString(key, value.to_string());
	}

	bool WriteVector(const std::string& key, const fluo::Vector& value) override
	{
		return WriteString(key, value.to_string());
	}

	bool WriteQuaternion(const std::string& key, const fluo::Quaternion& value) override
	{
		return WriteString(key, value.to_string());
	}

private:
	static std::string path_sep_s_;
	IniFormat _format_;

	std::unordered_map<std::string, std::string> dictionary_;
	mutable std::vector<std::string> sections_vector_;
	mutable std::vector<std::string> entries_vector_;

	std::string getSource(const std::string& key) const
	{
		std::string full_key = getFullKey(key);
		if (!dictionary_.count(full_key))
		{
			return "";
			//throw std::runtime_error("Key not present.");
		}
		return dictionary_.at(full_key);
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

		std::string new_parent, new_key;
		disp->d_len = ini_unquote(disp->data, disp->format);

		/*  remove quoted dots and "\ " from parent (section) name  */
		if (disp->at_len) {
			/*  has parent  */
			std::string parent(disp->append_to, disp->at_len);
			size_t pos = 0;
			while ((pos = parent.find('.', pos)) != std::string::npos) {
				parent.replace(pos, 1, path_sep_s_);
				pos += path_sep_s_.length();
			}
			// Replace "\ " with " " in the parent (section) name
			pos = 0;
			while ((pos = parent.find("\\ ", pos)) != std::string::npos) {
				parent.replace(pos, 2, " ");
			}
			new_parent = parent;
		}

		/*  remove dots and "\ " from key name  */
		std::string key(disp->data, disp->d_len);
		size_t pos = 0;
		while ((pos = key.find('.', pos)) != std::string::npos) {
			key.replace(pos, 1, path_sep_s_);
			pos += path_sep_s_.length();
		}
		// Replace "\ " with " " in the key name
		pos = 0;
		while ((pos = key.find("\\ ", pos)) != std::string::npos) {
			key.replace(pos, 2, " ");
		}
		new_key = key;

		if (!disp->format.case_sensitive) {
			std::transform(new_key.begin(), new_key.end(), new_key.begin(), ::tolower);
			std::transform(new_parent.begin(), new_parent.end(), new_parent.begin(), ::tolower);
		}

		std::string full_key = path_sep_s_ + (new_parent.empty() ? new_key : new_parent + path_sep_s_ + new_key);

		/*  check for duplicate keys  */
		if (thismap->count(full_key)) {
			std::cerr << "`" << full_key << "` will be overwritten (duplicate key found)\n";
			thismap->erase(full_key);
		}

		thismap->insert(
			std::pair<std::string, std::string>(
				full_key,
				disp->value ? std::string(disp->value, disp->v_len) : ""
			)
		);

		return 0;

#undef thismap
	}
};

#endif//_INIFILE_H_
