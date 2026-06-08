/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <Version.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#endif

inline std::filesystem::path GetExecutablePath()
{
#if defined(_WIN32)

	// Dynamically grow buffer to avoid MAX_PATH limits
	std::vector<wchar_t> buffer(260);

	while (true)
	{
		DWORD len = GetModuleFileNameW(nullptr, buffer.data(), (DWORD)buffer.size());

		if (len == 0)
			return {};

		if (len < buffer.size() - 1)
			return std::filesystem::path(buffer.data());

		buffer.resize(buffer.size() * 2);
	}

#elif defined(__linux__)

	std::vector<char> buffer(256);

	while (true)
	{
		ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);

		if (len < 0)
			return {};

		if (len < (ssize_t)buffer.size() - 1)
		{
			buffer[len] = '\0';
			return std::filesystem::path(buffer.data());
		}

		buffer.resize(buffer.size() * 2);
	}

#elif defined(__APPLE__)

	uint32_t size = 0;
	_NSGetExecutablePath(nullptr, &size); // get required size

	std::vector<char> buffer(size);

	if (_NSGetExecutablePath(buffer.data(), &size) == 0)
		return std::filesystem::path(buffer.data());

	return {};

#endif
}

inline std::filesystem::path GetExecutableDir()
{
	auto path = GetExecutablePath();
	return path.empty() ? std::filesystem::current_path()
		: path.parent_path();
}

inline std::filesystem::path GetDataRoot()
{
	// 1. Explicit override (always first)
	if (const char* env = std::getenv("FLUORENDER_DATA_PATH"))
	{
		std::filesystem::path p(env);
		if (std::filesystem::exists(p))
			return p;
	}

#if defined(_WIN32)

	auto exeDir = GetExecutableDir();

	// Normal case (installed or post-build copy)
	if (std::filesystem::exists(exeDir / "Scripts"))
		return exeDir;

	// Dev fallback (Visual Studio layout)
	auto parent = exeDir.parent_path();
	if (std::filesystem::exists(parent / "Scripts"))
		return parent;

	return exeDir;

#elif defined(__APPLE__)

	// macOS: inside app bundle Resources
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle)
	{
		CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
		if (resourcesURL)
		{
			char path[PATH_MAX];
			if (CFURLGetFileSystemRepresentation(
				resourcesURL, TRUE,
				(UInt8*)path, PATH_MAX))
			{
				CFRelease(resourcesURL);
				return std::filesystem::path(path);
			}
			CFRelease(resourcesURL);
		}
	}

	return GetExecutableDir();

#elif defined(__linux__)

	// Linux: typically installed under /usr or similar
	auto exeDir = GetExecutableDir();

	std::vector<std::filesystem::path> candidates = {
		exeDir,                            // portable / dev
		exeDir / ".." / "share" / "fluorender",
		"/usr/share/fluorender",
		"/usr/local/share/fluorender"
	};

	for (const auto& p : candidates)
	{
		if (std::filesystem::exists(p / "Scripts"))
			return std::filesystem::weakly_canonical(p);
	}

	return exeDir;

#endif
}

inline std::filesystem::path GetUserSettingsRoot()
{
#ifdef __APPLE__
	const char* home = std::getenv("HOME");
	if (home)
	{
		std::filesystem::path p = home;
		p /= "Library/Application Support/FluoRender";
		return p;
	}

#elif defined(__linux__)
	const char* home = std::getenv("HOME");
	if (home)
	{
		std::filesystem::path p = home;

		// follow XDG spec if available
		const char* xdg = std::getenv("XDG_CONFIG_HOME");
		if (xdg)
			p = xdg;
		else
			p /= ".config";

		p /= "FluoRender";
		return p;
	}
#endif

	return std::filesystem::current_path();
}

inline bool NeedsUserDataUpdate()
{
	auto userDir = GetUserSettingsRoot();
	auto versionFile = userDir / "version.txt";

	// First launch: no directory or no version file
	if (!std::filesystem::exists(userDir))
		return true;

	if (!std::filesystem::exists(versionFile))
		return true;

	// Read stored version
	int storedMajor = 0;
	int storedMinor = 0;

	{
		std::ifstream in(versionFile);
		in >> storedMajor >> storedMinor;
	}

	// Compare major/minor
	if (storedMajor < fluo::VersionMajor)
		return true;

	if (storedMajor == fluo::VersionMajor &&
		storedMinor < fluo::VersionMinor)
		return true;

	return false;
}

inline void InitializeUserSettings()
{
#if defined(__APPLE__) || defined(__linux__)

	auto srcRoot = GetDataRoot();
	auto dstRoot = GetUserSettingsRoot();

	// ALWAYS ensure base directory exists
	std::filesystem::create_directories(dstRoot);

	// Skip if already up-to-date
	if (!NeedsUserDataUpdate())
		return;

	// Directories to copy (must match EXACT names in your data)
	std::vector<std::string> dirsToCopy = {
		"CL_code",
		"Commands",
		"Database",
		"Scripts",
		"Templates"
	};

	for (const auto& dir : dirsToCopy)
	{
		auto src = srcRoot / dir;
		auto dst = dstRoot / dir;

		// ALWAYS create destination directory
		std::filesystem::create_directories(dst);

		// Copy only if source exists
		if (std::filesystem::exists(src))
		{
			try
			{
				std::filesystem::copy(
					src,
					dst,
					std::filesystem::copy_options::recursive |
					std::filesystem::copy_options::overwrite_existing
				);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error copying directory " << dir
					<< ": " << e.what() << std::endl;
			}
		}
		else
		{
			std::cerr << "Warning: source directory missing: "
				<< src << std::endl;
		}
	}

	// Files to copy
	std::vector<std::string> filesToCopy = {
		"fluorender.xml",
		"fluorender_default.xml"
	};

	for (const auto& file : filesToCopy)
	{
		auto src = srcRoot / file;
		auto dst = dstRoot / file;

		if (std::filesystem::exists(src))
		{
			try
			{
				std::filesystem::copy_file(
					src,
					dst,
					std::filesystem::copy_options::overwrite_existing
				);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error copying file " << file
					<< ": " << e.what() << std::endl;
			}
		}
		else
		{
			std::cerr << "Warning: source file missing: "
				<< src << std::endl;
		}
	}

	// Write version file
	try
	{
		std::ofstream out(dstRoot / "version.txt");
		out << fluo::VersionMajor << " "
			<< fluo::VersionMinor;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error writing version file: "
			<< e.what() << std::endl;
	}

	std::cout << "User settings initialized at: "
		<< dstRoot << std::endl;

#endif
}

#endif // DIRECTORY_H