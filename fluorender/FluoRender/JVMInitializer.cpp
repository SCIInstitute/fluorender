#include "JVMInitializer.h"
#include "SettingDlg.h"
#include <wx/stdpaths.h>

JVMInitializer* JVMInitializer::m_pJVMInstance = nullptr;
JavaVM* JVMInitializer::m_pJvm = nullptr;
JNIEnv* JVMInitializer::m_pEnv = nullptr;
JavaVMInitArgs JVMInitializer::m_VMargs;

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


JVMInitializer* JVMInitializer::getInstance(SettingDlg* inp_settingDlg){
	if (m_pJVMInstance == nullptr){
		if(create_JVM(inp_settingDlg) == true)
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

bool JVMInitializer::create_JVM(SettingDlg* inp_settingDlg){
	wxString jvm_path;
	wxString ij_path;
	wxString bioformats_path;
	if (inp_settingDlg)
	{
		jvm_path = inp_settingDlg->getJVMPath();
		ij_path = inp_settingDlg->getIJPath();
		bioformats_path = inp_settingDlg->getBioformatsPath();
	}

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

	using namespace std;	
	JavaVMOption* options = new JavaVMOption[1];
	//Geting absolute path to class file.
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	string imageJPath = "-Djava.class.path=" + exePath + GETSLASH() + "Java_Code" + GETSLASH() + getPathSeparator();
	imageJPath.append(ij_path + getPathSeparator());
	imageJPath.append(bioformats_path);
    
    //imageJPath.append(exePath + GETSLASH() + "Java_Code" + GETSLASH() + "ij.jar" + getPathSeparator());
    //imageJPath.append(exePath + GETSLASH() + "Java_Code" + GETSLASH() + "SlideBook6Reader.jar" + getPathSeparator());
    //imageJPath.append(exePath + GETSLASH() + "Java_Code" + GETSLASH() + "bioformats_package.jar" + getPathSeparator());
    std::cout << imageJPath << std::endl;
    
	options[0].optionString = const_cast<char*>(imageJPath.c_str());

	m_VMargs.version = JNI_VERSION_1_6;             // minimum Java version
	m_VMargs.nOptions = 1;                          // number of options
	m_VMargs.options = options;
	m_VMargs.ignoreUnrecognized = false;     // invalid options make the JVM init fail

	jint rc = m_createJVM_Ptr(&m_pJvm, (void**)&m_pEnv, &m_VMargs);
	delete[] options;
	if (rc != JNI_OK) {
        std::cout << "Error while calling JNI_CreteJavaVM." << std::endl;
		return false;
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
	FreeLibrary(m_jvm_dll);
#else
	dlclose(m_jvm_dll);
#endif
}
