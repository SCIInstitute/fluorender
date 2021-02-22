#include "JVMInitializer.hpp"
//#include "SettingDlg.h"
//#include <wx/stdpaths.h>
//#include <wx/dir.h>
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

//filenames to match
std::vector<std::regex> JVMInitializer::jstr1
{
	std::regex("(formats-)(.*)(\.jar)"),
	std::regex("(turbojpeg-)(.*)(\.jar)"),
	std::regex("(ome-xml)(.*)(\.jar)"),
	std::regex("(ome-codecs)(.*)(\.jar)"),
	std::regex("(ome-common)(.*)(\.jar)")
};


JVMInitializer* JVMInitializer::getInstance(std::vector<std::string> args){
	if (m_pJVMInstance == nullptr){
		if(create_JVM(args) == true)
			m_pJVMInstance = new JVMInitializer();		
	}
	return m_pJVMInstance;
}

char JVMInitializer::getPathSeparator(){
#ifdef _WIN32
	return ';';
#else
	return ':';
#endif
}

#ifdef _WIN32
bool JVMInitializer::create_JVM(std::vector<std::string> args)
{
	if (args.empty())
		return false;

	std::string jvm_path(args[0]);//inp_settingDlg->getJVMPath();
	std::string ij_path(args[1]);//inp_settingDlg->getIJPath();
	std::string bioformats_path(args[2]);//inp_settingDlg->getBioformatsPath();
	std::string jvm_ij_path = "";
	std::string jvm_bioformats_path = "";
	std::string name = ij_path;

	if (FILE *file = fopen((name + SLASH() + "ij.jar").c_str(), "r"))
	{
		std::cout << "Inside here.";
		m_with_fiji = false;
		jvm_ij_path = name + SLASH() + "ij.jar";
		jvm_bioformats_path = bioformats_path;
		fclose(file);
	}
	else
	{
		std::string filename;
		name = ij_path + SLASH() + "jars" + SLASH() + "bio-formats";
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(name.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			m_with_fiji = true;
			bool cont = true;
			while (cont)
			{
				filename = FindFileData.cFileName;
				if (match_jstr(filename, jstr1))
				{
					if (jvm_bioformats_path == "")
						jvm_bioformats_path = name + SLASH() + filename;
					else
						jvm_bioformats_path += getPathSeparator() + name + SLASH() + filename;
				}
				cont = FindNextFile(hFind, &FindFileData);
			}
		}
		else
		{
			m_with_fiji = false;
			return false;
		}
		FindClose(hFind);

		name = ij_path + SLASH() + "jars";
		hFind = FindFirstFile(name.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool cont = true;
			while (cont)
			{
				filename = FindFileData.cFileName;
				if (match_jstr(filename, jstr2))
				{
					if (jvm_bioformats_path == "")
						jvm_bioformats_path = name + SLASH() + filename;
					else
						jvm_bioformats_path += getPathSeparator() + name + SLASH() + filename;
				}
				else if (std::regex_match(filename, std::regex("(ij-)(.*)(\.jar)")))
					jvm_ij_path = name + SLASH() + filename;
				cont = FindNextFile(hFind, &FindFileData);
			}
		}
		FindClose(hFind);

		name = ij_path + SLASH() + "plugins";
		hFind = FindFirstFile(name.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool cont = true;
			while (cont)
			{
				filename = FindFileData.cFileName;
				if (std::regex_match(filename, std::regex("(bio-formats_plugins-)(.*)(\.jar)")))
					jvm_bioformats_path += getPathSeparator() + name + SLASH() + filename;
				cont = FindNextFile(hFind, &FindFileData);
			}
		}
		else
		{
			m_with_fiji = false;
			return false;
		}
		FindClose(hFind);

		//Checking for jvm path.
		if (jvm_path.empty())
		{
			jvm_path = ij_path + SLASH() + "java" + SLASH() + "win64";
			hFind = FindFirstFile(name.c_str(), &FindFileData);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				bool cont = true;
				while (cont)
				{
					filename = FindFileData.cFileName;
					if (std::regex_match(filename, std::regex("(.*)(jdk)(.*)")))
					{
						jvm_path = jvm_path + SLASH() + filename + SLASH();
					}
					cont = FindNextFile(hFind, &FindFileData);
				}
				jvm_path = jvm_path + "jre" + SLASH() + "bin" + SLASH() + "server" + SLASH() + "jvm.dll";
			}
		}
	}

	// The jvm lib is inside the fiji.app. So we actually need only the first path.

	//Loading JVM library and methods.
	m_jvm_dll = LoadLibrary(jvm_path.c_str());

	if (m_jvm_dll == nullptr)
	{
		std::cout << "Error while opening jvm library." << std::endl;
		return false;
	}

	m_createJVM_Ptr = (decltype(&JNI_CreateJavaVM))GetProcAddress(m_jvm_dll, "JNI_CreateJavaVM");
	if (m_createJVM_Ptr == nullptr)
	{
		std::cout << "Error while getting JNI_CreateJavaVM funciton address." << std::endl;
		return false;
	}

	std::cout << "\n";
	std::cout << jvm_path << "JVM\n";
	std::cout << jvm_ij_path << "IJ\n";
	std::cout << jvm_bioformats_path << "BIO\n";
	std::cout << "\n";

	JavaVMOption* options = new JavaVMOption[1];

	//Geting absolute path to class file.
	//wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	//exePath = wxPathOnly(exePath);
	std::string exePath = GetExePath();
	std::string imageJPath = "-Djava.class.path=" + exePath + SLASH() + "Java_Code" + SLASH() + getPathSeparator();
	imageJPath.append(jvm_ij_path + getPathSeparator());
	imageJPath.append(jvm_bioformats_path);

	std::cout << imageJPath << std::endl;

	options[0].optionString = const_cast<char*>(imageJPath.c_str());

	m_VMargs.version = JNI_VERSION_1_6;             // minimum Java version
	m_VMargs.nOptions = 1;                          // number of options
	m_VMargs.options = options;
	m_VMargs.ignoreUnrecognized = false;     // invalid options make the JVM init fail

	try {
		jint rc = m_createJVM_Ptr(&m_pJvm, (void**)&m_pEnv, &m_VMargs);
		delete[] options;
		if (rc != JNI_OK) {
			std::cout << "Error while calling JNI_CreteJavaVM." << std::endl;
			return false;
		}
	}
	catch (...) {
		std::cout << "Excption while creating jvm. Nothing to worry about!";
	}
	std::cout << "JVM created successfully." << std::endl;
	jint ver = m_pEnv->GetVersion();
	std::cout << ((ver >> 16) & 0x0f) << "." << (ver & 0x0f) << std::endl;
	std::cout.flush();
	return true;
}
#else
#endif
/*bool JVMInitializer::create_JVM(std::vector<std::string> args)
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
        name = name + WSLASH() + "Contents" + WSLASH() + "Java";
#endif
		if(FILE *file = fopen((name + SLASH() +"ij.jar").c_str(), "r")) {
            std::cout << "Inside here.";
			m_with_fiji = false;
			jvm_ij_path = name + SLASH() +"ij.jar";
			jvm_bioformats_path = bioformats_path;
			fclose(file);
		}
		else {
			name = ij_path + SLASH() + "jars" + SLASH() + "bio-formats";
			wxDir dir(name);
			if (!dir.IsOpened()) {
				m_with_fiji = false;
				return false;
			}
			else {			
				m_with_fiji = true;
				wxString filename;
				bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
				while (cont) {
					if (filename.Matches("formats-*.jar") || filename.Matches("turbojpeg-*.jar") ||
						filename.Matches("ome-xml*.jar") || filename.Matches("ome-codecs*.jar") ||
						filename.Matches("ome-common*.jar")) {
						if (jvm_bioformats_path == "")
							jvm_bioformats_path = dir.GetNameWithSep() + filename;
						else
							jvm_bioformats_path += getPathSeparator() + dir.GetNameWithSep() + filename;
					}
					cont = dir.GetNext(&filename);
				}

				dir.Open(ij_path + WSLASH() + std::string("jars"));
				cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
				while (cont) {
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
						filename.Matches("xml-apis-ext-*.jar") || filename.Matches("xmpcore-*.jar")){
						if (jvm_bioformats_path == "")
							jvm_bioformats_path = dir.GetNameWithSep() + filename;
						else
							jvm_bioformats_path += getPathSeparator() + dir.GetNameWithSep() + filename;
					}
					else if (filename.Matches("ij-*.jar"))
						jvm_ij_path = dir.GetNameWithSep() + filename;
					cont = dir.GetNext(&filename);
				}

				dir.Open(ij_path + WSLASH() + std::string("plugins"));
				cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
				while (cont)
				{
					if (filename.Matches("bio-formats_plugins-*.jar"))
						jvm_bioformats_path += getPathSeparator() + dir.GetNameWithSep() + filename;
					cont = dir.GetNext(&filename); 
				}
                //Checking for jvm path.
                if(wxIsEmpty(jvm_path)){
#ifdef _DARWIN
                    jvm_path = ij_path + WSLASH() + "java" + WSLASH() + "macosx";
#else
					jvm_path = ij_path + WSLASH() + "java" + WSLASH() + "win64";
#endif
                    dir.Open(jvm_path);
                    cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
                    while (cont)
                    {
                        if (filename.Matches("*jdk*")){
                            jvm_path = dir.GetNameWithSep() + filename + WSLASH();
                        }
                        cont = dir.GetNext(&filename);
                    }
#ifdef _DARWIN
                    jvm_path = jvm_path + "jre" + WSLASH() + "Contents" + WSLASH() + "HOME" + WSLASH() + "lib" + WSLASH() + "server" + WSLASH() + "libjvm.dylib";
#else
					jvm_path = jvm_path + "jre" + WSLASH() + "bin" + WSLASH() + "server" + WSLASH() + "jvm.dll";
#endif
                }
			}
		}
	}
    
    // The jvm lib is inside the fiji.app. So we actually need only the first path.

	//Loading JVM library and methods.
#ifdef _DARWIN
    if (const char *error = dlerror())
        std::cout << "Flushed existing error: " << error << std::endl;
#endif
    
    //jvm_path = "/Users/dev/Downloads/ImageJ/jre/lib/server/libjvm.dylib";
#ifdef _WIN32
	m_jvm_dll = LoadLibraryW(jvm_path.ToStdWstring().c_str());
#else
    m_jvm_dll = dlopen((const char*)jvm_path.mb_str(wxConvUTF8), RTLD_NOW);
#endif
    if (m_jvm_dll == nullptr){
        std::cout << "Error while opening jvm library." << std::endl;
        return false;
    }
    
#ifdef _DARWIN
    if (const char *error = dlerror())
        std::cout << "Err: " << error << std::endl;
#endif

#ifdef _WIN32
	m_createJVM_Ptr = (decltype(&JNI_CreateJavaVM))GetProcAddress(m_jvm_dll, "JNI_CreateJavaVM");
#else
    m_createJVM_Ptr = (CreateJavaVM_t*) dlsym(m_jvm_dll, "JNI_CreateJavaVM");
#endif
    if (m_createJVM_Ptr == nullptr){
        std::cout << "Error while getting JNI_CreateJavaVM funciton address." << std::endl;
        return false;
    }

#ifdef _DARWIN
    if (const char *error = dlerror())
        std::cout << "Err: " << error << std::endl;
#endif

    
    std::cout << "\n";
    std::cout << jvm_path << "JVM\n";
    std::cout << jvm_ij_path << "IJ\n";
    std::cout << jvm_bioformats_path << "BIO\n";
    std::cout << "\n";
	using namespace std;
	JavaVMOption* options = new JavaVMOption[1];
	//Geting absolute path to class file.
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	string imageJPath = "-Djava.class.path=" + exePath + WSLASH() + "Java_Code" + WSLASH() + getPathSeparator();
	imageJPath.append(jvm_ij_path + getPathSeparator());
	imageJPath.append(jvm_bioformats_path);
    
    std::cout << imageJPath << std::endl;
    
	options[0].optionString = const_cast<char*>(imageJPath.c_str());

	m_VMargs.version = JNI_VERSION_1_6;             // minimum Java version
	m_VMargs.nOptions = 1;                          // number of options
	m_VMargs.options = options;
	m_VMargs.ignoreUnrecognized = false;     // invalid options make the JVM init fail

	try {
		jint rc = m_createJVM_Ptr(&m_pJvm, (void**)&m_pEnv, &m_VMargs);
		delete[] options;
		if (rc != JNI_OK) {
			std::cout << "Error while calling JNI_CreteJavaVM." << std::endl;
			return false;
		}
	}
	catch (...) {
		std::cout << "Excption while creating jvm. Nothing to worry about!";
	}
    std::cout << "JVM created successfully." << std::endl;
    jint ver = m_pEnv->GetVersion();
    cout << ((ver>>16)&0x0f) << "."<<(ver&0x0f) << endl;
    std::cout.flush();
	return true;
}*/

void JVMInitializer::destroyJVM() {
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
