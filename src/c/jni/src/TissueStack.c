#include "TissueStack.h"

int main(int argc, char **argv) {
	init(argv[1]);

	volume_input_struct *volume = start(NULL);
	if (volume == NULL) {
		return -1;
	}

	printf("%s\n", volume->minc_file->dim_names[0]);

	unload(NULL);

	return 0;
}

JNIEXPORT jobject JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_getMincInfo(
		JNIEnv * env, jobject obj, jstring filename) {
	if (filename == NULL) {
		return NULL;
	}

	const char * file = (*env)->GetStringUTFChars(env, filename, NULL);

	// read the file
	init((char *)file);

	// request results
	volume_input_struct *volume = start(NULL);
	if (volume == NULL) {
		return NULL;
	}

	// construct an instance of MincInfo to hold return values
	jclass MincInfoClazz = (*env)->FindClass(env,
			"au/edu/uq/cai/TissueStack/dataobjects/MincInfo");
	if (MincInfoClazz == NULL) {
		return NULL;
	}
	jmethodID constructor = (*env)->GetMethodID(env, MincInfoClazz, "<init>",
			"(Ljava/lang/String;)V");
	if (constructor == NULL) {
		return NULL;
	}

	// set filename
	jobject mincInfo = (*env)->NewObject(env, MincInfoClazz, constructor,
			(*env)->NewStringUTF(env, volume->minc_file->filename));

	// set dimension sizes and labels if dims are not 0
	int numberOfDimensions = volume->minc_file->n_file_dimensions;
	if (numberOfDimensions > 0) {
		jobjectArray dim_names = (*env)->NewObjectArray(env, numberOfDimensions,
				(*env)->FindClass(env, "java/lang/String"), (*env)->NewStringUTF(env, ""));

		jlongArray dim_sizes = (*env)->NewLongArray(env, numberOfDimensions);
		int x = 0;
		for (; x < numberOfDimensions; x++) {
			(*env)->SetObjectArrayElement(env, dim_names, x,
					(*env)->NewStringUTF(env, volume->minc_file->dim_names[x]));
		}
		(*env)->SetLongArrayRegion(env, dim_sizes, 0, numberOfDimensions,
				volume->minc_file->sizes_in_file);
		jmethodID setMethod = (*env)->GetMethodID(env, MincInfoClazz, "setDimensions",
				"([Ljava/lang/String;)V");
		if (setMethod == NULL) {
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_names);
		setMethod = (*env)->GetMethodID(env, MincInfoClazz, "setSizes", "([J)V");
		if (setMethod == NULL) {
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_sizes);
	}

	// free resources
	(*env)->ReleaseStringUTFChars(env, filename, file);
	unload(NULL);

	return mincInfo;
}

