package au.edu.cai;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryMXBean;
import java.lang.management.MemoryUsage;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
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
	
	/*
	 * ENUM DEFINING CONVERSION STRATEGIES
	 */
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
	
	/*
	 * A WORKER THREAD CLASS FOR CONVERSION
	 */
	public static class ConverterThread extends Thread {
		private final List<String> filesToBeProcessed;
		private int start;
		private int end;
		private int offset;
		private File files[];
		
		
		public ConverterThread(
			final List<String> filesToBeProcessed,
			final int start, final int end,
			final int offset,
			final File[] files) {
			this.filesToBeProcessed = filesToBeProcessed;
			this.start = start;
			this.end = end;
			this.files = files;
			this.offset = offset;
		}

		@SuppressWarnings("resource")
		public void run() {
			 RandomAccessFile writer = null;
			 IFormatReader reader = null;
			 int actualStart = this.start;
			 int actualEnd = this.end > this.filesToBeProcessed.size() ? this.filesToBeProcessed.size() : this.end;
			 if (this.filesToBeProcessed.size() == 1) {
				 actualStart = 0;
				 actualEnd = 1;
			 }
			 
			 try {
				 writer = new RandomAccessFile(this.files[1], "rw");
				 for (int j=actualStart;
						j<actualEnd;
						j++) {
					 
					 // check if some other thread caused an error
					 if (TissueStackBioFormatsConverter.errorFlag) break;
					
			    	reader = new ImageReader();
	    			reader.setId(this.filesToBeProcessed.get(j));
					reader.setSeries(TissueStackBioFormatsConverter.series);

					if (j > 0 && TissueStackBioFormatsConverter.metaInfoFileImageCount != reader.getImageCount() &&
							(reader.getImageCount() / reader.getSizeC() > 1))
						throw new RuntimeException("This configuration of a series is not supported (Multiple meta info files)!");
					
					int internalLoopStart = 0;
					int internalLoopEnd = 1;
					if (j == 0 && this.filesToBeProcessed.size() == 1) {
						internalLoopStart = this.start;
						internalLoopEnd = this.end;
						if (TissueStackBioFormatsConverter.strategy == ConversionStrategy.TIME_SERIES &&
								internalLoopEnd > TissueStackBioFormatsConverter.numberOfTimePointsToBeProcessed)
							internalLoopEnd = TissueStackBioFormatsConverter.numberOfTimePointsToBeProcessed;
						else if (TissueStackBioFormatsConverter.strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED &&
								internalLoopEnd > TissueStackBioFormatsConverter.numberOfSlicesToBeProcessed)
							internalLoopEnd = TissueStackBioFormatsConverter.numberOfSlicesToBeProcessed;
					}
					
					int c=0;
					int t=0;
					int z=0;

					for (int l=internalLoopStart;l<internalLoopEnd;l++) {
						if (l > 0 && strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED) {
							c = 0;
							t = 0;
							z = l;
						} else if (l > 0 && strategy == ConversionStrategy.TIME_SERIES) {
							c = 0;
							z = 0;
							t = l;
						}
							
						int index = reader.getIndex(z, c, t);
							
						byte imageData[] = reader.openBytes(index);
						int bitsPerPixelDivisionFactor = reader.getBitsPerPixel() / 8; //converge on byte
	
						int image_size = reader.getSizeX() * reader.getSizeY();
						
						// here comes the nuisance: cater for all bit depths (incl. sign), color and endianess
						// for all intents and purposes we can treat 8 bit types and color similarly
						if (reader.isRGB() || reader.getSizeC() == 3 || bitsPerPixelDivisionFactor == 1) {
							// first of all allocate the output 
							byte tmpData[] = new byte[image_size * 3]; // we converge on an RGB
							int numOfChannels = reader.getSizeC() > 1 && !reader.isInterleaved() ? reader.getSizeC() : 1;
			
							for (c=0;c<numOfChannels;c++) {
								if (c > 0) {
									index = reader.getIndex(z, c, t);
									imageData = reader.openBytes(index);
									
									// we map only to RGB
									if (c>3) continue;
								}
									
								for (int i=0;i<imageData.length;i++) {
									byte val = imageData[i];
									if (reader.isRGB() || (reader.getSizeC() > 1 && reader.isInterleaved()))
										tmpData[i] = val;
									else if (reader.getSizeC() > 1 && !reader.isInterleaved()) {
										int k = i * 3 + c;
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
						}   	
						
						long sliceDataOffset =
							(long) this.offset + (long) image_size * 3 * (internalLoopEnd > 1 ? l : j);
	
						writer.seek(sliceDataOffset);
						writer.write(imageData);
	
						// TODO: correct and optimize this. does not work as is. just a skeleton copy of code
						if (TissueStackBioFormatsConverter.strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED &&
								!TissueStackBioFormatsConverter.avoid3DReconstruction) {
							throw new RuntimeException("3D volume reconstruction is work in progress. Sorry.");
							/*
							long dimensionSize = (long) image_size * filesToBeProcessed.size() * 3;
							long secondDimensionOffset = (long) this.offset + dimensionSize;
							long thirdDimensionOffset = secondDimensionOffset + dimensionSize;
							long secondDimensionImageSize = (long) reader.getSizeX() * filesToBeProcessed.size() * 3;
							long thirdDimensionImageSize = (long) reader.getSizeY() * filesToBeProcessed.size() * 3;
							
							for (int h=0 ; h < reader.getSizeY() ; h++)
							{
								 if (TissueStackBioFormatsConverter.errorFlag) {
									 System.err.println("ERROR SOMEWHERE ELSE");
									 break;
								 }
	
								for (int w=0; w < reader.getSizeX() ; w++) {
									final int dataOffset =
										h * reader.getSizeX() * 3 + w * 3;
									final long xPlaneOffset =
										secondDimensionOffset + w * secondDimensionImageSize +
											h * filesToBeProcessed.size() * 3 + j * 3;
									final long yPlaneOffset =
										thirdDimensionOffset + (reader.getSizeY() - (h + 1)) * thirdDimensionImageSize +
											w * filesToBeProcessed.size() * 3 + j * 3;
	
									byte[] bytesToBeWritten = Arrays.copyOfRange(imageData, dataOffset, dataOffset+3);
									writer.seek(xPlaneOffset);
									writer.write(bytesToBeWritten);
	
									writer.seek(yPlaneOffset);
									writer.write(bytesToBeWritten);
								}
							}
							*/
						}
	
						// increment and display progress
						TissueStackBioFormatsConverter.incrementAndDisplayProgress(reader.getCurrentFile());
					}
				 }
			 } catch (Exception any) {
				 errorFlag = true;
				 System.err.println("");
				 System.err.println("Error Conversion: " + any.getMessage());
				 any.printStackTrace(System.err);
			 } finally {
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
	}
	
	/*
	 *  GLOBAL VARIABLE SECTION
	 */

	// avoid 3D construction
	static boolean avoid3DReconstruction = true;
	// the strategy to be accessed globally
	static ConversionStrategy strategy = null;

	// the workload parameters to be accessed globally
	static int metaInfoFileImageCount = 0;
	static int series = 0;
	static int numberOfSlicesToBeProcessed = 0;
	static int numberOfSlicesProcessed = 0;
	static int numberOfTimePointsToBeProcessed = 0;
	static int numberOfTimePointsProcessed = 0;
	
	// global termination flag as a sign to delete temporary files as well as an incomplete output
	static boolean hasBeenTerminated = true;
	// global error flag as a sign for other threads to stop
	static boolean errorFlag = false;
	
	/*
	 *  MAIN METHOD
	 */
    public static void main(String[] args) {
    	DebugTools.enableLogging("ERROR");
    	
    	final File[] files =
    		TissueStackBioFormatsConverter.runChecks(args);
    	
		 Runtime.getRuntime().addShutdownHook(new Thread() {
			   public void run() {
				   if (TissueStackBioFormatsConverter.hasBeenTerminated || TissueStackBioFormatsConverter.errorFlag) {
						System.out.println();
						if (!TissueStackBioFormatsConverter.errorFlag)
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
    		System.out.println("\nConversion completed successfully");
    		TissueStackBioFormatsConverter.hasBeenTerminated = false;
    	}
    }
    
    /*
     * PRELIMINARY CHECKS BEFORE CONVERSION IS EVEN CONSIDERED
     */
    private static final File[] runChecks(String[] args) {
      	// some preliminary checks
    	if (args.length < 2) { // number of args
    		System.err.println("Please provide 2 input arguments at a minimum:\n"
    				+ "First: input file; Second: output file;\n"
    				+ "Third [optional, default: /tmp]: temporary directory; Fourth [optional, default: true]: avoid 3D reconstruction!");
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

    	if (args.length > 3)
    		TissueStackBioFormatsConverter.avoid3DReconstruction = Boolean.parseBoolean(args[3]);
    	
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
    
    /*
     *  CONVERSION FUNCTION:
     *  -) optionally: unzips archives
     *  -) adds files to processing list
     *  -) determins conversions strategy, 
     *     i.e. does something end up as a single image, time series or 3D reconstruction in the .raw 
     *  -) special treatment for OME-XML and TIFF
     *  -) performs further check on file list to weed out incompatible image dimensions and formats
     *  -) set work load figures
     *  -) created output file of proper size and writes header
     *  -) instantiates actual worker threads and waits for completion
     */
    private static final boolean convert(final File[] files) {
    	ArrayList<String> filesToBeProcessed =
    		new ArrayList<String>();
    	
    	// if we have a zip on our hands => extract each entry and have a look at its
    	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip") && 
    			!TissueStackBioFormatsConverter.extractZip(files, filesToBeProcessed)) {
    		TissueStackBioFormatsConverter.deleteDirectory(
	    		new File(files[2],files[0].getName()).getAbsoluteFile());

    		return false;
    	} else if (!files[0].getAbsolutePath().toLowerCase().endsWith(".zip"))
    		filesToBeProcessed.add(files[0].getAbsolutePath());

    	// remember the prefix of the first
    	if (filesToBeProcessed.isEmpty()) {
    		System.err.println("File list for processing happens to be empty which is weird ...!");
    		return false;
    	}
    	
		System.out.println("Revisit file list and determine conversion strategy...");
		
		// this looks at the image (series) and tries to find an appropriate conversion method
		TissueStackBioFormatsConverter.strategy = 
    		TissueStackBioFormatsConverter.revisitFileListAndDetermineStrategy(filesToBeProcessed);
    	if (TissueStackBioFormatsConverter.strategy == ConversionStrategy.UNKNOWN)
    	{
    		System.err.println("Couldn't find an appropriate conversion strategy!");
    		return false;
    	}
    		
		System.out.println("Chosen Conversion Strategy: " + TissueStackBioFormatsConverter.strategy + " [avoid 3D reconstruction flag: " +
				TissueStackBioFormatsConverter.avoid3DReconstruction + "]");
		
    	RandomAccessFile writer = null;
    	String format = null;
    	int headerLength = 0;
    	int sizeX = 0;
    	int sizeY = 0;
    	
    	List<String> filesToBeProcessedInParallel = new ArrayList<String>(filesToBeProcessed.size());
    	
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
	   				if (reader.isRGB() || reader.getSizeT() == 3)
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

	   				long freeMemEstimate = TissueStackBioFormatsConverter.getFreeMemoryEstimate();
	   				// we only ever process 1 series
		   			if (reader.getSeriesCount() > 1) {
		   				long lowestFreeMemoryRemaining = Long.MAX_VALUE;
		   				for (int s=0;s<reader.getSeriesCount();s++) {
		   					reader.setSeries(s);
		   					long projectedMemUsage = (long) reader.getSizeX() * reader.getSizeY() * 3 * 2 * 8;
		   					long tmpFreeMemRemaining = freeMemEstimate - projectedMemUsage;
		   					if (tmpFreeMemRemaining > 0 && tmpFreeMemRemaining < lowestFreeMemoryRemaining) {
		   						lowestFreeMemoryRemaining = tmpFreeMemRemaining;
		   						TissueStackBioFormatsConverter.series = s;
		   					}
		   				}
		   					
		   				reader.setSeries(TissueStackBioFormatsConverter.series);
		   			} else 
		   				reader.setSeries(TissueStackBioFormatsConverter.series);
		   			
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
	   				
	   				System.out.println("Number of Series: " + reader.getSeriesCount() + " [using: " + 
	   				TissueStackBioFormatsConverter.series + "]");
	   				
	   				TissueStackBioFormatsConverter.metaInfoFileImageCount = reader.getImageCount();
	   				System.out.println("Number of Images (incl. channels/time/slice): " + TissueStackBioFormatsConverter.metaInfoFileImageCount);
	   				System.out.println("Image Plane Dimension (X times Y): " + sizeX + " x " + sizeY);
	   				System.out.println("Number of Slices (Z): " + reader.getSizeZ());
	   				System.out.println("Number of Points in Time (T): " + reader.getSizeT());
	   				System.out.println("Number of Channels: " + reader.getSizeC());
	   				System.out.println("Looping over remaining files (might take a while) ... ");
	   				
	   				// open raw file for output
		   			writer = new RandomAccessFile(files[1], "rwd");

		   			// write out header
		   			StringBuffer header_buffer = new StringBuffer(100);
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED &&
		   					!TissueStackBioFormatsConverter.avoid3DReconstruction) {
			   			header_buffer.append(numberOfSlicesToBeProcessed);
		   				header_buffer.append(":");
		   			} 
	   				header_buffer.append(sizeX); // first dimensions
		   			header_buffer.append(":");
		   			header_buffer.append(sizeY);
		   			if (strategy == ConversionStrategy.TIME_SERIES) {
			   			header_buffer.append(":");		   				
		   				header_buffer.append(numberOfTimePointsToBeProcessed);
		   			} else if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED &&
		   				TissueStackBioFormatsConverter.avoid3DReconstruction) {
			   			header_buffer.append(":");		   				
		   				header_buffer.append(numberOfSlicesToBeProcessed);
		   			}
		   			header_buffer.append("|"); // now coordinates TODO: extract meta-info
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED &&
		   					!TissueStackBioFormatsConverter.avoid3DReconstruction)
		   				header_buffer.append("0:");
		   			header_buffer.append("0:0");
		   			header_buffer.append("|");	// now spacing/step size TODO: extract meta-info
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED  &&
		   					!TissueStackBioFormatsConverter.avoid3DReconstruction)
		   				header_buffer.append("1:");
		   			header_buffer.append("1:1"); 
		   			header_buffer.append("|"); // now coordinate naming 
		   			if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED  &&
		   					!TissueStackBioFormatsConverter.avoid3DReconstruction)
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

		   			final String head = header_buffer.toString();
		   			headerLength = head.length();
		   			writer.write(head.getBytes());
		   			
		   			// calculate data portion size
		   			long dataSectionLength = 3 * sizeX * sizeY;
		   			if (strategy != ConversionStrategy.TIME_SERIES) // if time series we need n timepoints of what we have  
		   				dataSectionLength *= numberOfTimePointsToBeProcessed;
		   			if (strategy != ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED) // if 3D reconstruction we need n slices * dimensions (3)  
		   				dataSectionLength *= numberOfSlicesToBeProcessed * 3;

		   			long prospectivefileSize = headerLength + dataSectionLength;
		   			writer.setLength(prospectivefileSize);
		   			writer.close();
	   			}
	   			
	   			if (!format.equals(reader.getFormat()))
   					throw new RuntimeException(
   						"All image files have to have the same format!");

	   			// dimension checks: they have to be the same as determined initially!
   				if (reader.getSizeX() != sizeX || reader.getSizeY() != sizeY)
   					throw new RuntimeException(
   						"The dimensions of this image don't match the dimensions of the first image in the series!");

   				// bit depth check
				if ((reader.getBitsPerPixel() % 8) != 0)
   					throw new RuntimeException(
   						"The bit depth is not a multiple of 8 bit!");
   				
				if (!(reader.getPixelType() == FormatTools.INT8 || reader.getPixelType() == FormatTools.UINT8 ||
						reader.getPixelType() == FormatTools.INT16 || reader.getPixelType() == FormatTools.UINT16 ||
						reader.getPixelType() == FormatTools.INT32 || reader.getPixelType() == FormatTools.UINT32 ||
						reader.getPixelType() == FormatTools.FLOAT || reader.getPixelType() == FormatTools.DOUBLE || 
						reader.isRGB()))
					throw new RuntimeException("Unsupported Pixel Type (only RGB, 8/16/32 grayscale and float/double are allowed!");
				
				filesToBeProcessedInParallel.add(fileToBeProcessed);
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
	        	
	        	TissueStackBioFormatsConverter.errorFlag = true;
	    		return false;
	    	} finally {
	    		try {
	    			if (reader != null)
	    				reader.close();
	    		} catch (Exception ignored) {}
	    	}
    	}

    	try {
	    	if (format == null) {
	    		System.err.println("It seems we weren't given any supported files!");
	        	TissueStackBioFormatsConverter.errorFlag = true;
	    		return false;
	    	}

	    	if (filesToBeProcessedInParallel.isEmpty()) {
	    		System.err.println("It seems we have no files for the threads after perusing the file list. This is highly dubious!");
	        	TissueStackBioFormatsConverter.errorFlag = true;
	    		return false;
	    	}

	    	// now dedicate a thread to a batch of slices / time points 
	    	int end = 1;
	    	//if (filesToBeProcessedInParallel.size() > 1 && strategy == ConversionStrategy.TIME_SERIES)
	    	if (strategy == ConversionStrategy.TIME_SERIES)
				end = numberOfTimePointsToBeProcessed;
			//else if (filesToBeProcessedInParallel.size() > 1 && strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
	    	else if (strategy == ConversionStrategy.VOLUME_TO_BE_RECONSTRUCTED)
				end = numberOfSlicesToBeProcessed;
			
			// find a good divisor to distribute the work load for threads
	    	// check how high we can go, we set 3 as a default
			int highestDivisor = 3;
			int tmpDivisor = highestDivisor;
			while (tmpDivisor <= 10) {
				if (end % tmpDivisor == 0)
					highestDivisor = tmpDivisor;
				tmpDivisor++;
			}

			final int modDivisor = end % highestDivisor;
			final int numberOfThreads = 
				modDivisor == 0 ? 
					highestDivisor :
					end < highestDivisor ? end : highestDivisor;
			final int numberOfFilesPerThread = 
				end < highestDivisor ? end : end / highestDivisor;
			
			System.out.println("Number of Conversion Threads used: " + numberOfThreads + " [" + numberOfFilesPerThread + " files per thread].");
			final Thread busyWorkerBees[] = new TissueStackBioFormatsConverter.ConverterThread[numberOfThreads];
			for (int j=0;j<numberOfThreads;j++) {
				busyWorkerBees[j] =
					new TissueStackBioFormatsConverter.ConverterThread(
						filesToBeProcessedInParallel, j*numberOfFilesPerThread, (j+1)*numberOfFilesPerThread, headerLength, files);
				busyWorkerBees[j].setDaemon(false);
				busyWorkerBees[j].start();
			}

			// wait until all threads have finished

			while (true) {
				int numberOfThreadsAlive = numberOfThreads;
				for (int j=0;j<numberOfThreads;j++) 
					if (!busyWorkerBees[j].isAlive())
						numberOfThreadsAlive--;
				
				if (numberOfThreadsAlive == 0)
					break;
				
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					// don't care
				}
			}
			
			if (TissueStackBioFormatsConverter.errorFlag)
				return false;
    	} finally {
	    	// erased temporay zip files if created
	    	if (files[0].getAbsolutePath().toLowerCase().endsWith(".zip")) {
	    		TissueStackBioFormatsConverter.deleteDirectory(
		    		new File(files[2],files[0].getName()).getAbsoluteFile());
	    	}
    	}

    	return true;
    }
    
    /*
     * CHECKS FOR FILE FORMATS THAT ARE MEANT TO BE CONVERTED WITH THE TISSUESTACK CONVERTER:
     * METHOD 1 
     */
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
    
    /*
     * CHECKS FOR FILE FORMATS THAT ARE MEANT TO BE CONVERTED WITH THE TISSUESTACK CONVERTER: 
     * METHOD 2
     */
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
    
    /*
     * RECURSIVE DELETION OF DIRECTORIES
     */
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

    /*
     * 1) DEALS WITH META-DATA SPECIFICS FOR OME-XML AND TIFF
     * 2) DETERMINES CONVERSION STRATEGY
     * 3) POTENTIALLY MODIFIES PROCESSING LIST (BASED ON 1 and/or 2)
     */
    private static ConversionStrategy revisitFileListAndDetermineStrategy(ArrayList<String> filesToBeProcessed) {
    	LinkedHashSet<String> alteredFileListToBeProcessed = new LinkedHashSet<String>(filesToBeProcessed.size());
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
    
    /*
     * ZIP ARCHIVE EXTRACTION HELPER
     */
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
			        if (ze.isDirectory()) {
			        	unzippedFile.mkdirs();
			        	continue;
			        }
			    	System.out.println("Unzipping: " +  unzippedFile.getAbsoluteFile() + "...");
			        
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
		
		        	TissueStackBioFormatsConverter.errorFlag = true;
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
	
	/*
	 * CONVERSION STRATEGY DETERMINATION RULES
	 */
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
	
	/*
	 * HANDLED WORKLOAD PROGRESS INCREMENT AND DISPLAY (SYNCed FOR THREADS) 
	 */
	private static synchronized void incrementAndDisplayProgress(final String fileToBeProcessed) {
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
	
	private static long getFreeMemoryEstimate() {
		MemoryMXBean memBean = ManagementFactory.getMemoryMXBean();
		MemoryUsage heapMem = memBean.getHeapMemoryUsage();

		return (long) ((heapMem.getMax()-heapMem.getUsed()) * 0.75);
	}
}