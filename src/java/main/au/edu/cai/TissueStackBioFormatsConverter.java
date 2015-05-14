package au.edu.cai;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
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
    	if (args.length < 2) { // number of args
    		System.err.println("Please provide 2 input arguments at a minimum! First: input file; Second: output file; Third [optional]: temporary directory!");
    		System.exit(-1);
    	}

    	// remember the input file
    	String input_file_name = args[0];
    	final File input = new File(input_file_name);

    	// check for the existence of a temporary directory
    	// for extraction of zip data
    	String temp_dir_name = System.getProperty("java.io.tmpdir");
    	if (input_file_name.toLowerCase().endsWith(".zip") && args.length > 2)
    		temp_dir_name = args[2];

    	final File temp = new File(temp_dir_name);
    	if (!temp.exists() || !temp.isDirectory() || !temp.canWrite()) 
    	{
    		System.err.println("The temporary directory '" + temp.getAbsolutePath() + "' does not exist/is not a directory or cannot be written to!");
    		System.exit(-1);
    	}

    	// exclude formats dicom, nifti and minc (based on extension for now)
    	if (TissueStackBioFormatsConverter.checkFileNameForMincNiftiAndDicom(input_file_name) ||
    			(input_file_name.toLowerCase().endsWith(".zip") &&
    			TissueStackBioFormatsConverter.checkZipForMincNiftiAndDicom(input))) {
    		System.err.println("Please use the native Tissue Stack Converter for the following input formats: minc, nifti and dicom!");
    		System.exit(-1);
    	}
    	 
    	// preliminary check: input file accessibility 
    	if (!input.exists() || !input.canRead()) { // input file existence and readability
    		System.err.println("Given input file does not exist or is not readible!");
    		System.exit(-1);
    	}

    	// preliminary check: output file accessibility and naming 
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
    	
    	return new File[] {input, output, temp};
    }
    
    private static final boolean convert(final File[] files) {
    	final ArrayList<String> filesToBeProcessed =
    		new ArrayList<String>();
    	
    	// if we have a zip on our hands => extract each entry and have a look at its
    	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip")) {
    		// first create our temporary extracton directory
    		final File zipContents = new File(files[2],files[0].getName());
    		if (zipContents.exists() && zipContents.isFile()) {
				System.err.println("Temporary Zip Extraction Location exists already: " + zipContents.getAbsolutePath());
	        	try { // delete output
	        		files[1].delete();
	        	} catch(Exception anyElse) {
	        		// ignored
	        	}
				return false;
    		} else if (!zipContents.exists()) {
	        	try {
	    			if (!zipContents.mkdirs()) { // output file existence & write check
	    				System.err.println("Could not create temporary zip extraction directory: " + zipContents.getAbsolutePath());
	    	        	try { // delete output
	    	        		files[1].delete();
	    	        	} catch(Exception anyElse) {
	    	        		// ignored
	    	        	}
	    				return false;
	    			}
	    		} catch (Exception e) {
    				System.err.println("Could not create temporary zip extraction directory: " + zipContents.getAbsolutePath());
    	        	try { // delete output
    	        		files[1].delete();
    	        	} catch(Exception anyElse) {
    	        		// ignored
    	        	}
					return false;
	    		}
    		}    		

    		// now go ahead and extract zip entries
	    	ZipInputStream zip = null;
	    	byte[] buffer = new byte[1024];
	    	
	    	try {
		    	zip = new ZipInputStream(new FileInputStream(files[0]));
		   	    ZipEntry ze = null;
		
		   	    while (true) {
		   	    	ze = zip.getNextEntry();
		    	    if (ze == null) break;
		
		    	    FileOutputStream fos = null;
		    	    try {
		    	        final File unzippedFile = new File(zipContents, ze.getName());
		    	        System.out.println("Unzipping: " +  unzippedFile.getAbsoluteFile() + "...");
		    	        new File(unzippedFile.getParent()).mkdirs();
		    	        
		    	        // read zip entry and write out into new file
	    	            fos = new FileOutputStream(unzippedFile);             
	    	            int len = 0;
	    	            while ((len = zip.read(buffer)) > 0)
	    	            	fos.write(buffer, 0, len);
	    	            
	    	            // we got this far so we store the extracted file location for processing
	    	            filesToBeProcessed.add(unzippedFile.getAbsolutePath());
	    	      	} catch(Exception any) {
	    				System.err.println(any.getMessage());
	    				
	    	        	try { // delete output
	    	        		files[1].delete();
	    	        	} catch(Exception anyElse) {
	    	        		// ignored
	    	        	}

	    	        	try {
	    	        		if (zip != null)
	    	        			zip.close();
	    	        	} catch(Exception anyElse) {
	    	        		// ignored
	    	        	}

	    	    		return false;
	    	    	} finally { // close writer
	    	        	try {
	    	        		if (fos != null)
	    	        			fos.close();
	    	        	} catch(Exception anyElse) {
	    	        		// ignored
	    	        	}
	    	    	}
		   	    }
	    	} catch (Exception any) {
	    		return false;
	    	} finally {
	    		try {
	    			if (zip != null)
	    				zip.close();
	    		} catch (Exception ignored) {}
	    	}
    	} else
    		filesToBeProcessed.add(files[0].getAbsolutePath());

    	// now for the main processing loop:
    	// if there is more than one file (e.g. zip archive contents)
    	// the format is determined by the first supported file encountered.
    	// all other formats are ignored with a warning being issued
    	String format = null;
    	
    	for (String fileToBeProcessed : filesToBeProcessed) {
	    	IFormatReader reader = null;
	    	BufferedOutputStream writer = null;
	    	try {
		        System.out.println("Processing File: " +  fileToBeProcessed + "...");
	    		reader = new ImageReader();
	    		try {
	    			reader.setId(fileToBeProcessed);
	    		} catch(Exception any) {
	   				System.out.println("Warning: Format is not supported by bioformats. Skipping file: " +  fileToBeProcessed);
	   				continue;
	    		}
	
	   			if (format == null) {
	   				format = reader.getFormat();
	   				System.out.println("Format recognized: " +  format);	   				
	   				if (TissueStackBioFormatsConverter.checkFormatStringForMincNiftiAndDicom(format))
	   					throw new RuntimeException(
	   						"Please use the native Tissue Stack Converter for the following input formats: minc, nifti and dicom!");
	   				System.out.println("Format to be processed: " +  format);
	   			} else if (!format.equals(reader.getFormat())) {
	   				System.out.println("Format is different from initially detected one ("
	   						+ format + "). Skipping file: " +  fileToBeProcessed);
	   				continue;
	   			}
	   			
	   			// now convert each file
	   			writer = new BufferedOutputStream(new FileOutputStream(files[1]));
	   			// TODO: implement .raw conversion, read in tiles

	    	} catch(Exception any) {
				System.err.println(any.getMessage());
				
	        	try { // delete output
	        		files[1].delete();
	        	} catch(Exception anyElse) {
	        		// ignored
	        	}
	
	        	// erased temporay zip files if created
	        	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip")) {
		    		TissueStackBioFormatsConverter.deleteDirectory(
			    		new File(files[2],files[0].getName()).getAbsoluteFile());
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
    	}
    	
    	// erased temporay zip files if created
    	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip")) {
    		TissueStackBioFormatsConverter.deleteDirectory(
	    		new File(files[2],files[0].getName()).getAbsoluteFile());
    	}

    	if (format == null) {
    		System.err.println("It seems we weren't given any supported files!");
    		files[1].delete();
    		return false;
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
    
    private static boolean checkFormatStringForMincNiftiAndDicom(String format) {
    	format = format.toLowerCase();
    	if (format.indexOf("minc") >= 0 ||
    		format.indexOf("nifti") >= 0 ||
    		format.indexOf("dicom") >= 0)
    		return true;
    	
    	return false;
    }
    
    static public void deleteDirectory(File path) {
    	try { 
            if( path.exists() ) {
                File[] files = path.listFiles();
                for(int i=0; i<files.length; i++) {
                   if(files[i].isDirectory()) {
                     TissueStackBioFormatsConverter.deleteDirectory(files[i]);
                   }
                   else {
                     files[i].delete();
                   }
                }
              }
              path.delete();
    	} catch(Exception anyElse) {
    		System.err.println("Warning: Unable to delete: " + path);
    	}
    }
}
