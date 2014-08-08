#include "imaging.h"

tissuestack::imaging::UncachedImageExtraction::UncachedImageExtraction()
{

}

void tissuestack::imaging::UncachedImageExtraction::extractImage(
						const tissuestack::imaging::TissueStackRawData * image,
						const tissuestack::networking::TissueStackImageRequest * request,
						const unsigned long long int slice) const
{
	// delegate
	this->extractImage(-1, image, request, slice);
}

void tissuestack::imaging::UncachedImageExtraction::extractImage(
						const int descriptor,
						const tissuestack::imaging::TissueStackRawData * image,
						const tissuestack::networking::TissueStackImageRequest * request,
						const unsigned long long int slice) const
{
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	unsigned long long int actualOffset =
			actualDimension->getOffset() +
			request->getSliceNumber() * actualDimension->getSliceSize() *
			static_cast<unsigned long long int>(
					(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? 1 : 3);

	ExceptionInfo exception;
	Image		*img = NULL;
	ImageInfo	*imgInfo = NULL;

	GetExceptionInfo(&exception);
	imgInfo = CloneImageInfo((ImageInfo *)NULL);
	if (imgInfo == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create ImageInfo!");

	unsigned long long int dataLength =
			request->getSliceNumber() * actualDimension->getSliceSize() *
			static_cast<unsigned long long int>(
					(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? 1 : 3);

	std::unique_ptr<unsigned char[]> data(new unsigned char[dataLength]);
	memset(data.get(), '\0', dataLength);
	pread64(
			const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor(),
			data.get(),
			dataLength,
			actualOffset);

	img = ConstituteImage(
			request->getXCoordinate(), request->getYCoordinate(),
			(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? "I" : "RGB",
			CharPixel,
			data.get(), &exception);
	if (imgInfo == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not constitute Image!");

    strcpy(img->magick, request->getOutputImageFormat().c_str());
    std::string image_format("image/");
    const std::string httpResponseHeader =
    		 tissuestack::utils::Misc::composeHttpResponse(
    				 "200 OK",
    				 image_format + request->getOutputImageFormat(),
    				 ""
    );

    //TODO: write back and fix descriptor issue with dup
    //write(descriptor, httpResponseHeader.c_str(), httpResponseHeader.length());
    //imgInfo->file = fdopen(descriptor, "w+");
    //WriteImage(imgInfo, img);

    if (img) DestroyImage(img);
    if (imgInfo) DestroyImageInfo(imgInfo);


}
