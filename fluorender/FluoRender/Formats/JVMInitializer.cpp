#include "JVMInitializer.h"
#include "SettingDlg.h"
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

bool JVMInitializer::create_JVM(std::vector<std::string> args){
	wxString jvm_path;
	wxString ij_path;
	wxString bioformats_path;
	std::string jvm_ij_path = "";
	std::string jvm_bioformats_path = "";

	if (!args.empty())
	{
		jvm_path = args[0]; //inp_settingDlg->getJVMPath();
		ij_path = args[1]; //inp_settingDlg->getIJPath();
		bioformats_path = args[2]; //inp_settingDlg->getBioformatsPath();
    std::string name = ij_path.ToStdString();
        // For Mac: ij_path is going to be ij.app or fiji.app.
        
#ifdef _WIN32
#else
        name = name + GETSLASH() + "Contents" + GETSLASH() + "Java";
#endif
		if(FILE *file = fopen((name + GETSLASH() +"ij.jar").c_str(), "r")) {
            std::cout << "Inside here.";
			m_with_fiji = false;
			jvm_ij_path = name + GETSLASH() +"ij.jar";
			jvm_bioformats_path = bioformats_path;
			fclose(file);
		}
		else {
			name = ij_path + GETSLASH() + std::string("jars") + GETSLASH() + std::string("bio-formats");
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

				dir.Open(ij_path + GETSLASH() + std::string("jars"));
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

				dir.Open(ij_path + GETSLASH() + std::string("plugins"));
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
                    jvm_path = ij_path + GETSLASH() + "java" + GETSLASH() + "macosx";
#else
					jvm_path = ij_path + GETSLASH() + "java" + GETSLASH() + "win64";
#endif
                    dir.Open(jvm_path);
                    cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
                    while (cont)
                    {
                        if (filename.Matches("*jdk*")){
                            jvm_path = dir.GetNameWithSep() + filename + GETSLASH();
                        }
                        cont = dir.GetNext(&filename);
                    }
#ifdef _DARWIN
                    jvm_path = jvm_path + "jre" + GETSLASH() + "Contents" + GETSLASH() + "HOME" + GETSLASH() + "lib" + GETSLASH() + "server" + GETSLASH() + "libjvm.dylib";
#else
					jvm_path = jvm_path + "jre" + GETSLASH() + "bin" + GETSLASH() + "server" + GETSLASH() + "jvm.dll";
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
	string imageJPath = "-Djava.class.path=" + exePath.ToStdString(); 
  imageJPath.append(GETSLASH() + "Java_Code" + GETSLASH() + getPathSeparator());
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
}

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
