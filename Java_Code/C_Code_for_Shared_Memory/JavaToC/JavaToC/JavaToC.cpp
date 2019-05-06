// Save as "HelloJNI.c"
#include <jni.h>        // JNI header provided by JDK
#include <stdio.h>      // C Standard IO Header
#include <windows.h>    // TODO: Later on change this to support Linux as well.
#include "JavaToC.h"   // Generated

// Implementation of the native method allocateMemory.
JNIEXPORT jobject JNICALL Java_JavaToC_allocateMemory (JNIEnv* env, jobject thisObj, jint capacity){
	void* myBuffer;
	int bufferLength;

	bufferLength = capacity; // assuming your buffer is 1024 bytes big
	HANDLE mem = OpenFileMapping(FILE_MAP_ALL_ACCESS, // assuming you only want to read
		false, "MyBuffer"); // assuming your file mapping is called "MyBuffer"
	myBuffer = MapViewOfFile(mem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	// don't forget to do UnmapViewOfFile when you're finished
	// put it into a ByteBuffer so the java code can use it
	printf("C++: Allocated Memory!\n");
	return env->NewDirectByteBuffer(myBuffer, bufferLength);
}

JNIEXPORT void JNICALL Java_JavaToC_writeToMemory(JNIEnv* env, jobject thisObj, jobject buffer) {
	jbyte *buff = (jbyte *)env->GetDirectBufferAddress(buffer);
	if (buff == NULL)
		printf("C++: Cannot get address from ByteBuffer argument");
	buff[0] = 100;
}