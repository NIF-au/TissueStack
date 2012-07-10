#ifndef TISSUE_STACK_H
	#define TISSUE_STACK_H
	#include "minc_info.h"
	#include "jni.h"
	#include "client.h"
	#include "utils.h"
	#include <signal.h>

	// for native signal handling
	void signal_handler(int sig);
	void registerSignalHandlers();

	// for JNI exception handling
	void checkAndClearJNIExceptions(JNIEnv *env);
	void throwJavaException(JNIEnv *env, const char *name, const char *msg);

	// MINC INFO CALL
	JNIEXPORT jobject JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_getMincInfo(JNIEnv *, jobject, jstring);

#endif
