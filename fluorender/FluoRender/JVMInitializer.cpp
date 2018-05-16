#include "JVMInitializer.h"

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
	//JavaVM *jvm;                      // Pointer to the JVM (Java Virtual Machine)
	//JNIEnv *env;                      // Pointer to native interface
									  
	//JavaVMInitArgs m_VMargs;                        // Initialization arguments
	JavaVMOption* options = new JavaVMOption[1];   // JVM invocation options
	options[0].optionString = "-Djava.class.path=D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\;D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\ij.jar;D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\SlideBook6Reader.jar;D:\\Dev_Environment\\fluorender\\build\\bin\\Debug\\Java_Code\\bioformats_package.jar";   // where to find java .class
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

	cout << "JVM load succeeded: Version ";
	jint ver = m_pEnv->GetVersion();
	cout << ((ver >> 16) & 0x0f) << "." << (ver & 0x0f) << endl;

	// TO DO: add the code that will use JVM <============  (see next steps)
}

void JVMInitializer::destroyJVM() {
	m_pJvm->DestroyJavaVM();
}
