#include "imaging.h"

#include <iostream>

int main(int argc, char * args[])
{
  tissuestack::imaging::TissueStackImageData ** threeImages = new tissuestack::imaging::TissueStackImageData * [3];

  threeImages[0] =
		  const_cast<tissuestack::imaging::TissueStackImageData *>(
				  tissuestack::imaging::TissueStackImageData::fromFile("/opt/Neuro Datasets/00-normal-model-nonsym.raw"));
  threeImages[1] =
		  const_cast<tissuestack::imaging::TissueStackImageData *>(
				  tissuestack::imaging::TissueStackImageData::fromFile("/data/workspace/nifticlib-2.0.0/Testing/Data/ATestReferenceImageForReadingAndWriting.nii.gz"));
  std::cout << static_cast<tissuestack::imaging::TissueStackNiftiData *>(threeImages[1])->isColor() << std::endl;
  threeImages[2] =
		  const_cast<tissuestack::imaging::TissueStackImageData *>(
				  tissuestack::imaging::TissueStackImageData::fromFile("/opt/Neuro Datasets/00-normal-model-nonsym.mnc"));

  for (int i=0;i<3;i++)
  {
	  if (i == 0 && !threeImages[i]->isRaw())
	  {
		  std::cerr << "First image is not raw!" << std::endl;
		  return 0;
	  } else if (i == 1 && (threeImages[i]->isRaw() || threeImages[i]->getFormat() != tissuestack::imaging::FORMAT::NIFTI))
	  {
		  std::cerr << "Second image is not Nifti!" << std::endl;
		  return 0;
	  } else if (i == 2 && (threeImages[i]->isRaw() || threeImages[i]->getFormat() != tissuestack::imaging::FORMAT::MINC))
	  {
		  std::cerr << "Third image is not Minc!" << std::endl;
		  return 0;
	  }
	  delete threeImages[i];
  }
  delete [] threeImages;


  tissuestack::logging::TissueStackLogger::purgeInstance();

  return 1;
}
