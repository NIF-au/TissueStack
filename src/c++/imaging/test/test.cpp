#include "imaging.h"

#include <iostream>

int main(int argc, char * args[])
{
  tissuestack::imaging::Image ** threeImages = new tissuestack::imaging::Image * [3];

  threeImages[0] = new tissuestack::imaging::RawImage("/opt/Neuro Datasets/00-normal-model-nonsym.raw");
  threeImages[1] = new tissuestack::imaging::NiftiImage("/data/workspace/nifticlib-2.0.0/Testing/Data/ATestReferenceImageForReadingAndWriting.nii.gz");
  threeImages[2] = new tissuestack::imaging::MincImage("/opt/Neuro Datasets/00-normal-model-nonsym.mnc");

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
