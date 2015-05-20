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
import loci.formats.FormatTools;
import loci.formats.IFormatReader;
import loci.formats.ImageReader;

public class TissueStackBioFormatsConverter {
	
	public static enum ConversionStrategy {
		SINGLE_IMAGE(1),
		TIME_SERIES(2),
		VOLUME_TO_BE_RECONSTRUCTED(3);

		int i;

	    ConversionStrategy(int i) {
	        this.i = i;
	    }
	    
	    public int getConversionStrategy() {
	        return this.i;
	    }
	}
	
	static boolean hasBeenTerminated = true;
	
    public static void main(String[] args) {
    	DebugTools.enableLogging("ERROR");
    	
    	final File[] files =
    		TissueStackBioFormatsConverter.runChecks(args);
    	
		 Runtime.getRuntime().addShutdownHook(new Thread() {
			   public void run() {
			    if (hasBeenTerminated)
			    	try { // delete output
						files[1].delete();
					} catch(Exception anyElse) {
						// ignored
					}
			   }
		  });

		 if (!TissueStackBioFormatsConverter.convert(files))
			 System.err.println("Conversion failed!!!!");
    	else {
    		System.out.println("Conversions completed successfully");
    		hasBeenTerminated = false;
    	}
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
    	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip") && 
    			!TissueStackBioFormatsConverter.extractZip(files, filesToBeProcessed)) {
    		TissueStackBioFormatsConverter.deleteDirectory(
	    		new File(files[2],files[0].getName()).getAbsoluteFile());

    		return false;
    	} else
    		filesToBeProcessed.add(files[0].getAbsolutePath());

    	// now for the main processing loop:
    	// if there is more than one file (e.g. zip archive contents)
    	// the format is determined by the first supported file encountered.
    	// all other formats are ignored with a warning being issued
    	BufferedOutputStream writer = null;
    	String format = null;
    	ConversionStrategy strategy = null;
    	int sizeX = 0;
    	int sizeY = 0;
    	int numberOfSlicesToBeProcessed = 0;
    	int numberOfSlicesProcessed = 0;
    	int numberOfTimePointsToBeProcessed = 0;
    	int numberOfTimePointsProcessed = 0;
    	
    	for (String fileToBeProcessed : filesToBeProcessed) {
	    	IFormatReader reader = null;
	    	try {
		        System.out.println("Processing File: " +  fileToBeProcessed + "...");
	    		reader = new ImageReader();
	    		try {
	    			reader.setId(fileToBeProcessed);
	    		} catch(Exception any) {
	    			System.err.println("Error (BioFormatsReader): " + any.getMessage() +"! Skipping file: " +  fileToBeProcessed);
	    			any.printStackTrace();
	   				continue;
	    		}
	
	   			if (format == null) {
	   				format = reader.getFormat();
	   				System.out.println("Format recognized: " +  format);	   				
	   				if (TissueStackBioFormatsConverter.checkFormatStringForMincNiftiAndDicom(format))
	   					throw new RuntimeException(
	   						"Please use the native Tissue Stack Converter for the following input formats: minc, nifti and dicom!");
	   				// some more preliminary checks ...
	   				System.out.println("Endian: " +  (reader.isLittleEndian() ? "little" : "big"));
	   				if (reader.isRGB())
	   					System.out.println("Color Image [" +  (!reader.isInterleaved() ? "non " : "") + "interleaved]");
	   				else {
	   					String pixelType = "UNKNOWN";
	   					try {
	   						pixelType = FormatTools.getPixelTypeString(reader.getPixelType());
	   					} catch(IllegalArgumentException ill) {}
	   					System.out.println("Gray Scale Image [" + pixelType + "]");
	   				}
	   				
	   				// read in essential parameters
	   				sizeX = reader.getSizeX();
	   				sizeY = reader.getSizeY();
	   				numberOfSlicesToBeProcessed = reader.getSizeZ();
	   				numberOfTimePointsToBeProcessed = reader.getSizeT();
	   				
	   				System.out.println("Number of Series: " + reader.getSeriesCount() + " [anything higher than 1 will be ignored!]");
	   				System.out.println("Number of Images: " + reader.getImageCount());
	   				System.out.println("Image Plane Dimension (X times Y): " + sizeX + " x " + sizeY);
	   				System.out.println("Number of Slices (Z): " + numberOfSlicesToBeProcessed);
	   				System.out.println("Number of Points in Time (T): " + numberOfTimePointsToBeProcessed);
	   				
	   				String[] filesInSeries = reader.getSeriesUsedFiles();
	   				for (String f : filesInSeries)
	   					System.out.println(f);
	   				System.out.println("Number of Points in Time (T): " + numberOfTimePointsToBeProcessed);
		   			// we only ever process 1 series
		   			if (reader.getSeriesCount() > 1)
		   				reader.setSeries(0);
	   				
		   			// let's determine the conversion strategy
		   			if (filesToBeProcessed.size() == 1 && numberOfSlicesToBeProcessed <= 1 && numberOfTimePointsToBeProcessed <= 1)
		   				strategy = ConversionStrategy.SINGLE_IMAGE;
		   			else if (filesToBeProcessed.size() == 1 && numberOfSlicesToBeProcessed > 1) {
			   			strategy = ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED;
			   			numberOfTimePointsToBeProcessed = 0;	
		   			}else if (filesToBeProcessed.size() == 1 && numberOfTimePointsToBeProcessed > 1) {
			   			strategy = ConversionStrategy.TIME_SERIES;
			   			numberOfSlicesToBeProcessed = 0;	
		   			} else if (filesToBeProcessed.size() > 1 && numberOfSlicesToBeProcessed <= 1 && numberOfTimePointsToBeProcessed <= 1) {
		   				strategy = ConversionStrategy.TIME_SERIES;
		   				numberOfSlicesToBeProcessed = 0;
		   				numberOfTimePointsToBeProcessed = filesToBeProcessed.size();
		   			} else
	   					throw new RuntimeException(
		   					"This configuration of multiple files with multiple slices/time points each is not currently supported!");
		   			System.out.println("Conversion Strategy: " + strategy);	
		   			
	   				// open raw file for output
		   			writer = new BufferedOutputStream(new FileOutputStream(files[1]));
		   			
		   			// write out header
		   			StringBuffer header_buffer = new StringBuffer(100);
		   			header_buffer.append(sizeX); // first dimensions
		   			header_buffer.append(":");
		   			header_buffer.append(sizeY);
		   			if (strategy != ConversionStrategy.SINGLE_IMAGE) {
		   				header_buffer.append(":");
			   			if (strategy == ConversionStrategy.TIME_SERIES)
			   				header_buffer.append(numberOfTimePointsToBeProcessed);
			   			else if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
			   				header_buffer.append(numberOfSlicesToBeProcessed);
		   			}
		   			header_buffer.append("|"); // now coordinates TODO: extract meta-info
		   			header_buffer.append("0:0");
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
		   				header_buffer.append(":0");
		   			header_buffer.append("|");	// now spacing/step size TODO: extract meta-info
		   			header_buffer.append("1:1"); 
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
		   				header_buffer.append(":1");
		   			header_buffer.append("|"); // now coordinate naming 
		   			header_buffer.append("x:y");
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
		   				header_buffer.append(":z");
		   			header_buffer.append("|6|"); // finish of by setting origin format 
		   			final String partHeader = header_buffer.toString();
		   			header_buffer = new StringBuffer(20+partHeader.length());
		   			header_buffer.append("@IaMraW@V1|");
		   			header_buffer.append(partHeader.length());
		   			header_buffer.append("|");
		   			header_buffer.append(partHeader);

		   			writer.write(header_buffer.toString().getBytes());
	   			} else if (!format.equals(reader.getFormat())) {
	   				System.out.println("Format is different from initially detected one ("
	   						+ format + "). Skipping file: " +  fileToBeProcessed);
	   				continue;
	   			}
	   			
	   			// dimension checks: they have to be the same as determined initially!
   				if (reader.getSizeX() != sizeX || reader.getSizeY() != sizeY)
   					throw new RuntimeException(
   						"The dimensions of this image don't match the dimensions of the first image in the series!");

   				int end =
   					(strategy == ConversionStrategy.SINGLE_IMAGE || strategy == ConversionStrategy.TIME_SERIES) ? 
   						numberOfTimePointsToBeProcessed : numberOfSlicesToBeProcessed;
   				for (int j=0;j<end;j++) {
   					int index = reader.getIndex(
   						(strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED ? j : 0), // z
   						0, // c TODO: adapt for interleaved
   						(strategy == ConversionStrategy.SINGLE_IMAGE || strategy == ConversionStrategy.TIME_SERIES ? j : 0)); // t

   					// TODO: read and write
   					byte imageData[] = reader.openBytes(index);
   					if ((reader.getBitsPerPixel() % 8) != 0)
   	   					throw new RuntimeException(
   	   						"The bit depth is not a multiple of 8 bit!");
   					
   					int bitsPerPixelDivisionFactor = reader.getBitsPerPixel() / 8; //converge on byte
   					int imageDataAsInt[] = new int[imageData.length / bitsPerPixelDivisionFactor];
   					int min = Integer.MAX_VALUE;
   					int min_b = Byte.MAX_VALUE;
   					int max_b = Byte.MIN_VALUE;
   					int max = Integer.MIN_VALUE;
   					for (int b=0;b<imageData.length;b+=2) {
   						if (imageData[b] < min_b)
   							min_b = imageData[b];
   						if (imageData[b+1] < min_b)
   							min_b = imageData[b+1];
   						if (imageData[b] > max_b)
   							max_b = imageData[b];
   						if (imageData[b+1] > max_b)
   							max_b = imageData[b+1];
   						int val = ((imageData[b+1] << 8) & 0x0000ff00) | (imageData[b] & 0x000000ff);
   						if (val < min)
   							min = val;
   						if (val > max)
   							max = val;
   						
   						imageDataAsInt[b/2] = val;
   					}
   					System.out.println("MIN: " + min + " MAX: " + max + " " + Integer.MAX_VALUE);
   					imageData = new byte[imageDataAsInt.length*3]; 
   					for (int i=0;i<imageDataAsInt.length;i++) {
   						int adjustedVal = (int) (((double) imageDataAsInt[i] / (max-min)) * 255);
   						imageData[i*3] = imageData[i*3 + 1] = imageData[i*3 + 2] = (byte) adjustedVal;
   					}
   					
   					writer.write(imageData);
   					
   	   				// increment
   	   				if (numberOfTimePointsToBeProcessed > 0 &&
   	   					(strategy == ConversionStrategy.SINGLE_IMAGE || strategy == ConversionStrategy.TIME_SERIES)) {
   	   					numberOfTimePointsProcessed++;
   	   					numberOfSlicesProcessed = numberOfSlicesToBeProcessed;
   	   				}
   	   				if (numberOfSlicesToBeProcessed > 0 && strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED) {
   	   					numberOfSlicesProcessed++;
   	   					numberOfTimePointsProcessed = numberOfTimePointsToBeProcessed;
   	   				}
   	   				
   	   				System.out.print("Slice:\t " + j + " of " + end + "\r");
   				}
	    	} catch(Exception any) {
				System.err.println(any.getMessage());
				
	        	try {
	        		if (writer != null)
	        			writer.close();
	        	} catch(Exception anyElse) {
	        		// ignored
	        	}
				
	        	// erased temporay zip files if created
	        	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip")) {
		    		TissueStackBioFormatsConverter.deleteDirectory(
			    		new File(files[2],files[0].getName()).getAbsoluteFile());
	        	}
	        	
	    		return false;
	    	} finally { // close reader
	        	try {
	        		if (reader != null)
	        			reader.close();
	        	} catch(Exception anyElse) {
	        		// ignored
	        	}
	    	}
    	}

    	// spit out some warnings in the case where the number of slices/time points processed
    	// does not match the total
    	if (numberOfSlicesProcessed != numberOfSlicesToBeProcessed || 
    		numberOfTimePointsProcessed != numberOfTimePointsToBeProcessed) {
    		System.out.println("WARNING: The number of processed slices/time points [" +
    			numberOfSlicesProcessed + "/" + numberOfTimePointsProcessed +
    			"] did not match its supposed total [" +
    			numberOfSlicesToBeProcessed + "/" + numberOfTimePointsToBeProcessed +
    			"] !!!!");
    	}
    	
    	// close raw file
    	try {
    		if (writer != null)
    			writer.close();
    	} catch(Exception anyElse) {
    		// ignored
    	}

    	// erased temporay zip files if created
    	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip")) {
    		TissueStackBioFormatsConverter.deleteDirectory(
	    		new File(files[2],files[0].getName()).getAbsoluteFile());
    	}

    	if (format == null) {
    		System.err.println("It seems we weren't given any supported files!");
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
    
    static private void deleteDirectory(File path) {
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

	private static boolean extractZip(final File[] files, final ArrayList<String> filesToBeProcessed) {
		// first create our temporary extracton directory
		final File zipContents = new File(files[2],files[0].getName());
		if (zipContents.exists() && zipContents.isFile()) {
			System.err.println("Temporary Zip Extraction Location exists already: " + zipContents.getAbsolutePath());
			return false;
		} else if (!zipContents.exists()) {
			try {
				if (!zipContents.mkdirs()) { // output file existence & write check
					System.err.println("Could not create temporary zip extraction directory: " + zipContents.getAbsolutePath());
					return false;
				}
			} catch (Exception e) {
				System.err.println("Could not create temporary zip extraction directory: " + zipContents.getAbsolutePath());
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
			
			return true;
		} catch (Exception any) {
			return false;
		} finally {
			try {
				if (zip != null)
					zip.close();
			} catch (Exception ignored) {}
		}
	}
}