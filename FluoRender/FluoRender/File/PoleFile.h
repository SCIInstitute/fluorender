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
#include <compatibility.h>
#include <sstream>

class PoleFile : public BaseTreeFile
{
public:
	PoleFile() :
		storage_(0),
		stream_(0)
	{
	}

	~PoleFile()
	{
		if (stream_)
			delete stream_;
		if (storage_)
		{
			storage_->close();
			delete storage_;
		}
	}

	int LoadFile(const std::string& filename) override
	{
		storage_ = new POLE::Storage(filename.c_str());
		if (storage_)
		{
			storage_->open();
			return 0;
		}
		return 1;
	}

	int LoadString(const std::string& ini_string) override
	{
		return 1;
	}

	int SaveFile(const std::string& filename) override
	{
		storage_ = new POLE::Storage(filename.c_str());
		if (storage_)
		{
			storage_->open(true, true);
			return 0;
		}
		return 1;
	}

	int SaveString(std::string& str) override
	{
		return 1;
	}

	// Implement group management methods
	bool Exists(const std::string& path) const override
	{
		if (!storage_)
			return false;
		return storage_->exists(path);
	}

	bool SetPath(const std::string& path) override
	{
		if (path.empty() || path.find("..") != std::string::npos) {
			return false;
		}
		if (!storage_)
			return false;
		current_path_ = path;
		groups_.clear();
		group_index_ = -1;
		entries_.clear();
		entry_index_ = -1;

		std::list<std::string> subpaths = storage_->entries(current_path_);
		if (subpaths.empty())
		{
			if (stream_)
				delete stream_;
			stream_ = new POLE::Stream(storage_, current_path_);

			if (!stream_)
				return false;
			if (stream_->eof() || stream_->fail()) {
				return false;
			}

			//allocate 
			std::vector<unsigned char> buffer(stream_->size());
			//read
			if (stream_->read(buffer.data(), buffer.size()))
			{
				std::string content(buffer.begin(), buffer.end());

				// Parse the content as key-value pairs
				std::istringstream ss(content);
				std::string line;
				while (std::getline(ss, line)) {
					std::istringstream lineStream(line);
					std::string key;
					if (std::getline(lineStream, key, '=')) {
						std::string value;
						if (std::getline(lineStream, value)) {
							entries_.emplace_back(key, value);
						}
					}
				}
			}
		}
		return true;
	}

	std::string GetPath() const override {
		return current_path_;
	}

	bool HasGroup(const std::string& group) const override
	{
		if (!storage_)
			return false;

		std::list<std::string> subpaths = storage_->entries(current_path_);
		return std::find(subpaths.begin(), subpaths.end(), group) != subpaths.end();
	}

	bool HasEntry(const std::string& entry) const override
	{
		for (const auto& kv : entries_) {
			if (kv.first == entry) {
				return true;
			}
		}
		return false;
	}

	// Implement enumeration methods
	bool GetFirstGroup(std::string* group, long* index) const override
	{
		if (!storage_)
			return false;

		groups_ = std::vector<std::string>(
			storage_->entries(current_path_).begin(),
			storage_->entries(current_path_).end());
		if (groups_.empty()) {
			return false;
		}
		group_index_ = 0;
		*group = groups_[group_index_];
		*index = group_index_;
		return true;
	}

	bool GetNextGroup(std::string* group, long* index) const override
	{
		if (group_index_ < 0 || group_index_ >= static_cast<long>(groups_.size()) - 1) {
			return false;
		}
		group_index_++;
		*group = groups_[group_index_];
		*index = group_index_;
		return true;
	}

	bool GetFirstEntry(std::string* entry, long* index) const override
	{
		if (entries_.empty()) {
			return false;
		}
		entry_index_ = 0;
		*entry = entries_[entry_index_].first;
		*index = entry_index_;
		return true;
	}

	bool GetNextEntry(std::string* entry, long* index) const override
	{
		if (entry_index_ < 0 || entry_index_ >= static_cast<long>(entries_.size()) - 1) {
			return false;
		}
		entry_index_++;
		*entry = entries_[entry_index_].first;
		*index = entry_index_;
		return true;
	}

	// Implement deletion methods
	bool DeleteEntry(const std::string& key) override
	{
		// Find and remove the entry from the entries vector
		auto it = std::remove_if(entries_.begin(), entries_.end(), [&](auto kv)
			{
				return kv.first == key;
			});

		if (it == entries_.end()) {
			return false; // Key not found
		}

		entries_.erase(it, entries_.end());

		// Update the stream with the modified entries
		std::ostringstream oss;
		for (const auto& kv : entries_) {
			oss << kv.first << "=" << kv.second << "\n";
		}

		std::string updatedContent = oss.str();
		std::vector<unsigned char> buffer(updatedContent.begin(), updatedContent.end());

		// Write the updated content to the stream
		if (stream_)
			stream_->write(buffer.data(), buffer.size());

		return true;
	}

	bool DeleteGroup(const std::string& group) override
	{
		if (!storage_)
			return false;

		// Construct the full path to the group
		std::string groupPath = current_path_ + "/" + group;

		// Use pole::Storage::deleteByName to delete the group
		return storage_->deleteByName(groupPath);
	}

protected:
	// Implement type-specific read methods
	bool ReadString(const std::string& key, std::string* value) const override
	{
		// Search for the key in the entries vector
		for (const auto& kv : entries_) {
			if (kv.first == key) {
				*value = kv.second;
				return true;
			}
		}
		return false; // Key not found
	}

	bool ReadWstring(const std::string& key, std::wstring* value) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = s2ws(str);
			return true;
		}
		return false;
	}

	bool ReadBool(const std::string& key, bool* value) const override
	{
		long lval;
		if (ReadLong(key, &lval))
		{
			*value = !(lval & 2);
			return true;
		}
		return false;
	}

	bool ReadLong(const std::string& key, long* value) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = std::stol(str);
			return true;
		}
		return false;
	}

	bool ReadDouble(const std::string& key, double* value) const override
	{
		std::string str;
		if (ReadString(key, &str))
		{
			*value = std::stod(str);
			return true;
		}
		return false;
	}

	// Implement type-specific write methods
	bool WriteString(const std::string& key, const std::string& value) override
	{
		// Search for the key in the entries vector
		auto it = std::find_if(entries_.begin(), entries_.end(), [&](auto kv) {
			return kv.first == key;
			});

		if (it != entries_.end()) {
			// Key found, update the value
			it->second = value;
		}
		else {
			// Key not found, add a new entry
			entries_.emplace_back(key, value);
		}

		// Update the stream with the modified entries
		std::ostringstream oss;
		for (const auto& kv : entries_) {
			oss << kv.first << "=" << kv.second << "\n";
		}

		std::string updatedContent = oss.str();
		std::vector<unsigned char> buffer(updatedContent.begin(), updatedContent.end());

		// Write the updated content to the stream
		if (stream_)
			stream_->write(buffer.data(), buffer.size());

		return true;
	}

	bool WriteWstring(const std::string& key, const std::wstring& value) override
	{
		std::string str = ws2s(value);
		return WriteString(key, str);
	}

	bool WriteBool(const std::string& key, bool value) override
	{
		std::string str = value ? "1" : "0";
		return WriteString(key, str);
	}

	bool WriteLong(const std::string& key, long value) override
	{
		std::string str = std::to_string(value);
		return WriteString(key, str);
	}

	bool WriteDouble(const std::string& key, double value) override
	{
		std::string str = std::to_string(value);
		return WriteString(key, str);
	}

private:
	POLE::Storage* storage_;
	POLE::Stream* stream_;
	std::string current_path_;
	mutable std::vector<std::string> groups_;
	mutable long group_index_;
	mutable std::vector<std::pair<std::string, std::string>> entries_;
	mutable long entry_index_;
};

#endif//_POLEFILE_H_