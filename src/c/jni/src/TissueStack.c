#include "TissueStack.h"

// global environment
JNIEnv * global_env = NULL;

// signal handling - action
void signal_handler(int sig)
{
	printf("Encountered signal: %i\n", sig);

    char * cause = NULL;

    switch (sig) {
    	case SIGABRT:
    		cause = "ABORT (SIGABRT)";
    		break;
		case SIGHUP:
			cause = "HANGUP (SIGHUP)";
			break;
		case SIGQUIT:
			cause = "QUIT (SIGHUP)";
			break;
		case SIGTERM:
			cause = "TERMINATION (SIGHUP)";
			break;
		case SIGINT:
			cause = "INTERRUPT (SIGINT)";
			break;
    }
	if (cause != NULL) { // anything else can pass through
		checkAndClearJNIExceptions(global_env);
		throwJavaException(global_env, "java/lang/RuntimeException", "RECEIVED FATAL SIGNAL. SHUTTING DOWN");
	}
}

// signal handling - registration
void registerSignalHandlers() {
  struct sigaction	act;

  act.sa_handler = signal_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  int i = 1;
  while (i < 32)
  {
      if (i != SIGSEGV) sigaction(i, &act, NULL);
      i++;
  }
}

// exception handling - clearing
void checkAndClearJNIExceptions(JNIEnv *env) {
	jthrowable exc = (*env)->ExceptionOccurred(env);
	if (exc) {
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
	}
}

// exception handling - throwing
void throwJavaException(JNIEnv *env, const char *name, const char *msg) {
	// log and clear exceptions that existed
	checkAndClearJNIExceptions(env);

	jclass newExcCls = (*env)->FindClass(env, name);
	if (newExcCls != NULL) {
		(*env)->ThrowNew(env, newExcCls, msg);
	}
	(*env)->DeleteLocalRef(env, newExcCls);
}

/* get minc info */
JNIEXPORT jobject JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_getMincInfo(JNIEnv * env, jobject obj, jstring filename) {
	// before anything install native signal handlers
	//global_env = env;
	//registerSignalHandlers();
	if (filename == NULL) {
		return NULL;
	}

	const char * file = (*env)->GetStringUTFChars(env, filename, NULL);
	if (file == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not convert java to c string");
		return NULL;
	}

	int fileDescriptor = init_sock_comm_client("/tmp/tissue_stack_communication");
	if (!fileDescriptor) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		throwJavaException(env, "java/lang/RuntimeException", "Could not establish unix socket to image server!");
		return NULL;
	}

	// load and start minc info plugin
	t_string_buffer * startMincInfoCommand = NULL;
	startMincInfoCommand = appendToBuffer(startMincInfoCommand, "try_start minc_info /usr/local/plugins/TissueStackMincInfo.so ");
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


	// get token count
	int numberOfTokens = countTokens(response->buffer, '|', '\\');
	// tokenize response
	char ** tokens = tokenizeString(response->buffer, '|', '\\');

	// free response buffer
	free(response->buffer);
	free(response);

	// tokens are NULL
	if (tokens == NULL || numberOfTokens == 0) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		throwJavaException(env, "java/lang/RuntimeException", "Failed to tokenize buffer content!");
		return NULL;
	}

	// construct Java Objects for return and populate with response content
	jclass MincInfoClazz = (*env)->FindClass(env, "au/edu/uq/cai/TissueStack/dataobjects/MincInfo");
	if (MincInfoClazz == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not create MincInfo object!");
		return NULL;
	}
	jmethodID constructor = (*env)->GetMethodID(env, MincInfoClazz, "<init>", "(Ljava/lang/String;)V");
	if (constructor == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not create MincInfo object!");
		return NULL;
	}

	// we have 1 token but it is 'NULL'
	if (numberOfTokens == 1 && strcmp(tokens[0], "NULL") == 0) {
		// we received nothing, this is legitimate, we return null
		return NULL;
	}

	// call constructor with file name
	jobject mincInfo = (*env)->NewObject(env, MincInfoClazz, constructor, (*env)->NewStringUTF(env, tokens[0]));
	free(tokens[0]);
	if (mincInfo == NULL) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		free(tokens);
		throwJavaException(env, "java/lang/RuntimeException", "Failed to construct MincInfo object!");
		return NULL;
	}

	if (numberOfTokens < 2) {
		// nothing more to do => free resources and return
		free(tokens);
		(*env)->ReleaseStringUTFChars(env, filename, file);
		return mincInfo;
	}

	// get number of dimensions next
	int numberOfDimensions = atoi(tokens[1]);
	free(tokens[1]);

	if (numberOfDimensions == 0) {
		// nothing more to do => free resources and return
		free(tokens);
		(*env)->ReleaseStringUTFChars(env, filename, file);
		return mincInfo;
	}

	// do we have 1 more token
	if (numberOfTokens < 3) {
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
		if (dim_names == NULL) {
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to construct string array for dimension names!");
			return NULL;
		}

		int x = 0;
		while (dimensionNames[x] != NULL) {
			// copy dimension names
			(*env)->SetObjectArrayElement(env, dim_names, x, (*env)->NewStringUTF(env, dimensionNames[x]));
			free(dimensionNames[x]);
			x++;
		}
		free(dimensionNames);

		// call java setter
		jmethodID setMethod = (*env)->GetMethodID(env, MincInfoClazz, "setDimensions", "([Ljava/lang/String;)V");
		if (setMethod == NULL) {
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to set string array for dimension names!");
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_names);
	}

	// do we have 1 more token
	if (numberOfTokens < 4) {
		// nothing more to do => free resources and return
		free(tokens);
		(*env)->ReleaseStringUTFChars(env, filename, file);
		return mincInfo;
	}

	// get dimension sizes, separated by :
	if (tokens[3] != NULL) {
		char ** dimensionSizes = tokenizeString(tokens[3], ':','\\');
		free(tokens[3]);

		// initialize the int array
		jintArray dim_sizes = (*env)->NewIntArray(env, numberOfDimensions);
		if (dim_sizes == NULL) {
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to construct int array for dimension sizes!");
			return NULL;
		}

		int x = 0;
		int * tmp = malloc(sizeof(tmp) * numberOfDimensions);
		while (dimensionSizes[x] != NULL) {
			tmp[x] = atoi(dimensionSizes[x]);
			// copy dimension sizes
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
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to set int array for dimension sizes!");
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_sizes);
	}

	// do we have 1 more token
	if (numberOfTokens < 5) {
		// nothing more to do => free resources and return
		free(tokens);
		(*env)->ReleaseStringUTFChars(env, filename, file);
		return mincInfo;
	}

	// get steps, separated by :
	if (tokens[4] != NULL) {
		char ** steps = tokenizeString(tokens[4], ':','\\');
		free(tokens[4]);

		// initialize the double array
		jdoubleArray dim_steps = (*env)->NewDoubleArray(env, numberOfDimensions);
		if (dim_steps == NULL) {
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to construct double array for steps!");
			return NULL;
		}

		int x = 0;
		double * tmp = malloc(sizeof(tmp) * numberOfDimensions);
		while (steps[x] != NULL) {
			tmp[x] = atof(steps[x]);
			// copy steps
			free(steps[x]);
			x++;
		}
		// copy array
		(*env)->SetDoubleArrayRegion(env, dim_steps, 0, x, (jdouble *) tmp);

		free(tmp);
		free(steps);

		// call java setter
		jmethodID setMethod = (*env)->GetMethodID(env, MincInfoClazz, "setSteps", "([D)V");
		if (setMethod == NULL) {
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to set double array for steps!");
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_steps);
	}

	// do we have 1 more token
	if (numberOfTokens < 6) {
		// nothing more to do => free resources and return
		free(tokens);
		(*env)->ReleaseStringUTFChars(env, filename, file);
		return mincInfo;
	}

	// get starts, separated by :
	if (tokens[5] != NULL) {
		char ** starts = tokenizeString(tokens[5], ':','\\');
		free(tokens[5]);

		// initialize the double array
		jdoubleArray dim_starts = (*env)->NewDoubleArray(env, numberOfDimensions);
		if (dim_starts == NULL) {
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to construct double array for starts!");
			return NULL;
		}

		int x = 0;
		double * tmp = malloc(sizeof(tmp) * numberOfDimensions);
		while (starts[x] != NULL) {
			tmp[x] = atof(starts[x]);
			// copy steps
			free(starts[x]);
			x++;
		}
		// copy array
		(*env)->SetDoubleArrayRegion(env, dim_starts, 0, x, (jdouble *) tmp);

		free(tmp);
		free(starts);

		// call java setter
		jmethodID setMethod = (*env)->GetMethodID(env, MincInfoClazz, "setStarts", "([D)V");
		if (setMethod == NULL) {
			free(tokens);
			(*env)->ReleaseStringUTFChars(env, filename, file);
			throwJavaException(env, "java/lang/RuntimeException", "Failed to set double array for starts!");
			return NULL;
		}
		(*env)->CallVoidMethod(env, mincInfo, setMethod, dim_starts);
	}

	// free resources
	free(tokens);
	(*env)->ReleaseStringUTFChars(env, filename, file);

	return mincInfo;
}
