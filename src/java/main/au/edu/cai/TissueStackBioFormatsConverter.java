package au.edu.cai;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import ome.xml.meta.OMEXMLMetadataRoot;
import ome.xml.model.Image;
import ome.xml.model.Pixels;
import ome.xml.model.TiffData;
import loci.common.DebugTools;
import loci.common.services.ServiceFactory;
import loci.formats.FormatTools;
import loci.formats.IFormatReader;
import loci.formats.ImageReader;
import loci.formats.in.OMETiffReader;
import loci.formats.in.OMEXMLReader;
import loci.formats.ome.OMEXMLMetadata;
import loci.formats.services.OMEXMLService;

public class TissueStackBioFormatsConverter {
	
	public static enum ConversionStrategy {
		SINGLE_IMAGE(1),
		TIME_SERIES(2),
		VOLUME_TO_BE_RECONSTRUCTED(3),
		UNKNOWN(4);

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
				   if (TissueStackBioFormatsConverter.hasBeenTerminated) {
						System.out.println();
						System.err.println("Process terminated: Ctrl-C or kill signal!");
						try { // delete output
							files[1].delete();
						} catch(Exception anyElse) {
							// ignored
						}
				   }
			   }
		  });

		 if (!TissueStackBioFormatsConverter.convert(files))
			 System.err.println("\nConversion failed!!!!");
    	else {
    		System.out.println("\nConversions completed successfully");
    		TissueStackBioFormatsConverter.hasBeenTerminated = false;
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
    	ArrayList<String> filesToBeProcessed =
    		new ArrayList<String>();
    	
    	// if we have a zip on our hands => extract each entry and have a look at its
    	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip") && 
    			!TissueStackBioFormatsConverter.extractZip(files, filesToBeProcessed)) {
    		TissueStackBioFormatsConverter.deleteDirectory(
	    		new File(files[2],files[0].getName()).getAbsoluteFile());

    		return false;
    	} else
    		filesToBeProcessed.add(files[0].getAbsolutePath());

    	// remember the prefix of the first
    	if (filesToBeProcessed.isEmpty()) {
    		System.err.println("Couldn't find an appropriate conversion strategy!");
    		return false;
    	}
    	
		System.out.println("Revisit file list and determine conversion strategy...");
		
		// this looks at the image (series) and tries to find an appropriate conversion method
		ConversionStrategy strategy = 
    		TissueStackBioFormatsConverter.revisitFileListAndDetermineStrategy(filesToBeProcessed);
    	if (strategy == ConversionStrategy.UNKNOWN)
    	{
    		System.err.println("Couldn't find an appropriate conversion strategy!");
    		return false;
    	}
    		
		System.out.println("Chosen Conversion Strategy: " + strategy);
		
    	BufferedOutputStream writer = null;
    	String format = null;
    	int sizeX = 0;
    	int sizeY = 0;
    	int numberOfSlicesToBeProcessed = 0;
    	int numberOfSlicesProcessed = 0;
    	int numberOfTimePointsToBeProcessed = 0;
    	int numberOfTimePointsProcessed = 0;
    	
    	for (String fileToBeProcessed : filesToBeProcessed) {
	    	IFormatReader reader = new ImageReader();
	    	try {
	    		try {
	    			reader.setId(fileToBeProcessed);
	    		} catch(Exception any) {
	   				System.out.println("");	    			
	    			System.err.println("Error (BioFormatsReader): " + any.getMessage() +"! Skipping file: " +  fileToBeProcessed);
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
	   					} catch(IllegalArgumentException ill) {
	   						throw new RuntimeException("Pixel Type is undefined!");
	   					}
	   					System.out.println("Gray Scale Image [" + pixelType + "]");
	   				}
	   				
	   				// read in essential parameters
	   				sizeX = reader.getSizeX();
	   				sizeY = reader.getSizeY();
	   				numberOfSlicesToBeProcessed = reader.getSizeZ();
	   				numberOfTimePointsToBeProcessed = reader.getSizeT();
		   			// let's adjust our work figures to reflect the strategy
	   				if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED) {
	   					if (numberOfSlicesToBeProcessed <= 1)
		   					numberOfSlicesToBeProcessed = filesToBeProcessed.size();
	   					numberOfTimePointsToBeProcessed = 0;
	   				} else if (strategy == ConversionStrategy.TIME_SERIES) {
		   				if (numberOfTimePointsToBeProcessed <= 1)
		   					numberOfTimePointsToBeProcessed = filesToBeProcessed.size();
		   				numberOfSlicesToBeProcessed = 0;
		   			}
	   				
	   				System.out.println("Number of Series: " + reader.getSeriesCount() + " [anything higher than 1 will be ignored!]");
	   				System.out.println("Number of Images: " + reader.getImageCount());
	   				System.out.println("Image Plane Dimension (X times Y): " + sizeX + " x " + sizeY);
	   				System.out.println("Number of Slices (Z): " + numberOfSlicesToBeProcessed);
	   				System.out.println("Number of Points in Time (T): " + numberOfTimePointsToBeProcessed);

	   				// we only ever process 1 series
		   			if (reader.getSeriesCount() > 1)
		   				reader.setSeries(0);
	   				
	   				// open raw file for output
		   			writer = new BufferedOutputStream(new FileOutputStream(files[1]));

		   			// write out header
		   			StringBuffer header_buffer = new StringBuffer(100);
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED) {
			   			header_buffer.append(numberOfSlicesToBeProcessed);
		   				header_buffer.append(":");
		   			} 
	   				header_buffer.append(sizeX); // first dimensions
		   			header_buffer.append(":");
		   			header_buffer.append(sizeY);
		   			if (strategy == ConversionStrategy.TIME_SERIES) {
			   			header_buffer.append(":");		   				
		   				header_buffer.append(numberOfTimePointsToBeProcessed);
		   			}
		   			header_buffer.append("|"); // now coordinates TODO: extract meta-info
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
		   				header_buffer.append("0:");
		   			header_buffer.append("0:0");
		   			header_buffer.append("|");	// now spacing/step size TODO: extract meta-info
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
		   				header_buffer.append("1:");
		   			header_buffer.append("1:1"); 
		   			header_buffer.append("|"); // now coordinate naming 
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
			   			header_buffer.append("y:x:z");
		   			else
		   				header_buffer.append("x:y");
		   			header_buffer.append("|6|"); // finish of by setting origin format 
		   			final String partHeader = header_buffer.toString();
		   			header_buffer = new StringBuffer(20+partHeader.length());
		   			header_buffer.append("@IaMraW@V1|");
		   			header_buffer.append(partHeader.length());
		   			header_buffer.append("|");
		   			header_buffer.append(partHeader);

		   			writer.write(header_buffer.toString().getBytes());
	   			} else if (!format.equals(reader.getFormat())) {
	   				System.out.println("");
	   				System.out.println("Format is different from initially detected one ("
	   						+ format + "). Skipping file: " +  fileToBeProcessed);
	   				continue;
	   			}

	   			// dimension checks: they have to be the same as determined initially!
   				if (reader.getSizeX() != sizeX || reader.getSizeY() != sizeY)
   					throw new RuntimeException(
   						"The dimensions of this image don't match the dimensions of the first image in the series!");

   				// if we have one file in the list only, there is another loop with a minimum of 1 iteration (single 2D)
   				// otherwise we read the file data from each file individually, representing what the conversion strategy
   				// believes it is...
   				int end = 1;
   				if (filesToBeProcessed.size() == 1 && strategy == ConversionStrategy.TIME_SERIES)
   					end = numberOfTimePointsToBeProcessed;
   				else if (filesToBeProcessed.size() == 1 && strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
   					end = numberOfSlicesToBeProcessed;
   				for (int j=0;j<end;j++) {
   					int index = 0; 
   					if (end > 1)
   						index = reader.getIndex(
   						(strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED ? j : 0), // z
   						0, // c TODO: adapt for interleaved
   						(strategy == ConversionStrategy.SINGLE_IMAGE || strategy == ConversionStrategy.TIME_SERIES ? j : 0)); // t

   					byte imageData[] = reader.openBytes(index);
   					if ((reader.getBitsPerPixel() % 8) != 0)
   	   					throw new RuntimeException(
   	   						"The bit depth is not a multiple of 8 bit!");
   					int bitsPerPixelDivisionFactor = reader.getBitsPerPixel() / 8; //converge on byte
   					int image_size = sizeX * sizeY;
   					
   					// here comes the nuisance: cater for all bit depths (incl. sign), color and endianess
   					// for all intents and purposes we can treat 8 bit types and color similarly
   					if (reader.isRGB() || bitsPerPixelDivisionFactor == 1) {
   						// first of all allocate the output 
   						byte tmpData[] = new byte[image_size * 3]; // we converge on an RGB
   						int channel = 0;

   						for (int i=0;i<imageData.length;i++) {
   							byte val = imageData[i];
   	   						if (reader.isRGB() && !reader.isInterleaved())
   	   							tmpData[i] = val;
   	   						else if (reader.isRGB() && reader.isInterleaved()) {
   	   							int i_mod_image_size = 
   	   								(i < image_size) ? i : i % image_size;
   	   							if (i_mod_image_size == 0 && i != 0)
   	   								++channel;
   	   							
   	   							int k = channel * image_size + i_mod_image_size * 3;
   	   							tmpData[k] = imageData[i];
   	   						} else {
   	   							// do we have a lookup ?
   	   							if (!reader.isIndexed()) // plain 8 bit gray
   	   								tmpData[i*3] = tmpData[i*3+1] = tmpData[i*3+2] = imageData[i];
   	   							else {
   	   	   							final byte [][] lookupTable = reader.get8BitLookupTable();
   	   	   							if (lookupTable == null)
   	   	   								break;
   	   	   							tmpData[i*3] = lookupTable[imageData[i]][0];
   	   	   							tmpData[i*3+1] = lookupTable[imageData[i]][1];
   	   	   							tmpData[i*3+2] = lookupTable[imageData[i]][2];
   	   							}
   	   						}
   	   							 
   						}
   						imageData = tmpData; // set image data to final output data 
   					} else if (reader.getPixelType() == FormatTools.INT16 || reader.getPixelType() == FormatTools.UINT16 ||
   							reader.getPixelType() == FormatTools.INT32 || reader.getPixelType() == FormatTools.UINT32) {
   						// converge on the java type that covers 16 as well as 32 bits => long (64b)
   						long tmpData[] = new long[imageData.length / bitsPerPixelDivisionFactor];

   						boolean isSignedType = 
   	   							reader.getPixelType() == FormatTools.INT16 || reader.getPixelType() == FormatTools.INT32; 

   						// min/max for contrast range madness in case people use the sledge-hammer for a incy wincy nail
   						long min = Long.MAX_VALUE;
   	   					long max = Long.MIN_VALUE;

   	   					for (int b=0;b<imageData.length;b+=bitsPerPixelDivisionFactor) {
   	   						long val = 0x0000000000000000;
   	   						for (int s=0;s<bitsPerPixelDivisionFactor;s++) {
   	   							int endianOffset =
   	   								reader.isLittleEndian() ? b+s : (bitsPerPixelDivisionFactor-s)-1;
   	   							long newVal = ((long)imageData[endianOffset]) << (s*8);
   	   							if (!isSignedType)
   	   								newVal &= ( ( (long) (0x00000000000000FF) ) << (s*8) );
   	   							val |= newVal;
   	   						}
   	   						if (val < min)
   	   							min = val;
   	   						if (val > max)
   	   							max = val;
   	   						
   	   						tmpData[b/bitsPerPixelDivisionFactor] = val;
   	   					}
   	   					// allocate new output data 
   	   					imageData = new byte[image_size * 3];
   	   					for (int i=0;i<tmpData.length;i++) {
   	   						if (!reader.isIndexed()) // plain 16 bit signed/unsigned data	
   	   							imageData[i*3] = imageData[i*3 + 1] = imageData[i*3 + 2] = 
   	   								(byte) (((double) tmpData[i] / (max-min)) * 255);
   	   						else { // lookup
   	   							final short [][] lookupTable = reader.get16BitLookupTable();
   	   							if (lookupTable == null)
   	   								break;
   	   							imageData[i*3] = (byte) lookupTable[(int)tmpData[i]][0];
   	   							imageData[i*3+1] = (byte) lookupTable[(int)tmpData[i]][1];
   	   							imageData[i*3+2] = (byte) lookupTable[(int)tmpData[i]][2];
   	   						}
   	   					} 
   					} else if (reader.getPixelType() == FormatTools.FLOAT || reader.getPixelType() == FormatTools.DOUBLE) {
   						// converge java type double
   						double tmpData[] = new double[imageData.length / bitsPerPixelDivisionFactor];

   						// min/max for contrast range madness in case people use the sledge-hammer for a incy wincy nail
   						double min = Double.MAX_VALUE;
   	   					double max = Double.MIN_VALUE;
   	   					
   	   					for (int b=0;b<imageData.length;b+=bitsPerPixelDivisionFactor) {
   	   						double val = 0;
   	   						long tmpVal = 0;
	   	   					for (int s=0;s<bitsPerPixelDivisionFactor;s++) {
	   							int endianOffset =
	   								reader.isLittleEndian() ? b+s : (bitsPerPixelDivisionFactor-s)-1;
	   							tmpVal = ((long)imageData[endianOffset]) << (s*8) & ( ( (long) (0x00000000000000FF) ) << (s*8) );
	   						}
   	   						if (reader.getPixelType() == FormatTools.FLOAT)
   	   							val = (double) Float.intBitsToFloat((int) tmpVal);
   	   						else
   	   							val = Double.longBitsToDouble(tmpVal);
   	   						if (val < min)
   	   							min = val;
   	   						if (val > max)
   	   							max = val;
   	   						
   	   						tmpData[b/bitsPerPixelDivisionFactor] = val;
   	   					}
   	   					// allocate new output data 
   	   					imageData = new byte[image_size * 3];
   	   					for (int i=0;i<tmpData.length;i++) {
   							imageData[i*3] = imageData[i*3 + 1] = imageData[i*3 + 2] = 
   								(byte) (((double) tmpData[i] / (max-min)) * 255);
   	   					} 
   					} else {
   						throw new RuntimeException("Unsupported Pixel Type (only RGB, 8/16/32 grayscale and float/double are allowed!");
   					}
   						
   					writer.write(imageData);
   					
   	   				// increment
   	   				if (numberOfTimePointsToBeProcessed > 0 && strategy == ConversionStrategy.TIME_SERIES) {
   	   					++numberOfTimePointsProcessed;
   	   					numberOfSlicesProcessed = numberOfSlicesToBeProcessed;
   	   	   				System.out.print("Time Point " + numberOfTimePointsProcessed + 
   	    	   					" of " + numberOfTimePointsToBeProcessed + " [" +
   	    	   					fileToBeProcessed + "]\t\t\t\r");
   	   				} else if (numberOfSlicesToBeProcessed > 0 && (
   	   					strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED || strategy == ConversionStrategy.SINGLE_IMAGE)) {
   	   					++numberOfSlicesProcessed;
   	   					numberOfTimePointsProcessed = numberOfTimePointsToBeProcessed;
   	   	   				System.out.print("Slice " + numberOfSlicesProcessed + 
   	    	   					" of " + numberOfSlicesToBeProcessed + " [" +
   	    	   					fileToBeProcessed + "]\t\t\t\r");
   	   				}
   	   				System.out.flush();
   				}
	    	} catch(Exception any) {
	    		System.out.println();
				System.err.println("Error: " + any.getMessage());
				
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

    private static ConversionStrategy revisitFileListAndDetermineStrategy(ArrayList<String> filesToBeProcessed) {
    	ArrayList<String> alteredFileListToBeProcessed = new ArrayList<String>(filesToBeProcessed.size());
    	IFormatReader metaDataFileReaderForSeries = null;
    	
		final ImageReader formatInspectorInstance = new ImageReader();
    	
    	String pathPrefix = new File(filesToBeProcessed.get(0)).getParent();
		try {
			for (String f : filesToBeProcessed) {
				try {
		    		final IFormatReader reader = formatInspectorInstance.getReader(f);
		    		
		    		// deal with special case OME XML and TIFF
		    		if ((reader instanceof OMEXMLReader) ||  
		    				(reader instanceof OMETiffReader))
		    		{
			    		// now get to the meta data ...
						ServiceFactory serviceFactory = new ServiceFactory();
						OMEXMLService omexmlService =
							serviceFactory.getInstance(OMEXMLService.class);
						OMEXMLMetadata meta = omexmlService.createOMEXMLMetadata();
						reader.setMetadataStore(meta);
		    		} 
					reader.setId(f);
					
					if (reader.getImageCount() == 1) // we skip single image files
						continue;
					
					// we remember the meta data file reader 
					metaDataFileReaderForSeries = reader;
					
		    		// deal with special case OME XML and TIFF
		    		if ((reader instanceof OMEXMLReader) || 
		    				(reader instanceof OMETiffReader))
		    		{
						OMEXMLMetadataRoot root = (OMEXMLMetadataRoot) reader.getMetadataStoreRoot();
						int timeOrSliceCounter = 0;
						int imageCounter = 0;
						while (true) {
							try {
								// loop over all images
								final Image i = root.getImage(imageCounter);
								final Pixels p = i.getPixels();
								
								if (p != null) {
									int tiffDataCounter = 0;
									while (true) {
										try {
											// loop over all tiff data
											TiffData td = p.getTiffData(tiffDataCounter);
											if (td != null && td.getUUID() != null &&
												((reader.getSizeZ() > 1 && td.getFirstZ().getValue() == timeOrSliceCounter && td.getFirstT().getValue() == 0) ||
												 (reader.getSizeT() > 1	&& td.getFirstT().getValue() == timeOrSliceCounter && td.getFirstZ().getValue() == 0 ))) {
												timeOrSliceCounter++;
												String fileName = td.getUUID().getFileName();
												if (fileName != null && !fileName.isEmpty()) {
													if (new File(fileName).getParent() == null &&
															pathPrefix != null)
														fileName = new File(pathPrefix, fileName).getAbsolutePath(); 
													alteredFileListToBeProcessed.add(fileName);
												}
													
											}
											tiffDataCounter++;
										} catch (Exception endOfTiffData)
										{
											break;
										}
									}
								}
								imageCounter++;
							} catch (Exception endOfImages)
							{
								break;
							}
						}
						break;
		    		} else // other multi image formats
		    			break;
				} catch(Exception any) {
					any.printStackTrace();
					continue;
				}
			}
			
			// distinguish between 2 cases: do we have a metaDataFileReader => use it for strategy determination
			// othewise => first/only file
			if (metaDataFileReaderForSeries == null) {
				metaDataFileReaderForSeries = new ImageReader();
				try {
					metaDataFileReaderForSeries.setId(filesToBeProcessed.get(0));
				} catch(Exception any) {
					return ConversionStrategy.UNKNOWN;
				}
				
			}

	    	ConversionStrategy strategy = 
	    		TissueStackBioFormatsConverter.determineConversionStrategy(filesToBeProcessed, metaDataFileReaderForSeries);

	    	if (!alteredFileListToBeProcessed.isEmpty()) {
	    		filesToBeProcessed.clear();
	    		filesToBeProcessed.addAll(alteredFileListToBeProcessed);
	    	}
	    	return strategy;
		} finally {
    		try {
    			formatInspectorInstance.close();    			
    		} catch(Exception ignored) {}
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
	
	private static ConversionStrategy determineConversionStrategy(final ArrayList<String> filesToBeProcessed, final IFormatReader reader) {
		if (reader.getSizeZ() > 1)
			return ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED;
		else if (reader.getSizeT() > 1)
			return ConversionStrategy.TIME_SERIES;
		else if (filesToBeProcessed.size() == 1) 
			return ConversionStrategy.SINGLE_IMAGE;
		else
			return ConversionStrategy.TIME_SERIES;
	}
}