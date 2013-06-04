/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "TissueStack.h"

// global environment
JNIEnv * global_env = NULL;

// signal handling - action
void jni_signal_handler(int sig)
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

  act.sa_handler = jni_signal_handler;
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
	i_am_jni = 1;

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

	int fileDescriptor = init_sock_comm_client(UNIX_SOCKET_PATH);
	if (!fileDescriptor) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		throwJavaException(env, "java/lang/RuntimeException", "Could not establish unix socket to image server!");
		return NULL;
	}

	// load and start minc info plugin
	t_string_buffer * startMincInfoCommand = NULL;
	startMincInfoCommand = appendToBuffer(startMincInfoCommand, "try_start minc_info ");
	startMincInfoCommand = appendToBuffer(startMincInfoCommand, PLUGINS_PATH);
	startMincInfoCommand = appendToBuffer(startMincInfoCommand, "/TissueStackMincInfo.so ");
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
		if (buffer[size] != '\0') { // for EOF scenario
			buffer[size] = '\0';
		}

		// append to response
		response = appendToBuffer(response, buffer);
	}
	// free temp buffer
	free(buffer);

	// close filedescriptor which should be closed aready actually
	shutdown(fileDescriptor, 2);
	close(fileDescriptor);

	if (response == NULL) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		throwJavaException(env, "java/lang/RuntimeException", "0 length response!");
		return NULL;
	}

	// get token count
	int numberOfTokens = countTokens(response->buffer, '|', '\\');
	// tokenize response
	char ** tokens = tokenizeString(response->buffer, '|', '\\');

	// free response buffer
	free_t_string_buffer(response);

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

JNIEXPORT jstring JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_tileMincVolume
		  (JNIEnv * env, jobject obj, jstring filename, jstring base_dir, jintArray arr_dimensions, jint size, jdouble zoom_factor, jstring image_type, jboolean preview) {
	i_am_jni = 1;

	// tedious parameter checking
	if (filename == NULL) { // MINC FILE NAME
		throwJavaException(env, "java/lang/RuntimeException", "File Name is null!");
		return NULL;
	}
	const char * file = (*env)->GetStringUTFChars(env, filename, NULL);
	if (file == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not convert java to c string");
		return NULL;
	}
	if (base_dir == NULL) { // TILING BASE DIRECTORY
		(*env)->ReleaseStringUTFChars(env, filename, file);
		throwJavaException(env, "java/lang/RuntimeException", "Base Directory is null!");
		return NULL;
	}
	const char * dir = (*env)->GetStringUTFChars(env, base_dir, NULL);
	if (dir == NULL) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		throwJavaException(env, "java/lang/RuntimeException", "Could not convert java to c string");
		return NULL;
	}
	if (arr_dimensions == NULL) { // 6 element DIMENSION array
		(*env)->ReleaseStringUTFChars(env, filename, file);
		(*env)->ReleaseStringUTFChars(env, base_dir, dir);
		throwJavaException(env, "java/lang/RuntimeException", "Dimensions Argument is null!");
		return NULL;
	}
	int arraySize = (int) (*env)->GetArrayLength(env, arr_dimensions);
	if (arraySize != 6) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		(*env)->ReleaseStringUTFChars(env, base_dir, dir);
		throwJavaException(env, "java/lang/RuntimeException", "Dimensions Array has to have 6 elements!");
		return NULL;
	}
	if (zoom_factor <= 0) { // ZOOM FACTOR
		(*env)->ReleaseStringUTFChars(env, filename, file);
		(*env)->ReleaseStringUTFChars(env, base_dir, dir);
		throwJavaException(env, "java/lang/RuntimeException", "Zoom Factor has to be greater than 0.0!");
		return NULL;
	}

	if (size <= 0) {
		size = 256;
	}

	int fileDescriptor = init_sock_comm_client(UNIX_SOCKET_PATH);
	if (!fileDescriptor) {
		(*env)->ReleaseStringUTFChars(env, filename, file);
		(*env)->ReleaseStringUTFChars(env, base_dir, dir);
		//(*env)->ReleaseIntArrayElements(env, arr_dimensions, dimensions, 0);
		throwJavaException(env, "java/lang/RuntimeException", "Could not establish unix socket to image server!");
		return NULL;
	}

	const char * imageType = image_type == NULL ? "PNG" : (*env)->GetStringUTFChars(env, image_type, NULL);
	if (imageType == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not convert java to c string");
		return NULL;
	}

	// load and start image extract plugin
	t_string_buffer * startTilingCommand = NULL;
	startTilingCommand = appendToBuffer(startTilingCommand, "start image ");
	startTilingCommand = appendToBuffer(startTilingCommand, (char *) file);

	jint * dimensions = (*env)->GetIntArrayElements(env, arr_dimensions, NULL);
	int x;
	char conversionBuffer[50];
	for (x=0;x<arraySize;x++)
	{
		sprintf(conversionBuffer, " %i", (int)dimensions[x]);
		startTilingCommand = appendToBuffer(startTilingCommand, conversionBuffer);
	}
	if (preview == JNI_TRUE) {
		sprintf(conversionBuffer, " %.4g 6 full %s grey 0 0 10000 ", (double)zoom_factor, imageType);
		startTilingCommand = appendToBuffer(startTilingCommand, conversionBuffer);
	} else {
		sprintf(conversionBuffer, " %.4g 1 tiles %s %i", (double)zoom_factor, imageType, (int) size);
		startTilingCommand = appendToBuffer(startTilingCommand, conversionBuffer);
		startTilingCommand = appendToBuffer(startTilingCommand, " -1 -1 grey 0 0 10000 ");
	}

	startTilingCommand = appendToBuffer(startTilingCommand, "0 0 ");
	startTilingCommand = appendToBuffer(startTilingCommand, (char *) dir);
	startTilingCommand = appendToBuffer(startTilingCommand, " @tasks@");

	write(fileDescriptor, startTilingCommand->buffer, startTilingCommand->size);
	free_t_string_buffer(startTilingCommand);

	char * buff = malloc(17 * sizeof(*buff));
	memset(buff, 0, 17);
	read(fileDescriptor, buff, 16);

	jstring ret = (*env)->NewStringUTF(env, buff);

	// clean up
	(*env)->ReleaseStringUTFChars(env, filename, file);
	(*env)->ReleaseStringUTFChars(env, base_dir, dir);
	if (image_type != NULL) (*env)->ReleaseStringUTFChars(env, image_type, imageType);
	(*env)->ReleaseIntArrayElements(env, arr_dimensions, dimensions, 0);

	return ret;
}

JNIEXPORT jstring Java_au_edu_uq_cai_TissueStack_jni_TissueStack_convertImageFormatToRaw(
		JNIEnv * env, jobject obj, jstring imageFile, jstring newRawFile, jshort  formatIdentifier) {
	i_am_jni = 1;

	// tedious parameter checking
	if (imageFile == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "File name for original image file is null!");
		return NULL;
	}
	const char * in_file = (*env)->GetStringUTFChars(env, imageFile, NULL);
	if (in_file == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not convert java to c string");
		return NULL;
	}
	if (newRawFile == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "File name for raw output file is null!");
		return NULL;
	}
	const char * out_file = (*env)->GetStringUTFChars(env, newRawFile, NULL);
	if (out_file == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not convert java to c string");
		return NULL;
	}
	if (formatIdentifier != MINC_FORMAT && formatIdentifier != NIFTI_FORMAT) {
		throwJavaException(env, "java/lang/RuntimeException", "Image Format Identifier has to have 1 (MINC) or 2 (NIFTI) !");
		return NULL;
	}

	int fileDescriptor = init_sock_comm_client(UNIX_SOCKET_PATH);
	if (!fileDescriptor) {
		(*env)->ReleaseStringUTFChars(env, imageFile, in_file);
		(*env)->ReleaseStringUTFChars(env, newRawFile, out_file);
		throwJavaException(env, "java/lang/RuntimeException", "Could not establish unix socket to image server!");
		return NULL;
	}

	// load and start appropriate conversion plugin
	t_string_buffer * startConversionCommand = NULL;
	startConversionCommand = appendToBuffer(startConversionCommand, "start ");
	startConversionCommand = appendToBuffer(startConversionCommand, formatIdentifier == MINC_FORMAT ? "minc_converter " : "nifti_converter ");
	startConversionCommand = appendToBuffer(startConversionCommand, (char *) in_file);
	startConversionCommand = appendToBuffer(startConversionCommand, " ");
	startConversionCommand = appendToBuffer(startConversionCommand, (char *) out_file);
	startConversionCommand = appendToBuffer(startConversionCommand, " @tasks@");
	// send command to plugin
	write(fileDescriptor, startConversionCommand->buffer, startConversionCommand->size);
	free_t_string_buffer(startConversionCommand);

	char * buff = malloc(17 * sizeof(*buff));
	memset(buff, 0, 17);
	read(fileDescriptor, buff, 16);

	// close filedescriptor which should be closed aready actually
	shutdown(fileDescriptor, 2);
	close(fileDescriptor);

	if (buff == NULL || strcmp(buff, "NULL") == 0 || strcmp(buff, "") == 0) {
		(*env)->ReleaseStringUTFChars(env, imageFile, in_file);
		(*env)->ReleaseStringUTFChars(env, newRawFile, out_file);
		throwJavaException(env, "java/lang/RuntimeException", "0 length response!");
		return NULL;
	}

	// return value
	jstring ret = (*env)->NewStringUTF(env, buff);

	// freeing
	(*env)->ReleaseStringUTFChars(env, imageFile, in_file);
	(*env)->ReleaseStringUTFChars(env, newRawFile, out_file);

	return ret;
}

JNIEXPORT jobject  Java_au_edu_uq_cai_TissueStack_jni_TissueStack_callTaskAction(
		JNIEnv * env, jobject obj, jstring taskID, jshort action) {
	i_am_jni = 1;

	if (taskID == NULL)
	{
		throwJavaException(env, "java/lang/RuntimeException", "Task ID is null!");
		return NULL;
	}

	if (action != TASK_PROGRESS && action != TASK_RESUME && action != TASK_PAUSE && action != TASK_CANCEL) {
		throwJavaException(env, "java/lang/RuntimeException", "Action can only take on values: 0 (=PROGRESS), 1 (=RESUME), 2 (=PAUSE) or 3 (=CANCEL) !");
		return NULL;
	}

	const char * task = (*env)->GetStringUTFChars(env, taskID, NULL);
	if (task == NULL) {
		throwJavaException(env, "java/lang/RuntimeException", "Could not convert java to c string");
		return NULL;
	}

	int fileDescriptor = init_sock_comm_client(UNIX_SOCKET_PATH);
	if (!fileDescriptor) {
		(*env)->ReleaseStringUTFChars(env, taskID, task);
		throwJavaException(env, "java/lang/RuntimeException", "Could not establish unix socket to image server!");
		return NULL;
	}

	// load and start appropriate conversion plugin
	t_string_buffer * startProgressCommand = NULL;

	char * action_buffer = NULL;
	switch ((short)action) {
		case TASK_PROGRESS:
			action_buffer = strdup(" get ");
			break;
		case TASK_RESUME:
			action_buffer = strdup(" resume ");
			break;
		case TASK_PAUSE:
			action_buffer = strdup(" pause ");
			break;
		case TASK_CANCEL:
			action_buffer = strdup(" cancel ");
			break;
	}
	if (action_buffer == NULL) {
		(*env)->ReleaseStringUTFChars(env, taskID, task);
		throwJavaException(env, "java/lang/RuntimeException", "Action could not be identified!");
		return NULL;
	}

	startProgressCommand = appendToBuffer(startProgressCommand, "start progress");
	startProgressCommand = appendToBuffer(startProgressCommand, action_buffer);
	if (action_buffer != NULL) free(action_buffer);
	startProgressCommand = appendToBuffer(startProgressCommand, (char *) task);

	// send command to plugin
	write(fileDescriptor, startProgressCommand->buffer, startProgressCommand->size);
	free_t_string_buffer(startProgressCommand);

	// read response
	char * buffer = malloc(sizeof(buffer) * 1024);
	memset(buffer, 0, 1024);
	read(fileDescriptor, buffer, 1023);

	// close filedescriptor which should be closed aready actually
	shutdown(fileDescriptor, 2);
	close(fileDescriptor);

	if (buffer == NULL || strcmp(buffer, "") == 0) {
		(*env)->ReleaseStringUTFChars(env, taskID, task);
		throwJavaException(env, "java/lang/RuntimeException", "0 length response!");
		return NULL;
	}

	if (strcmp(buffer, "NULL") == 0) return NULL;

	// get token count
	int numberOfTokens = countTokens(buffer, '|', '\\');
	// tokenize response
	char ** tokens = tokenizeString(buffer, '|', '\\');
	// free original response now
	free(buffer);

	if (numberOfTokens < 2) {
		(*env)->ReleaseStringUTFChars(env, taskID, task);
		throwJavaException(env, "java/lang/RuntimeException", "Didn't receive all required data tokens (file name & progress)!");
		return NULL;
	}

	const char * filename = strdup(tokens[0]);
	free(tokens[0]);
	const char * progress = strdup(tokens[1]);
	free(tokens[1]);
	free(tokens);

	// construct Java Objects for return and populate with response content
	jclass taskStatusClazz = (*env)->FindClass(env, "au/edu/uq/cai/TissueStack/dataobjects/TaskStatus");
	if (taskStatusClazz == NULL) {
		(*env)->ReleaseStringUTFChars(env, taskID, task);
		throwJavaException(env, "java/lang/RuntimeException", "Could not find TaskStatus class!");
		return NULL;
	}
	jmethodID constructor = (*env)->GetMethodID(env, taskStatusClazz, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	if (constructor == NULL) {
		(*env)->ReleaseStringUTFChars(env, taskID, task);
		throwJavaException(env, "java/lang/RuntimeException", "Could not create TaskStatus object!");
		return NULL;
	}
	jstring jfilename = (*env)->NewStringUTF(env, filename);
	jstring jprogress = (*env)->NewStringUTF(env, progress);

	jobject ret = (*env)->NewObject(env, taskStatusClazz, constructor, jfilename, jprogress);
	if (ret == NULL) {
		(*env)->ReleaseStringUTFChars(env, jprogress, progress);
		(*env)->ReleaseStringUTFChars(env, jfilename, filename);

		(*env)->ReleaseStringUTFChars(env, taskID, task);
		throwJavaException(env, "java/lang/RuntimeException", "Could not create TaskStatus return object!");
		return NULL;
	}

	// clean up
	(*env)->ReleaseStringUTFChars(env, jprogress, progress);
	(*env)->ReleaseStringUTFChars(env, jfilename, filename);
	(*env)->ReleaseStringUTFChars(env, taskID, task);

	return ret;
}
