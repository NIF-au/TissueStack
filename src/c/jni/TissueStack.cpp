#include <stdio.h>
#include <minc2.h>
#include <volume_io.h>
#include <volume_io/vol_io_prototypes.h>
#include "TissueStack.h"

TissueStack::TissueStack() {
// constructor: empty for now
}

TissueStack::~TissueStack() {
// destructor: empty for now
}

MincTest * TissueStack::test(const char * filename) const {
	if (filename == NULL) {
		return NULL;
	}

		FILE * file =fopen(filename, "r");
		if (file == NULL) {
			return NULL;
		} else {
			fclose(file);
		}

	   volume_input_struct input_info;
	   Volume   in_volume;
	   const char * axis_order[3] = {const_cast<const char *>(MIzspace), const_cast<const char *>(MIyspace), const_cast<const char *>(MIxspace) };

	   MincTest * result = NULL;

	    if (start_volume_input(const_cast<char *>(filename), MAX_VAR_DIMS, const_cast<char **>(axis_order), NC_UNSPECIFIED,
	                       TRUE, 0.0, 0.0, TRUE, &in_volume, (minc_input_options *) NULL,
	                       &input_info) != OK) {
	    	return NULL;
	    }

		result = new MincTest();
		result->setFilename(std::string(input_info.minc_file->filename));

		result->setNumberOfDimensions(input_info.minc_file->n_file_dimensions);
		std::string *dim_names = new std::string[result->getNumberOfDimensions()];
		long * sizes = new long[result->getNumberOfDimensions()];

		for (int i=0;i<result->getNumberOfDimensions();i++) {
			dim_names[i] = std::string(input_info.minc_file->dim_names[i]);
			sizes[i] = input_info.minc_file->sizes_in_file[i];
		 }

		result->setDimensions(dim_names);
		result->setSizes(sizes);

	    // clean up after ourselves
	    delete_volume_input( &input_info );
	    delete_volume(in_volume);

        return result;
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		printf("No minc filename handed in");
		return 0;
	}
   TissueStack *instance = new TissueStack();
   MincTest * result = instance->test(argv[1]);
   if (result != NULL) {
	  printf("%li\n%li\n%li\n", result->getSizes()[0], result->getSizes()[1], result->getSizes()[2]);
	  printf("%s\n%s\n%s\n", result->getDimensions()[0].c_str(), result->getDimensions()[1].c_str(), result->getDimensions()[2].c_str());
   } else {
	   printf("Error occured reading minc file");
   }

   delete result;
   delete instance;

    return 1;
}

JNIEXPORT jobject JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_test(JNIEnv * env, jobject obj, jstring filename) {
   if (filename == NULL) {
	   return NULL;
   }

	const char * file = env->GetStringUTFChars(filename, 0);

   TissueStack *instance = new TissueStack();
   MincTest * result = instance->test( file);
   if (result == NULL) {
	   return NULL;
   }

   jclass MincTestClazz = env->FindClass("au/edu/uq/cai/TissueStack/dataobjects/MincTest");
   if (MincTestClazz == NULL) {
	   return NULL;
   }

   // construct an instance of MincTest
   jmethodID constructor = env->GetMethodID(MincTestClazz, "<init>", "(Ljava/lang/String;)V");
      if (constructor == NULL) {
   	   return NULL;
    }
   jobject mincTestReturn = env->NewObject(
		   MincTestClazz,
		   constructor,
		   env->NewStringUTF(result->getFilename().c_str()));

   if (result->getNumberOfDimensions() > 0) {
 	   jobjectArray dim_names =
 			   env->NewObjectArray(result->getNumberOfDimensions(),
 			   env->FindClass("java/lang/String"),
 			   env->NewStringUTF(""));

 	   jlongArray dim_sizes =
 	 			   env->NewLongArray(result->getNumberOfDimensions());

 	   for (int x=0;x<result->getNumberOfDimensions();x++) {
 		 env->SetObjectArrayElement(dim_names,x,env->NewStringUTF((result->getDimensions()[x]).c_str()));
 	   }
	   env->SetLongArrayRegion(dim_sizes, 0, result->getNumberOfDimensions(), result->getSizes());

 	    // get setDimensions() handle
 	   jmethodID setMethod = env->GetMethodID(MincTestClazz, "setDimensions", "([Ljava/lang/String;)V");
 	   if (setMethod == NULL) {
 		   return NULL;
 	   }

 	   env->CallVoidMethod(mincTestReturn, setMethod, dim_names);

 	   // get setSizes() handle
	   setMethod = env->GetMethodID(MincTestClazz, "setSizes", "([J)V");
	   if (setMethod == NULL) {
		   return NULL;
	   }

 	  env->CallVoidMethod(mincTestReturn, setMethod, dim_sizes);
    }

   // free resources
   env->ReleaseStringUTFChars(filename, file);
   delete result;
   delete instance;

   return mincTestReturn;
}
