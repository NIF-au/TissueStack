#ifndef TISSUE_STACK_H
#define TISSUE_STACK_H
#include <jni.h>
class TissueStack
{
	public:
		TissueStack();
		void test(const char * filename);
		~TissueStack();
};

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_au_edu_uq_cai_TissueStack_jni_TissueStack_test
  (JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif

#endif
