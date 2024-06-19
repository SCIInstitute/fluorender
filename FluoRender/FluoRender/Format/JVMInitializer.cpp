/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#include <JVMInitializer.h>
#include <SettingDlg.h>
#include <Debug.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <sys/stat.h>
#include <sys/types.h>

JVMInitializer* JVMInitializer::m_pJVMInstance = nullptr;
JavaVM* JVMInitializer::m_pJvm = nullptr;
JNIEnv* JVMInitializer::m_pEnv = nullptr;
JavaVMInitArgs JVMInitializer::m_VMargs;
bool JVMInitializer::m_with_fiji = false;

#ifdef _WIN32
HMODULE JVMInitializer::m_jvm_dll = nullptr;
#else
void* JVMInitializer::m_jvm_dll = nullptr;
#endif

#ifdef _WIN32
decltype(&JNI_CreateJavaVM) JVMInitializer::m_createJVM_Ptr = nullptr;
#else
JVMInitializer::CreateJavaVM_t* JVMInitializer::m_createJVM_Ptr = nullptr;
#endif

decltype(&JNIEnv::FindClass) m_FindClass_Ptr = nullptr;


JVMInitializer* JVMInitializer::getInstance(std::vector<std::string> args)
{
	if (m_pJVMInstance == nullptr) {
		if (create_JVM(args) == true)
			m_pJVMInstance = new JVMInitializer();
	}
	return m_pJVMInstance;
}

char JVMInitializer::getPathSeparator()
{
#ifdef _WIN32
	return ';';
#else
	return ':';
#endif
}

bool JVMInitializer::create_JVM(std::vector<std::string> args)
{
	std::string jvm_path;
	std::string ij_path;
	std::string bioformats_path;
	std::string jvm_ij_path = "";
	std::string jvm_bioformats_path = "";

	if (!args.empty())
	{
		jvm_path = args[0]; //inp_settingDlg->getJVMPath();
		ij_path = args[1]; //inp_settingDlg->getIJPath();
		bioformats_path = args[2]; //inp_settingDlg->getBioformatsPath();
		std::string name = ij_path;
		// For Mac: ij_path is going to be ij.app or fiji.app.

#ifdef _WIN32
#else
		name = name + GETSLASHA() + "Contents" + GETSLASHA() + "Java";
#endif
		if (FILE* file = fopen((name + GETSLASHA() + "ij.jar").c_str(), "r"))
		{
			DBGPRINT(L"Inside here.\n");
			m_with_fiji = false;
			jvm_ij_path = name + GETSLASHA() + "ij.jar";
			jvm_bioformats_path = bioformats_path;
			fclose(file);
		}
		else
		{
			name = ij_path + GETSLASHA() + std::string("jars") + GETSLASHA() + std::string("bio-formats");
			wxDir dir(name);
			if (!dir.IsOpened())
			{
				m_with_fiji = false;
				return false;
			}
			else
			{
				m_with_fiji = true;
				wxString filename;
				bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
				while (cont)
				{
					if (filename.Matches("formats-*.jar") || filename.Matches("turbojpeg-*.jar") ||
						filename.Matches("ome-xml*.jar") || filename.Matches("ome-codecs*.jar") ||
						filename.Matches("ome-common*.jar"))
					{
						if (jvm_bioformats_path == "")
							jvm_bioformats_path = dir.GetNameWithSep() + filename;
						else
							jvm_bioformats_path += getPathSeparator() + dir.GetNameWithSep() + filename;
					}
					cont = dir.GetNext(&filename);
				}

				dir.Open(ij_path + GETSLASHA() + std::string("jars"));
				cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
				while (cont)
				{
					if (filename.Matches("commons-collections-*.jar") || filename.Matches("commons-lang-*.jar") ||
						filename.Matches("commons-logging-*.jar") || filename.Matches("guava-*.jar") ||
						filename.Matches("jcommander-*.jar") || filename.Matches("jgoodies-common-*.jar") ||
						filename.Matches("jgoodies-forms-*.jar") || filename.Matches("jhdf5-*.jar") ||
						filename.Matches("joda-time-*.jar") || filename.Matches("kryo-*.jar") ||
						filename.Matches("log4j-*.jar") || filename.Matches("logback-classic*.jar") ||
						filename.Matches("logback-core*.jar") || filename.Matches("metadata-extractor-*.jar") ||
						filename.Matches("minlog-*.jar") || filename.Matches("native-lib-loader-*.jar") ||
						filename.Matches("netcdf-*.jar") || filename.Matches("objenesis-*.jar") ||
						filename.Matches("perf4j-*.jar") || filename.Matches("slf4j-api-*.jar") ||
						filename.Matches("snakeyaml-*.jar") || filename.Matches("xercesImpl-*.jar") ||
						filename.Matches("xml-apis-ext-*.jar") || filename.Matches("xmpcore-*.jar")) {
						if (jvm_bioformats_path == "")
							jvm_bioformats_path = dir.GetNameWithSep() + filename;
						else
							jvm_bioformats_path += getPathSeparator() + dir.GetNameWithSep() + filename;
					}
					else if (filename.Matches("ij-*.jar"))
						jvm_ij_path = dir.GetNameWithSep() + filename;
					cont = dir.GetNext(&filename);
				}

				dir.Open(ij_path + GETSLASHA() + std::string("plugins"));
				cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
				while (cont)
				{
					if (filename.Matches("bio-formats_plugins-*.jar"))
						jvm_bioformats_path += getPathSeparator() + dir.GetNameWithSep() + filename;
					cont = dir.GetNext(&filename);
				}
				//Checking for jvm path.
				if (wxIsEmpty(jvm_path))
				{
#ifdef _DARWIN
					jvm_path = ij_path + GETSLASHA() + "java" + GETSLASHA() + "macosx";
#else
					jvm_path = ij_path + GETSLASHA() + "java" + GETSLASHA() + "win64";
#endif
					dir.Open(jvm_path);
					cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
					while (cont)
					{
						if (filename.Matches("*jdk*"))
						{
							jvm_path = dir.GetNameWithSep() + filename + GETSLASHA();
						}
						cont = dir.GetNext(&filename);
					}
#ifdef _DARWIN
					jvm_path = jvm_path + "jre" + GETSLASHA() + "Contents" + GETSLASHA() + "HOME" + GETSLASHA() + "lib" + GETSLASHA() + "server" + GETSLASHA() + "libjvm.dylib";
#else
					jvm_path = jvm_path + "jre" + GETSLASHA() + "bin" + GETSLASHA() + "server" + GETSLASHA() + "jvm.dll";
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
	m_createJVM_Ptr = (decltype(&JNI_CreateJavaVM))GetProcAddress(m_jvm_dll, "JNI_CreateJavaVM");
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
	using namespace std;
	JavaVMOption* options = new JavaVMOption[3];
	//Geting absolute path to class file.
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	string imageJPath = "-Djava.class.path=";
	imageJPath += exePath.ToStdString() + GETSLASHA() + "Java_Code" + GETSLASHA() + getPathSeparator();
	imageJPath.append(jvm_ij_path + getPathSeparator());
	imageJPath.append(jvm_bioformats_path);

	DBGPRINT(L"%ls\n", s2ws(imageJPath).c_str());

	options[0].optionString = const_cast<char*>(imageJPath.c_str());

	m_VMargs.version = JNI_VERSION_1_6;             // minimum Java version
	m_VMargs.nOptions = 1;                          // number of options
	m_VMargs.options = options;
	m_VMargs.ignoreUnrecognized = false;     // invalid options make the JVM init fail

	try
	{
		jint rc = m_createJVM_Ptr(&m_pJvm, (void**)&m_pEnv, &m_VMargs);
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
		FreeLibrary(m_jvm_dll);
#else
	if (m_jvm_dll)
		dlclose(m_jvm_dll);
#endif
}
