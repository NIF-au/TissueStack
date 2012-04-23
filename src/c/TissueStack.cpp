#include <stdio.h>
#include "minc.h"
#include "volume_io.h"
#include "volume_io/vol_io_prototypes.h"
#include "TissueStack.h"


TissueStack::TissueStack() {
// constructor: empty for now
}

TissueStack::~TissueStack() {
// destructor: empty for now
}

void TissueStack::test(const char * filename) {
	   volume_input_struct input_info;
	   Volume   in_volume;
	   char    *axis_order[3] = { MIzspace, MIyspace, MIxspace };

	    start_volume_input(const_cast<char *>(filename), MAX_VAR_DIMS, axis_order, NC_UNSPECIFIED,
	                       TRUE, 0.0, 0.0, TRUE, &in_volume, (minc_input_options *) NULL,
	                       &input_info);

	    printf("Hello Minc File (via shared lib): %s\n", input_info.minc_file->filename);

            for (int i=0;i<input_info.minc_file->n_file_dimensions;i++) {
	      printf("\t%s => %li\n", input_info.minc_file->dim_names[i], input_info.minc_file->sizes_in_file[i]);
            }		
}

int main(int argc, char *argv[]) {
   TissueStack *instance = new TissueStack();
   instance->test("/data/home/harald/data/mouse/00-normal-model-nonsym.mnc");
   delete instance;

   return 1;
}

JNIEXPORT void JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_test(JNIEnv * env, jobject obj, jstring filename) {
   const char * file = env->GetStringUTFChars(filename, 0);
   
   TissueStack *instance = new TissueStack();
   instance->test( file);
   delete instance;

   env->ReleaseStringUTFChars(filename, file);


}
