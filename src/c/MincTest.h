#ifndef TISSUE_TEST_H
#define TISSUE_TEST_H

#include <string>

class MincTest {
	private:
	    std::string  filename;
	    int number_of_dimensions;
	    std::string * dimensions;
	    long  * sizes;

	public:
		MincTest() {};
		~MincTest() {
			delete [] dimensions;
			delete [] sizes;
		};

		std::string getFilename() const {
	    	return this->filename;
	    }

	    void setFilename(std::string filename) {
	    	this->filename = filename;
	    }

		std::string * getDimensions() const {
	    	return this->dimensions;
	    }

	    void setDimensions(std::string * dimensions) {
	    	this->dimensions = dimensions;
	    }

		long * getSizes() const {
	    	return this->sizes;
	    }

	    void setSizes(long * sizes) {
	    	this->sizes = sizes;
	    }

	    void setNumberOfDimensions(int number_of_dimensions) {
	    	this->number_of_dimensions = number_of_dimensions;
	    }

	    int getNumberOfDimensions() const {
	    	return this->number_of_dimensions;
	    }
};

#endif
