#ifndef TISSUE_TEST_H
#define TISSUE_TEST_H

#include <string>

class MincTest
{
	private:
	    std::string  _filename;
	    int _number_of_dimensions;
	    std::string * _dimensions;
	    long  * _sizes;

	public:
		MincTest() {};
		~MincTest() {
			delete _dimensions;
			delete _sizes;
		};

		std::string getFilename() {
	    	return _filename;
	    }

	    void setFilename(std::string filename) {
	    	_filename = filename;
	    }

		std::string * getDimensions() {
	    	return _dimensions;
	    }

	    void setDimensions(std::string * dimensions) {
	    	_dimensions = dimensions;
	    }

		long * getSizes() {
	    	return _sizes;
	    }

	    void setSizes(long * sizes) {
	    	_sizes = sizes;
	    }

	    void setNumberOfDimensions(int number_of_dimensions) {
	    	_number_of_dimensions = number_of_dimensions;
	    }

	    int getNumberOfDimensions() {
	    	return _number_of_dimensions;
	    }
};

#endif
