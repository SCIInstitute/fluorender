#include "../compatibility.h"
#include "JVMInitializer.h"
#include <wx/stdpaths.h>

JVMInitializer* JVMInitializer::m_pJVMInstance = nullptr;
JavaVM* JVMInitializer::m_pJvm = nullptr;
JNIEnv* JVMInitializer::m_pEnv = nullptr;
JavaVMInitArgs JVMInitializer::m_VMargs;

JVMInitializer* JVMInitializer::getInstance(){
	if (m_pJVMInstance == nullptr){
		m_pJVMInstance = new JVMInitializer();
		create_JVM();
	}
	return m_pJVMInstance;
}

void JVMInitializer::create_JVM(){
	using namespace std;	
	JavaVMOption* options = new JavaVMOption[1];
	//options[0].optionString = "-Djava.class.path=D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\;D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\ij.jar;D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\SlideBook6Reader.jar;D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\bioformats_package.jar";   // where to find java .class	
	//Geting absolute path to class file.
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	string imageJPath = "-Djava.class.path=" + exePath + GETSLASH() + "Java_Code" + GETSLASH() + ";";
	imageJPath.append(exePath + GETSLASH() + "Java_Code" + GETSLASH() + "ij.jar;");
	imageJPath.append(exePath + GETSLASH() + "Java_Code" + GETSLASH() + "SlideBook6Reader.jar;");
	imageJPath.append(exePath + GETSLASH() + "Java_Code" + GETSLASH() + "bioformats_package.jar;");

	char * writable = new char[imageJPath.size() + 1];
	std::copy(imageJPath.begin(), imageJPath.end(), writable);
	writable[imageJPath.size()] = '\0';

	options[0].optionString = writable;	

	m_VMargs.version = JNI_VERSION_1_6;             // minimum Java version
	m_VMargs.nOptions = 1;                          // number of options
	m_VMargs.options = options;
	m_VMargs.ignoreUnrecognized = false;     // invalid options make the JVM init fail

	jint rc = JNI_CreateJavaVM(&m_pJvm, (void**)&m_pEnv, &m_VMargs);  // YES !!	
	delete options;    // we then no longer need the initialisation options. 
	if (rc != JNI_OK) {
		//TODO: error processing... 		
		//exit(EXIT_FAILURE);
	}	
}

void JVMInitializer::destroyJVM() {
	m_pJvm->DestroyJavaVM();
}
