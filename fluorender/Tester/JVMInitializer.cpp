#include "JVMInitializer.h"

JVMInitializer* JVMInitializer::m_pJVMInstance = nullptr;
JavaVM* JVMInitializer::m_pJvm = nullptr;
JNIEnv* JVMInitializer::m_pEnv = nullptr;
JavaVMInitArgs JVMInitializer::m_VMargs;

JVMInitializer* JVMInitializer::getInstance(SettingDlg* inp_settingDlg){
	return 0;
}

char JVMInitializer::getPathSeparator(){
#ifdef _WIN32
	return ';';
#else
	return ':';
#endif
}

bool JVMInitializer::create_JVM(SettingDlg* inp_settingDlg){
	return true;
}

void JVMInitializer::destroyJVM() {
}
