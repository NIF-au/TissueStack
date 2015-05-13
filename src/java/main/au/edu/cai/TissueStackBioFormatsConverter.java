package au.edu.cai;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import loci.common.DebugTools;
import loci.formats.IFormatReader;
import loci.formats.ImageReader;

public class TissueStackBioFormatsConverter {
	
    public static void main(String[] args) {
    	DebugTools.enableLogging("ERROR");
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
    	
    	String input_file_name = args[0];
    	final File input = new File(input_file_name);
    	
    	if (TissueStackBioFormatsConverter.checkFileNameForMincNiftiAndDicom(input_file_name) ||
    			(input_file_name.toLowerCase().endsWith(".zip") &&
    			TissueStackBioFormatsConverter.checkZipForMincNiftiAndDicom(input))) {
    		System.err.println("Please use the native Tissue Stack Converter for the following input formats: minc, nifti and dicom!");
    		System.exit(-1);
    	}
    	 
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
   			
   			System.out.println(reader.getFormat());
   			writer = new BufferedOutputStream(new FileOutputStream(files[1]));
   			// TODO: implement .raw conversion, read in tiles
   			// if we have a zip, think hard what to do ...
   			
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
    
    private static boolean checkZipForMincNiftiAndDicom(final File zipFile) {
    	ZipInputStream zip = null;
    	try {
	    	zip = new ZipInputStream(new FileInputStream(zipFile));
	   	    ZipEntry ze = null;
	   	    String entryName = null;
	
	   	    while (true) {
	    	      ze = zip.getNextEntry();
	    	      if (ze == null) break;
	
	    	      if (entryName == null) {
	    	        entryName = ze.getName();
	    	        break;
	    	      }
	   	    }
	   	    
	   	    if (entryName == null) return false;
	   	    
	   	    return TissueStackBioFormatsConverter.checkFileNameForMincNiftiAndDicom(entryName);
    	} catch (Exception any) {
    		return false;
    	} finally {
    		try {
    			if (zip != null)
    				zip.close();
    		} catch (Exception ignored) {}
    	}
    }
    
    private static boolean checkFileNameForMincNiftiAndDicom(String fileName) {
    	fileName = fileName.toLowerCase();
    	if (fileName.endsWith(".mnc") ||
    			fileName.endsWith(".nii") ||    			
    			fileName.endsWith(".nii.gz") ||
    			fileName.endsWith(".dcm") ||
    			fileName.endsWith(".ima"))
    		return true;
    	
    	return false;
    }
}
