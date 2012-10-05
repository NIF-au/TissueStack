package au.edu.uq.cai.TissueStack.resources;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.InputStream;
import java.io.OutputStream;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Context;

import org.apache.commons.fileupload.FileItemIterator;
import org.apache.commons.fileupload.FileItemStream;
import org.apache.commons.fileupload.servlet.ServletFileUpload;
import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataobjects.TaskStatus;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;
import au.edu.uq.cai.TissueStack.dataobjects.MincInfo;
import au.edu.uq.cai.TissueStack.jni.TissueStack;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;
import au.edu.uq.cai.TissueStack.utils.ImageUtils;

/*
 * !!!!!!! IMPORTANT : always call SecurityResources.checkSession(session) to check for session validity !!!!!!
 */
@Path("/admin")
@Description("Tissue Stack Admin Resources")
public final class AdminResources extends AbstractRestfulMetaInformation {
	
	final static Logger logger = Logger.getLogger(AdminResources.class);
	
	private final static String DEFAULT_UPLOAD_DIRECTORY = "/opt/upload";
	private final static String DEFAULT_DATA_DIRECTORY = "/opt/data";
	private final static long DEFAULT_MAX_UPLOAD_SIZE = 1024 * 1024 * 1024;
			
	@Path("/")
	public RestfulResource getDefault() {
		return this.getAdminResourcesMetaInfo();
	}
		
	@Path("/upload")
	@Description("Uploads a file to the upload directory")
	public RestfulResource uploadFile(
			@Description("Internal parameter")
			@Context HttpServletRequest request,
			@Description("Mandatory: The session token")
			@QueryParam("session") String session) {
		// check permissions
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}
		
		boolean isMultipart = ServletFileUpload.isMultipartContent(request);
		
		// preliminary check
		if (!isMultipart) {
			throw new RuntimeException("Not a File Upload");
		}

		// query file upload directory
		final File uploadDirectory = AdminResources.getUploadDirectory();
		if (!uploadDirectory.exists()) {
			uploadDirectory.mkdir();
		}
		// check if we have write permission
		if (!uploadDirectory.canWrite()) {
			throw new RuntimeException("Cannot Write to Upload Directory");
		}

		// query maximum upload size
		long maxUploadInBytes = DEFAULT_MAX_UPLOAD_SIZE;
		final Configuration maxUpSize = ConfigurationDataProvider.queryConfigurationById("max_upload_size");
		try {
			maxUploadInBytes = Long.parseLong(maxUpSize.getValue());
		} catch(Exception nullOrFailedNumberConversion) {
			// use default
		}
		
		/* a custom progress listener
		ProgressListener progressListener = new ProgressListener(){
		   // TODO: this has to become more outside accessible for front-end progress updates
		   public void update(long pBytesRead, long pContentLength, int pItems) {
		       if (pContentLength == -1) {
		           System.out.println("So far, " + pBytesRead + " bytes have been read.");
		       } else {
		           System.out.println("So far, " + pBytesRead + " of " + pContentLength
		                              + " bytes have been read.");
		       }
		   }
		};*/
		
		// Create a new file upload handler
		ServletFileUpload upload = new ServletFileUpload();
		//upload.setProgressListener(progressListener);
		
		FileItemIterator files;
		InputStream in = null;
		File fileToBeUploaded = null;

		try {
			// 'loop' through results (we only want the first!) 
			files = upload.getItemIterator(request);

			while (files.hasNext()) {
			    FileItemStream file = files.next();
			    
			    if (file == null || file.getName() == null || file.getName().isEmpty()) {
			    	throw new IllegalArgumentException("No File was Selected!");
			    }
			    
			    if (!file.isFormField()) {
			    	OutputStream out = null;
			    	
			    	byte buffer[] = new byte[1024];
			    	long accBytesRead = 0;
			    	int bytesRead = 0;
			    	boolean erroneousUpload = true;
			    	
			    	try {
				       fileToBeUploaded = new File(uploadDirectory, file.getName());

				       if (fileToBeUploaded.exists()) {
				    	   erroneousUpload = false; // otherwise we delete the existing file
		    			   throw new RuntimeException(
		    					   "File " + file.getName() + " Exists Already In The Upload Destination!");
				       }
				       
				       // open streams
				       in = file.openStream();
			    	   out = new FileOutputStream(fileToBeUploaded);
			    	   
			    	   long lastChunkStep = -1;
			    	   
			    	   while ((bytesRead = in.read(buffer)) > 0) {
					       // if we go under 5% free disk space => stop upload
			    		   // check every 10 megs uploaded
			    		   long chunksOf1024 = accBytesRead / (1024 * 1024); 
					       if ((chunksOf1024 != lastChunkStep && chunksOf1024 % 10 == 0) 
					    		   && uploadDirectory.getUsableSpace() < uploadDirectory.getTotalSpace() * 0.03) {
					    		   throw new RuntimeException("Available disk space is less than 3% of the overall disk space! We abort the upload!");   
				    	   }
					       lastChunkStep = chunksOf1024;

			    		   if (accBytesRead > maxUploadInBytes) {
			    			   throw new RuntimeException(
			    					   "Exceeded number of bytes that can be uploaded. Present limit is: " + maxUploadInBytes);
			    		   }
			    		   out.write(buffer, 0, bytesRead);
			    		   accBytesRead += bytesRead;
			    	   }
			    	   
			    	   // flag upload as clean
			    	   erroneousUpload = false;
					} catch (RuntimeException internallyThrown) {
						// propagate
						throw internallyThrown;
			    	} catch(Exception writeToDiskException) {
			    	  throw new RuntimeException("Failed to Save Uploaded File", writeToDiskException);
			    	} finally {
			    		// partial uploads are deleted
			    		if (erroneousUpload) {
			    			try {
			    				fileToBeUploaded.delete();
			    			} catch (Exception ignored) {
			    				// we can safely ignore that
							}
			    		}
			    		
			    		// clean up
			    		try {
			    			out.close();
			    		} catch(Exception ignored) {
			    			//ignored
			    		}
			    	}
			    }
			    // only one file is supposed to come along
			    break;
			}
		} catch (RuntimeException internallyThrown) {
			// log and propagate
			logger.error("Failed to upload file", internallyThrown);
			throw internallyThrown;
		} catch (Exception e) {
			logger.error("Failed to upload file", e);
			throw new RuntimeException("Failed to upload file", e);
		} finally {
    		try {
	    		// clean up
    			in.close();
    		} catch(Exception ignored) {
    			// ignored
    		}
		}
			
		return new RestfulResource(new Response("Upload finished successfully."));
	}
	
	@Path("/upload_directory")
	@Description("Displays contents of upload directory")
	public RestfulResource readFile() {
		final File fileDirectory = AdminResources.getUploadDirectory();
		
		File[] listOfFiles = fileDirectory.listFiles(new FilenameFilter() {
			public boolean accept(File path, String fileOrDir) {
				final File full = new File(path, fileOrDir);
				if (full.isDirectory()) {
					return false;
				}
				return true;
			}
		});
		String fileNames[] = new String[listOfFiles.length];
		int i = 0;
		while (i<fileNames.length) {
			fileNames[i] = listOfFiles[i].getName();
			i++;
		}

 		return new RestfulResource(new Response(fileNames));
	}
	
	@Path("/add_dataset")
	@Description("add uploaded dataset to the configuration database")
	public RestfulResource updateDataSet(
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: the file name of the already uploaded file")
			@QueryParam("filename") String filename,
			@Description("Optional: A description for the data set that is about to be added")
			@QueryParam("description") String description) {
		// check user session
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}
		
		// check for empty filename
		if (filename == null || filename.trim().isEmpty()) {
			throw new IllegalArgumentException("File Parameter Is Empty");
		}
		// check for existence of file
		final File uploadedFile = new File(AdminResources.getUploadDirectory(), filename); 
		if (!uploadedFile.exists()) {
			throw new IllegalArgumentException("File '" + uploadedFile.getAbsolutePath() + "' does Not Exist");
		}
		
		// we only allow raw converted files any more....
		if (!ImageUtils.isRawFormat(uploadedFile)) 
			throw new RuntimeException("Given file is not in RAW format. Please convert first!");
		
		// call native code to read meta info from minc file
		final MincInfo newDataSet = new TissueStack().getMincInfo(uploadedFile.getAbsolutePath());
		if (newDataSet == null) {
			throw new RuntimeException("File '" + uploadedFile.getAbsolutePath() + "' could not be processed by native minc info libs. Check whether it is a minc file...");
		}
		// convert minc file info into data set to be stored in the db later on
		final DataSet dataSetToBeAdded = DataSet.fromMincInfo(newDataSet);
		if (dataSetToBeAdded == null) {
			throw new RuntimeException("Failed to convert minc file '" + uploadedFile.getAbsolutePath() + "' into a data set...");
		}
		// set custom description if it exists
		if (description != null && !description.trim().isEmpty()) {
			dataSetToBeAdded.setDescription(description);
		}

		// relocate file from upload directory to data directory
		final File dataDir = this.getDataDirectory();
		if (!dataDir.exists()) { // try to create it if it doesn't exist
			dataDir.mkdirs();
		}
		final File destination = new File(dataDir, uploadedFile.getName());
		// double check if file exists already
		if (destination.exists()) {
			throw new RuntimeException(
					"File cannot be moved to the data directory since another file exists with the same name: "
							+ destination.getAbsolutePath());
		}
		
		if (!uploadedFile.renameTo(destination)) {
			throw new RuntimeException(
					"Failed to move uploaded file (" + uploadedFile.getAbsolutePath()
					+ ") to destination: " + destination.getAbsolutePath());
		}
		dataSetToBeAdded.setFilename(destination.getAbsolutePath());
		
		// store dataSetToBeAdded in the database
		try {
			DataSetDataProvider.insertNewDataSets(dataSetToBeAdded);
		} catch(RuntimeException any) {
			// move file back to upload directory
			destination.renameTo(uploadedFile);
			// propagate error
			throw any;
		}

		// return the new data set
		return new RestfulResource(new Response(dataSetToBeAdded));
	}
		
	public static File getUploadDirectory() {
		final Configuration upDir = ConfigurationDataProvider.queryConfigurationById("upload_directory");
		return new File(upDir == null || upDir.getValue() == null ? DEFAULT_UPLOAD_DIRECTORY : upDir.getValue());
	}

	private File getDataDirectory() {
		final Configuration dataDir = ConfigurationDataProvider.queryConfigurationById("data_directory");
		return new File(dataDir == null || dataDir.getValue() == null ? DEFAULT_DATA_DIRECTORY : dataDir.getValue());
	}

	@Path("/convert")
	@Description("Converts a given minc/nifti file to RAW")
	public RestfulResource convertImageFormatToRaw(
			@Description("Mandatory: Parameter 'file': file name of the image format to be tiled (only nifti and minc are supported)")
			@QueryParam("file")
			String imageFile,
			@Description("Optional: Parameter 'new_raw_file_name': the name of the new RAW file (default: original file name + extension: raw")
			@QueryParam("new_raw_file")
			String newRawFileName){			
		/*
		 *  TODO: put back in once working
		// check permissions
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}*/
		
		// check existence of parameters and files
		final File uploadDir = AdminResources.getUploadDirectory();
		
		if (imageFile == null || !new File(imageFile).exists())
			throw new IllegalArgumentException("Either no file location was given or the one supplied is erroneous!");
		
		if (!imageFile.startsWith(uploadDir.getAbsolutePath()))
				throw new IllegalArgumentException("Given image file is not in upload directory. only files in the upload directory can be converted!");
		
		final int extStart = imageFile.lastIndexOf(".");
		if (extStart<1) 
			throw new IllegalArgumentException("Given image file has to be NIFTI or minc with its corresponding extension .nii or .mnc !");
		
		final String ext = imageFile.substring(extStart);
		if (ImageUtils.isRawFormat(imageFile)) 
			throw new IllegalArgumentException("According to its header the given image file is raw already!");
		else if (!(ext.equalsIgnoreCase(".nii") || ext.equalsIgnoreCase(".mnc")))
				throw new IllegalArgumentException("Given image file has to be NIFTI or minc with its corresponding extension .nii or .mnc !");
		
		final String originalFilenameWithoutExtension = imageFile.substring(0, extStart);
		
		// assemble new raw file name, either by using the given name or the default: original file name with extension changed to .raw
		if (newRawFileName != null && !newRawFileName.trim().isEmpty()) {
			// append upload directory at beginning if not supplied
			if (!newRawFileName.startsWith(uploadDir.getAbsolutePath()))
				newRawFileName = new File (uploadDir, newRawFileName).getAbsolutePath();
			
			// see if we have an extension raw at the end already, if not append it
			if (!newRawFileName.endsWith(".raw"))
				newRawFileName += ".raw";
		} else {
			newRawFileName = originalFilenameWithoutExtension + ".raw";
		}
		
		final File newRawFile = new File(newRawFileName);
		if (newRawFile.exists())
			throw new IllegalArgumentException("Raw file '" + newRawFile.getAbsolutePath() + "' exists already. Please supply a different file name for the new raw file");
			
		// check whether we have a minc file on our hands
		short formatIdentifier = 1; // 1 for minc, 2 for nifti
		if (ext.equalsIgnoreCase(".mnc")) {
			final MincInfo mincTest = new TissueStack().getMincInfo(imageFile);
			if (mincTest == null)
				throw new IllegalArgumentException("Given file had .mnc extension but proved to be non minc. If it is in fact minc version 1, please convert it to minc 2 before you launch the raw file conversion!");
		} else 
			formatIdentifier = 2; // we assume it is the only other possibility left, if not the converter will give us an error anyhow!
		
		// now let JNI do the rest
		return new RestfulResource(
				new Response(new TissueStack().convertImageFormatToRaw(imageFile, newRawFile.getAbsolutePath(), formatIdentifier)));		
	}

	@Path("/progress")
	@Description("Returns the progress for a running/finished task")
	public RestfulResource queryTaskProgress(
			@Description("Mandatory: Task Id")
			@QueryParam("task_id")
			String taskId){			
		/*
		 *  TODO: put back in once working
		// check permissions
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}*/
		if (taskId == null || taskId.length() != 10)
			throw new IllegalArgumentException("Task Id has to be a non-empty string of 10 alphanumeric characters!");
		
		TaskStatus ret = new TissueStack().queryTaskProgress(taskId);
		if (ret == null)  throw new RuntimeException("Task with id '" + taskId + "' does not exist!");
		
		// now let JNI do the rest
		return new RestfulResource(new Response(ret));		
	}
	
	@Path("/meta-info")
	@Description("Shows the Tissue Stack Admin's Meta Info.")
	public RestfulResource getAdminResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}
}
