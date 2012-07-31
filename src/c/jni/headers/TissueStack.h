#ifndef TISSUE_STACK_H
	#define TISSUE_STACK_H
	#include "minc_info.h"
	#include "jni.h"
	#include "client.h"
	#include "utils.h"
	#include <signal.h>

	#ifdef __cplusplus
	extern "C" {
	#endif

		// for native signal handling
		void signal_handler(int sig);
		void registerSignalHandlers();
		// for JNI exception handling
		void checkAndClearJNIExceptions(JNIEnv *env);
		void throwJavaException(JNIEnv *env, const char *name, const char *msg);

		/*
		 * MINC INFO CALL
		 * Class:     au_edu_uq_cai_TissueStack_jni_TissueStack
		 * Method:    getMincInfo
		 * Signature: (Ljava/lang/String;)Lau/edu/uq/cai/TissueStack/dataobjects/MincInfo;
		 */
		JNIEXPORT jobject JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_getMincInfo(JNIEnv *, jobject, jstring);

		/*
		 * MINC TILE CALL
		 * Class:     au_edu_uq_cai_TissueStack_jni_TissueStack
		 * Method:    tileMincVolume
		 * Signature: (Ljava/lang/String;Ljava/lang/String;[IDZ)V
		 */
		JNIEXPORT void JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_tileMincVolume
		  (JNIEnv *, jobject, jstring, jstring, jintArray, jint, jdouble, jboolean);
	#ifdef __cplusplus
	}
	#endif
#endif
