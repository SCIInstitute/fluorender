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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <JVMInitializer.h>
#include <compatibility.h>
#include <Debug.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#ifdef __linux__
#include <dlfcn.h>
#endif

decltype(&JNIEnv::FindClass) m_FindClass_Ptr = nullptr;

JVMInitializer::JVMInitializer(const std::vector<std::string>& args)
{
	m_VMargs = std::make_unique<JavaVMInitArgs>();
	m_valid = create_JVM(args);
}

JVMInitializer::~JVMInitializer()
{
	destroyJVM();
}

bool JVMInitializer::create_JVM(const std::vector<std::string>& args)
{
	std::string jvm_path;
	std::string ij_path;
	std::string bioformats_path;
	std::string jvm_ij_path = "";
	std::string jvm_bioformats_path = "";
	std::filesystem::path p;

	if (!args.empty())
	{
		jvm_path = args[0]; //inp_settingDlg->getJVMPath();
		ij_path = args[1]; //inp_settingDlg->getIJPath();
		bioformats_path = args[2]; //inp_settingDlg->getBioformatsPath();
		// For Mac: ij_path is going to be ij.app or fiji.app.

#ifdef _WIN32
		p = std::filesystem::path(ij_path) / "ij.jar";
#else
		p = std::filesystem::path(ij_path) / "Contents" / "Java" / "ij.jar";
#endif
		std::string name = p.string();
		if (FILE* file = fopen(name.c_str(), "r"))
		{
			DBGPRINT(L"Inside here.\n");
			m_with_fiji = false;
			jvm_ij_path = name;
			jvm_bioformats_path = bioformats_path;
			fclose(file);
		}
		else
		{
			p = std::filesystem::path(ij_path) / "jars" / "bio-formats";
			name = p.string();
			if (!std::filesystem::exists(name) ||
				!std::filesystem::is_directory(name))
			{
				m_with_fiji = false;
				return false;
			}
			else
			{
				m_with_fiji = true;
				std::string filename;
				for (const auto& entry : std::filesystem::directory_iterator(name))
				{
					std::string filename = entry.path().filename().string();
					if (filename.find("formats-") != std::string::npos || filename.find("turbojpeg-") != std::string::npos ||
						filename.find("ome-xml") != std::string::npos || filename.find("ome-codecs") != std::string::npos ||
						filename.find("ome-common") != std::string::npos) {
						if (jvm_bioformats_path.empty())
							jvm_bioformats_path = (p / filename).string();
						else
							jvm_bioformats_path += getPathSeparator() + (p / filename).string();
					}
				}

				p = std::filesystem::path(ij_path) / "jars";
				name = p.string();
				for (const auto& entry : std::filesystem::directory_iterator(name)) {
					std::string filename = entry.path().filename().string();
					if (filename.find("commons-collections-") != std::string::npos || filename.find("commons-lang-") != std::string::npos ||
						filename.find("commons-logging-") != std::string::npos || filename.find("guava-") != std::string::npos ||
						filename.find("jcommander-") != std::string::npos || filename.find("jgoodies-common-") != std::string::npos ||
						filename.find("jgoodies-forms-") != std::string::npos || filename.find("jhdf5-") != std::string::npos ||
						filename.find("joda-time-") != std::string::npos || filename.find("kryo-") != std::string::npos ||
						filename.find("log4j-") != std::string::npos || filename.find("logback-classic") != std::string::npos ||
						filename.find("logback-core") != std::string::npos || filename.find("metadata-extractor-") != std::string::npos ||
						filename.find("minlog-") != std::string::npos || filename.find("native-lib-loader-") != std::string::npos ||
						filename.find("netcdf-") != std::string::npos || filename.find("objenesis-") != std::string::npos ||
						filename.find("perf4j-") != std::string::npos || filename.find("slf4j-api-") != std::string::npos ||
						filename.find("snakeyaml-") != std::string::npos || filename.find("xercesImpl-") != std::string::npos ||
						filename.find("xml-apis-ext-") != std::string::npos || filename.find("xmpcore-") != std::string::npos) {
						if (jvm_bioformats_path.empty())
							jvm_bioformats_path = (p / filename).string();
						else
							jvm_bioformats_path += getPathSeparator() + (p / filename).string();
					}
					else if (filename.find("ij-") != std::string::npos) {
						jvm_ij_path = (p / filename).string();
					}
				}

				p = std::filesystem::path(ij_path) / "plugins";
				name = p.string();
				for (const auto& entry : std::filesystem::directory_iterator(name)) {
					std::string filename = entry.path().filename().string();
					if (filename.find("bio-formats_plugins-") != std::string::npos) {
						jvm_bioformats_path += getPathSeparator() + (p / filename).string();
					}
				}

				//Checking for jvm path.
				if (jvm_path.empty())
				{
					p = std::filesystem::path(ij_path) / "java";
#ifdef _DARWIN
					p /= "macosx";
#else
					p /= "win64";
#endif
					jvm_path = p.string();
					for (const auto& entry : std::filesystem::directory_iterator(jvm_path)) {
						std::string filename = entry.path().filename().string();
						if (filename.find("jdk") != std::string::npos) {
							p = entry.path();
							jvm_path = p.string();
						}
					}

#ifdef _DARWIN
					p = std::filesystem::path(jvm_path) / "jre" / "Contents" / "Home" / "lib" / "server" / "libjvm.dylib";
					jvm_path = p.string();
#else
					p = std::filesystem::path(jvm_path) / "jre" / "bin" / "server" / "jvm.dll";
					jvm_path = p.string();
#endif
				}
			}
		}
	}

	// The jvm lib is inside the fiji.app. So we actually need only the first path.

	//Loading JVM library and methods.
#ifdef _DARWIN
	if (const char* error = dlerror())
		DBGPRINT(L"Flushed existing error: %s\n", error);
#endif

	//jvm_path = "/Users/dev/Downloads/ImageJ/jre/lib/server/libjvm.dylib";
#ifdef _WIN32
	m_jvm_dll = LoadLibrary(s2ws(jvm_path).c_str());
#else
	m_jvm_dll = dlopen((const char*)jvm_path.c_str(), RTLD_NOW);
#endif
	if (m_jvm_dll == nullptr)
	{
		DBGPRINT(L"Error while opening jvm library.\n");
		return false;
	}

#ifdef _DARWIN
	if (const char* error = dlerror())
		DBGPRINT(L"Err: %s\n", error);
#endif

#ifdef _WIN32
	m_createJVM_Ptr = (decltype(&JNI_CreateJavaVM))GetProcAddress(HMODULE(m_jvm_dll), "JNI_CreateJavaVM");
#else
	m_createJVM_Ptr = (CreateJavaVM_t*)dlsym(m_jvm_dll, "JNI_CreateJavaVM");
#endif
	if (m_createJVM_Ptr == nullptr)
	{
		DBGPRINT(L"Error while getting JNI_CreateJavaVM funciton address.\n");
		return false;
	}

#ifdef _DARWIN
	if (const char* error = dlerror())
		DBGPRINT(L"Err: %s\n", error);
#endif


	DBGPRINT(L"\n");
	DBGPRINT(L"%lsJVM\n", s2ws(jvm_path).c_str());
	DBGPRINT(L"%lsIJ\n", s2ws(jvm_ij_path).c_str());
	DBGPRINT(L"%ls\n", s2ws(jvm_bioformats_path).c_str());
	DBGPRINT(L"\n");

	JavaVMOption* options = new JavaVMOption[3];
	//Geting absolute path to class file.
	std::filesystem::path currentPath = std::filesystem::current_path();
	std::string exePath = currentPath.string();
	std::string imageJPath = "-Djava.class.path=";
	p = exePath; p /= "Java_Code"; p /= "";
	imageJPath += p.string() + getPathSeparator();
	imageJPath.append(jvm_ij_path + getPathSeparator());
	imageJPath.append(jvm_bioformats_path);

	DBGPRINT(L"%ls\n", s2ws(imageJPath).c_str());

	options[0].optionString = const_cast<char*>(imageJPath.c_str());

	m_VMargs->version = JNI_VERSION_1_6;             // minimum Java version
	m_VMargs->nOptions = 1;                          // number of options
	m_VMargs->options = options;
	m_VMargs->ignoreUnrecognized = false;     // invalid options make the JVM init fail

	try
	{
		jint rc = m_createJVM_Ptr(&m_pJvm, (void**)&m_pEnv, m_VMargs.get());
		delete[] options;
		if (rc != JNI_OK)
		{
			DBGPRINT(L"Error while calling JNI_CreteJavaVM.\n");
			return false;
		}
	}
	catch (...)
	{
		DBGPRINT(L"Excption while creating jvm. Nothing to worry about!\n");
	}
	DBGPRINT(L"JVM created successfully.\n");
	jint ver = m_pEnv->GetVersion();
	DBGPRINT(L"%d.%d\n", (ver >> 16) & 0x0f, ver & 0x0f);
	return true;
}

void JVMInitializer::destroyJVM()
{
	if (m_pJvm)
		m_pJvm->DestroyJavaVM();
#ifdef _WIN32
	if (m_jvm_dll)
		FreeLibrary(HMODULE(m_jvm_dll));
#else
	if (m_jvm_dll)
		dlclose(m_jvm_dll);
#endif
}
