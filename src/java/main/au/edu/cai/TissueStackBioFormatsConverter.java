package au.edu.cai;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import loci.formats.IFormatReader;
import loci.formats.ImageReader;

public class TissueStackBioFormatsConverter {
	
    public static void main(String[] args) {
    	final File[] files =
    		TissueStackBioFormatsConverter.runChecks(args);
    	if (!TissueStackBioFormatsConverter.convert(files))
    		System.err.println("Conversion failed!!!!");
    	else
    		System.out.println("Conversions completed successfully");
    }
    
    private static final File[] runChecks(String[] args) {
      	// some preliminary checks
    	if (args.length != 2) { // number of args
    		System.err.println("Please provide 2 input arguments, the first being the input file, the second the output file!");
    		System.exit(-1);
    	}
    	
    	final File input = new File(args[0]); 
    	if (!input.exists() || !input.canRead()) { // input file existence and readability
    		System.err.println("Given input file does not exist or is not readible!");
    		System.exit(-1);
    	}

    	String output_file_name = args[1];
    	if (!output_file_name.toLowerCase().endsWith(".raw")) // output file conventions
    		output_file_name += ".raw";
    	
    	final File output = new File(output_file_name); 
    	try {
			if (!output.createNewFile()) { // output file existence & write check
				System.err.println("Output file '" + output_file_name + "' exists already!");
				System.exit(-1);
			}
		} catch (IOException e) {
			System.err.println("Could not create output file!. Check permissions!");
			System.exit(-1);
		}
    	
    	return new File[] {input, output};
    }
    
    private static final boolean convert(final File[] files) {
    	IFormatReader reader = null;
    	BufferedOutputStream writer = null;
    	try {
    		reader = new ImageReader();
   			reader.setId(files[0].getAbsolutePath());
   			writer = new BufferedOutputStream(new FileOutputStream(files[1]));
   			// TODO: implement .raw conversion, read in tiles
   			// perhaps rule out conversion of dicom, mnc and niftii
   			// adjust log level
   			
    	} catch(Exception any) {
			System.err.println(any.getMessage());
			
        	try { // delete output
        		files[1].delete();
        	} catch(Exception anyElse) {
        		// ignored
        	}

    		return false;
    	} finally { // close reader and writer
        	try {
        		if (reader != null)
        			reader.close();
        	} catch(Exception anyElse) {
        		// ignored
        	}
        	try {
        		if (writer != null)
        			writer.close();
        	} catch(Exception anyElse) {
        		// ignored
        	}
    	}
    	return true;
    }
}
