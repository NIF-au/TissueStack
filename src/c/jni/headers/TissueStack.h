#ifndef TISSUE_STACK_H
	#define TISSUE_STACK_H

	#include "minc_info.h"
	#include "jni.h"
	#include "client.h"
	#include "utils.h"
	#include "strings.h"
	#include <signal.h>

	// IMPORTANT: use this variable to avoid seg faults that are caused by logging when called from JNI
	extern short i_am_jni;

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
		JNIEXPORT jstring JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_tileMincVolume
		  (JNIEnv *, jobject, jstring, jstring, jintArray, jint, jdouble, jstring, jboolean);

		// convert nifti and mnc to raw
		JNIEXPORT jstring Java_au_edu_uq_cai_TissueStack_jni_TissueStack_convertImageFormatToRaw(
				JNIEnv *, jobject, jstring imageFile, jstring newRawFile, jshort  formatIdentifier);

		// per cent querying
		JNIEXPORT jobject  Java_au_edu_uq_cai_TissueStack_jni_TissueStack_queryTaskProgress(JNIEnv *, jobject, jstring taskID);

	#ifdef __cplusplus
	}
	#endif
#endif
