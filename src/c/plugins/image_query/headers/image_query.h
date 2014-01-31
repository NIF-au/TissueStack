#include "core.h"
#include <magick/api.h>


Image * extractSliceData(t_vol * volume, int dim, char * image_data, int width, int height, FILE * socketDescriptor);
void dealWithException(ExceptionInfo *exception, FILE * socketDescriptor, Image * img, ImageInfo * image_info);
