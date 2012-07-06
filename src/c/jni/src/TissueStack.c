#include "TissueStack.h"

JNIEXPORT jobject JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_getMincInfo(JNIEnv * env, jobject obj, jstring filename) {
	if (filename == NULL) {
		return NULL;
	}

	const char * file = (*env)->GetStringUTFChars(env, filename, NULL);

	int fileDescriptor = init_sock_comm_client("/tmp/tissue_stack_communication");
	if (!fileDescriptor) {
		// TODO: throw java error signalling something went wrong opening the socket
		return NULL;
	}

	// load minc info plugin
	char * loadPluginCommand = "load minc_info /usr/local/plugins/TissueStackMincInfo.so";
	write(fileDescriptor, loadPluginCommand, strlen(loadPluginCommand));
	sleep(2);

	// fire minc_info request with given file name
	t_string_buffer * startMincInfoCommand = NULL;
	startMincInfoCommand = appendToBuffer(startMincInfoCommand, "start minc_info ");
	startMincInfoCommand = appendToBuffer(startMincInfoCommand, (char *) file);
	write(fileDescriptor, startMincInfoCommand->buffer, startMincInfoCommand->size);
	free(startMincInfoCommand->buffer);
	free(startMincInfoCommand);

	// read response into dynamic response buffer
	t_string_buffer * response = NULL;

	// a smaller buffer for read iterations
	char * buffer = malloc(sizeof(buffer) * 1024);
	size_t size = 0;

	while ((size = read(fileDescriptor, buffer, 1024)) > 0) {
		if (buffer[size] != '\0') {
			buffer[size] = '\0';
		}
		// append to response
		response = appendToBuffer(response, buffer);
	}
	shutdown(fileDescriptor, 2);
	// free temp buffer
	free(buffer);

	// tokenize response
	char ** tokens = tokenizeString(response->buffer, '|', '\\');

	// free response buffer
	free(response->buffer);
	free(response);

	// construct Java Objects for return and populate with response content
	jclass MincInfoClazz = (*env)->FindClass(env, "au/edu/uq/cai/TissueStack/dataobjects/MincInfo");
	if (MincInfoClazz == NULL) {
		return NULL;
	}
	jmethodID constructor = (*env)->GetMethodID(env, MincInfoClazz, "<init>", "(Ljava/lang/String;)V");
	if (constructor == NULL) {
		return NULL;
	}

	if (tokens == NULL) {
		// TODO: throw error
		return NULL;
	}

	if (tokens[0] == NULL) {
		// we received nothing, this is legitimate, we return null
		return NULL;
	}

	// set filename
	jobject mincInfo = (*env)->NewObject(env, MincInfoClazz, constructor, (*env)->NewStringUTF(env, tokens[0]));
	free(tokens[0]);

	// get number of dimensions next
	int numberOfDimensions = atoi(tokens[1]);
	free(tokens[1]);

	if (numberOfDimensions == 0) {
		// nothing more to do => free resources and return
		free(tokens);
		(*env)->ReleaseStringUTFChars(env, filename, file);
		return mincInfo;
	}

	// get dimension names, separated by :
	if (tokens[2] != NULL) {
		char ** dimensionNames = tokenizeString(tokens[2], ':','\\');
		free(tokens[2]);

		// initialize the String array
		jobjectArray dim_names = (*env)->NewObjectArray(env, numberOfDimensions,
				(*env)->FindClass(env, "java/lang/String"), (*env)->NewStringUTF(env, ""));

		int x = 0;
		while (dimensionNames[x] != NULL) {
			// copy dimension names
			(*env)->SetObjectArrayElement(env, dim_names, x, (*env)->NewStringUTF(env, dimensionNames[x]));
			free(dimensionNames[x]);
			x++;
		}
		free(dimensionNames);

		// call java setter
		jmethodID setMethod = (*env)->GetMethodID(env, MincInfoClazz, "setDimensions",
				"([Ljava/lang/String;)V");
		if (setMethod == NULL) {
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_names);

	}

	// get dimension sizes, separated by :
	if (tokens[3] != NULL) {
		char ** dimensionSizes = tokenizeString(tokens[3], ':','\\');
		free(tokens[3]);

		// initialize the int array
		jintArray dim_sizes = (*env)->NewIntArray(env, numberOfDimensions);

		int x = 0;
		int * tmp = malloc(sizeof(tmp) * numberOfDimensions);
		while (dimensionSizes[x] != NULL) {
			tmp[x] = atoi(dimensionSizes[x]);
			// copy dimension names
			free(dimensionSizes[x]);
			x++;
		}
		// copy array
		(*env)->SetIntArrayRegion(env, dim_sizes, 0, x, (jint *) tmp);

		free(tmp);
		free(dimensionSizes);

		// call java setter
		jmethodID setMethod = (*env)->GetMethodID(env, MincInfoClazz, "setSizes", "([I)V");
		if (setMethod == NULL) {
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_sizes);
	}

	// free resources
	free(tokens);
	(*env)->ReleaseStringUTFChars(env, filename, file);

	return mincInfo;
}
