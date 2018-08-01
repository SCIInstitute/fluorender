#include "JVMInitializer.h"
#include "SettingDlg.h"
#include <wx/stdpaths.h>

JVMInitializer* JVMInitializer::m_pJVMInstance = nullptr;
JavaVM* JVMInitializer::m_pJvm = nullptr;
JNIEnv* JVMInitializer::m_pEnv = nullptr;
JavaVMInitArgs JVMInitializer::m_VMargs;
HMODULE JVMInitializer::m_jvm_dll = nullptr;
decltype(&JNI_CreateJavaVM) JVMInitializer::m_createJVM_Ptr = nullptr;
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

	////Loading JVM library and methods.
	m_jvm_dll = LoadLibraryW(jvm_path.ToStdWstring().c_str());
	if (m_jvm_dll == nullptr)
		return false;
	
	m_createJVM_Ptr = (decltype(&JNI_CreateJavaVM))GetProcAddress(m_jvm_dll, "JNI_CreateJavaVM");
	if (m_createJVM_Ptr == nullptr)
		return false;

	using namespace std;	
	JavaVMOption* options = new JavaVMOption[1];
	//Geting absolute path to class file.
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	string imageJPath = "-Djava.class.path=" + exePath + GETSLASH() + "Java_Code" + GETSLASH() + getPathSeparator();
	imageJPath.append(ij_path + getPathSeparator());
	imageJPath.append(bioformats_path);
	options[0].optionString = const_cast<char*>(imageJPath.c_str());

	m_VMargs.version = JNI_VERSION_1_6;             // minimum Java version
	m_VMargs.nOptions = 1;                          // number of options
	m_VMargs.options = options;
	m_VMargs.ignoreUnrecognized = false;     // invalid options make the JVM init fail

	jint rc = m_createJVM_Ptr(&m_pJvm, (void**)&m_pEnv, &m_VMargs);
	delete[] options;
	if (rc != JNI_OK) {
		return false;
	}
	return true;
}

void JVMInitializer::destroyJVM() {
	if (m_pJvm)
		m_pJvm->DestroyJavaVM();
}
