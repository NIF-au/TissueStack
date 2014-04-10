/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
package au.edu.uq.cai.TissueStack.resources;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Context;

import net.sf.json.JSONArray;
import net.sf.json.JSONException;

import org.apache.commons.fileupload.FileItemIterator;
import org.apache.commons.fileupload.FileItemStream;
import org.apache.commons.fileupload.ProgressListener;
import org.apache.commons.fileupload.servlet.ServletFileUpload;
import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataobjects.TaskAction;
import au.edu.uq.cai.TissueStack.dataobjects.TaskStatus;
import au.edu.uq.cai.TissueStack.dataprovider.ColorMapsProvider;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;
import au.edu.uq.cai.TissueStack.dataobjects.MincInfo;
import au.edu.uq.cai.TissueStack.jni.TissueStack;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;
import au.edu.uq.cai.TissueStack.rest.JSONBodyWriter;
import au.edu.uq.cai.TissueStack.utils.ImageUtils;
import au.edu.uq.cai.TissueStack.utils.StringUtils;
import au.edu.uq.cai.TissueStack.utils.TaskUtils;

/*
 * !!!!!!! IMPORTANT : always call SecurityResources.checkSession(session) to check for session validity !!!!!!
 */
@Path("/admin")
@Description("Tissue Stack Admin Resources")
public final class AdminResources extends AbstractRestfulMetaInformation {
	final static Logger logger = Logger.getLogger(AdminResources.class);
	
	private final static String DEFAULT_UPLOAD_DIRECTORY = "/opt/tissuestack/upload";
	private final static String DEFAULT_DATA_DIRECTORY = "/opt/tissuestack/data";
	private final static long DEFAULT_MAX_UPLOAD_SIZE = 1024 * 1024 * 1024 * 100; // 100 GB

	private final static Map<String, TaskStatus> uploadsMap = 
			new HashMap<String, TaskStatus>(10);

	class UploadProgressListener implements ProgressListener {
		private String fileName;
		
		public void setFilename(String filename) {
			this.fileName = filename;
		}
		
		public void update(long pBytesRead, long pContentLength, int pItems) {
	    	double progress = 
	    			   ((double) pBytesRead / pContentLength) * 100;
	    	if (progress > 100)
	    		progress = 100;
	    	
			AdminResources.updateUploadStatus(this.fileName, Math.floor(progress));
		};
	}

	private static void updateUploadStatus(String filename, double progress) {
		TaskStatus status = AdminResources.getUploadStatus(filename);
		
		if (status == null) { // we do not exist, create us
			status = new TaskStatus(filename, "0.0");
		}
		
		// update status
		status.setProgress(progress);
		// update map
		AdminResources.uploadsMap.put(filename, status);
	}
	
	private static TaskStatus getUploadStatus(String filename) {
		if (filename == null) return null;
		
		return AdminResources.uploadsMap.get(filename.trim());
	}
	
	@Path("/upload_progress")
	@Description("Returns the progress for a running upload")
	public RestfulResource queryUploadProgress(
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: filename")
			@QueryParam("file")
			String file){		
		if (file == null) 
			throw new RuntimeException("You have to supply a filename to query the upload status!");
		
		final TaskStatus status = AdminResources.getUploadStatus(file); 
		if (status == null) 
			throw new RuntimeException("No status present for file upload: " + file + "! Check filename again...");
		
		// we remove the map entry after it has been read successfully at a 100%
		if (status.getProgress() >= 100) {
			AdminResources.uploadsMap.remove(file);
		}
		return new RestfulResource(new Response(status));
	}
	
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
		
		// Create a new file upload handler
		ServletFileUpload upload = new ServletFileUpload();
		upload.setProgressListener(new UploadProgressListener());
		
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
			    
			    // set filename to be accessible within the progres listener
			    ((UploadProgressListener) upload.getProgressListener()).setFilename(file.getName());
			    
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

			    	   // init progress status
			    	   AdminResources.updateUploadStatus(fileToBeUploaded.getName(), 0.0);
			    	   
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

		// set upload to 100 per cent
		AdminResources.updateUploadStatus(fileToBeUploaded.getName(), 100);
		return new RestfulResource(new Response("Upload finished successfully."));
	}
	
	@Path("/upload_directory")
	@Description("Displays contents of the upload directory")
	public RestfulResource getContentsOfUploadDirectory(
			@Description("Optional: display only raw files")
			@QueryParam("display_raw_only") String onlyRaw,
			@Description("Optional: display only possible conversion formats")
			@QueryParam("display_conversion_formats_only") String conversionFormatsOnly) {
		final File fileDirectory = AdminResources.getUploadDirectory();
		
		final Map<String, Long> queuedTasks = TaskUtils.getMapOfFilesPresentlyInTaskQueue();
		
		final boolean displayOnlyRawFiles = (onlyRaw != null) ? Boolean.parseBoolean(onlyRaw) : false;
		final boolean displayOnlyConversionFormats = (conversionFormatsOnly != null) ? Boolean.parseBoolean(conversionFormatsOnly) : false;
		
		File[] listOfFiles = fileDirectory.listFiles(new FilenameFilter() {
			public boolean accept(File path, String fileOrDir) {
				final File full = new File(path, fileOrDir);
				if (full.isDirectory() 
						|| (queuedTasks != null && queuedTasks.containsKey(full.getAbsolutePath()))
						|| (displayOnlyRawFiles && !full.getAbsolutePath().endsWith(".raw"))
						|| (displayOnlyConversionFormats  
								&& !(full.getAbsolutePath().endsWith(".nii") 
										|| full.getAbsolutePath().endsWith(".nii.gz")
										|| full.getAbsolutePath().endsWith(".mnc")))) {
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

	@Path("/data_set_raw_files")
	@Description("Displays raw files for the registered data sets")
	public RestfulResource getRawFilesOfDataSets() {
		final List<DataSet> dataSetsConfigured = DataSetDataProvider.getDataSets(0, 1000, null, false);
		
		final Map<String, Long> queuedTasks = TaskUtils.getMapOfFilesPresentlyInTaskQueue();

		final List<String> fileNames = new ArrayList<String>(dataSetsConfigured.size());
		for (DataSet d : dataSetsConfigured) {
			final File file = new File(d.getFilename());
			if (queuedTasks == null || (queuedTasks != null && !queuedTasks.containsKey(file.getAbsolutePath())))
				fileNames.add(file.getName());
		}

 		return new RestfulResource(new Response(fileNames.toArray(new String[]{})));
	}

	@Path("/toggle_tiling")
	@Description("Sets the flag whether pre-tiling should be used or the image server")
	public RestfulResource toggleDataSetPreTiledOrImageService(
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: a dataset id")
			@QueryParam("id") String id,
			@Description("Mandatory: a flag: true or false")
			@QueryParam("flag") String flag) {
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}

		long dataSetId = -1;
		try {
			dataSetId = Long.parseLong(id);
		} catch (Exception e) {
			// use -1 and react below
		}
		if (dataSetId <=0) {
			throw new RuntimeException("Invalid id! Please check if numeric");
		}

		Boolean flagAsBoolean = null;
		try {
			flagAsBoolean = Boolean.valueOf(flag);
		} catch (Exception e) {
			// use null and react below
		}
		if (flagAsBoolean == null)	{
			throw new RuntimeException("Invalid flag! Should be true/false!");
		}
		
		DataSetDataProvider.toggleDataSetPreTiledOrImageService(dataSetId, flagAsBoolean.booleanValue());
		
		return new RestfulResource(new Response("Toggled pre-tiling/image flag successfully"));
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
					"Data Set could not be added since another configuration exists with the same input file: "
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
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: Parameter 'file': file name of the image format to be tiled (only nifti and minc are supported)")
			@QueryParam("file")
			String imageFile,
			@Description("Optional: Parameter 'new_raw_file_name': the name of the new RAW file (default: original file name + extension: raw")
			@QueryParam("new_raw_file")
			String newRawFileName){			
		
		// check permissions
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}
		
		// check existence of parameters and files
		final File uploadDir = AdminResources.getUploadDirectory();
		
		if (imageFile == null || !new File(imageFile).exists())
			throw new IllegalArgumentException("Either no file location was given or the one supplied is erroneous!");
		
		if (!imageFile.startsWith(uploadDir.getAbsolutePath()))
				throw new IllegalArgumentException("Given image file is not in upload directory. only files in the upload directory can be converted!");
		
		final int extStart = imageFile.lastIndexOf(".");
		if (extStart<1) 
			throw new IllegalArgumentException("Given image file has to be NIFTI or minc with its corresponding extension .nii/.nii.gz or .mnc !");
		
		final String ext = imageFile.substring(extStart);
		if (ImageUtils.isRawFormat(imageFile)) 
			throw new IllegalArgumentException("According to its header the given image file is raw already!");
		else if (!(ext.equalsIgnoreCase(".nii") || ext.equalsIgnoreCase(".gz") || ext.equalsIgnoreCase(".mnc")))
				throw new IllegalArgumentException("Given image file has to be NIFTI or minc with its corresponding extension .nii/.nii.gz or .mnc !");
		
		String originalFilenameWithoutExtension = imageFile.substring(0, extStart);
		if (originalFilenameWithoutExtension.toLowerCase().endsWith(".nii"))
			originalFilenameWithoutExtension = originalFilenameWithoutExtension.substring(0, originalFilenameWithoutExtension.lastIndexOf('.'));
		
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
			// this is to prevent a fatal seg fault if a mnc is being converted by the image server
			// most likely it is a hdf5 concurrency issue.
			// bottom line: we allow only 1 minc conversion at a time !
			if (TaskUtils.isAMincFileInConversionTaskList()) {
				throw new IllegalArgumentException("Only one minc conversion at a time, sorry!");
			}
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
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: Task Id")
			@QueryParam("task_id")
			String taskId){			
		return this.doTaskStuff(session, taskId, TaskAction.PROGRESS);
	}

	@Path("/resume")
	@Description("Resumes a stopped/cancelled task")
	public RestfulResource resumeTask(
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: Task Id")
			@QueryParam("task_id")
			String taskId){
		return this.doTaskStuff(session, taskId, TaskAction.RESUME);
	}

	@Path("/pause")
	@Description("Pauses a running task")
	public RestfulResource pauseTask(
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: Task Id")
			@QueryParam("task_id")
			String taskId){
		return this.doTaskStuff(session, taskId, TaskAction.PAUSE);
	}

	@Path("/cancel")
	@Description("Cancels a task")
	public RestfulResource cancelTask(
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Mandatory: Task Id")
			@QueryParam("task_id")
			String taskId){
		return this.doTaskStuff(session, taskId, TaskAction.CANCEL);
	}

	private RestfulResource doTaskStuff(String session, String taskId, TaskAction action) {
		// check permissions
		if (action != TaskAction.PROGRESS && !SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}
		
		if (taskId == null || taskId.length() != 16)
		throw new IllegalArgumentException("Task Id has to be a non-empty string of 16 alphanumeric characters!");
	    
		TaskStatus ret = new TissueStack().callTaskAction(taskId, (short)action.ordinal());
		if (ret == null)  throw new RuntimeException("Task with id '" + taskId + "' does not exist!");
		
		// now let JNI do the rest
		return new RestfulResource(new Response(ret));
	}

	@Path("/tile")
	@Description("Tiles a given data set")
	public RestfulResource tileDataSet(
			@Description("Mandatory: a user session token")
			@QueryParam("session") String session,
			@Description("Parameter 'base_directory': base directory for the tiles. default: /tmp/tiles")
			@QueryParam("tile_dir")
			String baseDirectory,
			@Description("Parameter 'dataset_id': data set id of the data set to be tiled (Either 'file' or 'dataset_id' have to be given!!!)")
			@QueryParam("dataset_id")
			String dataSetId,
			@Description("Parameter 'file': file name of the data to be tiled (Either 'file' or 'dataset_id' have to be given!!!)")
			@QueryParam("file")
			String mincFile,
			@Description("Optional: dimensions to be tiled (comma separated) default: -1")
			@QueryParam("dimensions")
			String dimensions,
			@Description("Optional: zoom level. default: 0")
			@QueryParam("zoom")
			String zoom,
			@Description("Optional: tile size. default: 256")
			@QueryParam("tile_size")
			String tileSize,
			@Description("Optional: tile as low res preview ('true' or 'false'), default: false")
			@QueryParam("preview")
			String preview,
			@Description("Optional: store a non-existant data set ('true' or 'false'), default: false")
			@QueryParam("store_data_set")
			String storeDataSet,
			@Description("Optional: the image type, default: PNG")
			@QueryParam("image_type")
			String imageType,
			@Description("Optional: a color map, default: grey")
			@QueryParam("color_map")
			String colorMap){	
		// check permissions
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Invalid Session! Please Log In.");
		}
		
		// check tile directory
		if (baseDirectory == null || baseDirectory.trim().isEmpty()) {
			baseDirectory = "/tmp/tiles";
		}
		File tileDir = new File(baseDirectory);
		if (!tileDir.exists()) {
			// try to create non existing directories
			if (!tileDir.mkdirs()) {
				throw new IllegalArgumentException("Could not create tile directory: " + tileDir.getAbsolutePath());
			}
		}

		DataSet dataSet = null;
		MincInfo associatedMincInfo = null;

		boolean missingSource = true;
		// look for data set id
		try {
			dataSet = DataSetDataProvider.queryDataSetById(Long.parseLong(dataSetId));
			if (dataSet != null) {
				missingSource = false;
				mincFile = dataSet.getFilename();
			}
		} catch(Exception idNotThereOrNotNUmeric) {
			// we ignore that
		}
		// look for file
		if (dataSet == null && mincFile != null && new File(mincFile).exists()) {
			missingSource = false;
			
			// we only allow raw converted files any more....
			if (!ImageUtils.isRawFormat(mincFile)) 
				throw new RuntimeException("Given file is not in RAW format. Please convert first!");
			
			try {
				dataSet = DataSetDataProvider.queryDataSetByFileName(mincFile);
			} catch (Exception e) {
				// we can ignore that, we have to rely on the file solely
			}
		}

		// check whether we have either a file or valid id as input
		if (missingSource) {
			throw new IllegalArgumentException(
					"Not a valid source. You have to hand in either an existing data set id or a valid minc file location!");
		}

		boolean storeDataSetAsBoolean = false; 
		try {
			storeDataSetAsBoolean = Boolean.parseBoolean(storeDataSet);
		} catch (Exception e) {
			// fall back onto default
		}
		
		// evaluate image type
		if (imageType == null || imageType.trim().isEmpty()) {
			imageType = "PNG";
		}
		// evaluate colormap
		if (colorMap == null || colorMap.trim().isEmpty() ||
				!ColorMapsProvider.instance().containsColorMap(colorMap)) {
			colorMap = "grey";
		}
		final TissueStack jniTissueStack = new TissueStack();
		associatedMincInfo = jniTissueStack.getMincInfo(mincFile);
		// if we didn't find a data set in the db, query the minc file and populate a data set
		if (dataSet == null) {
			dataSet = DataSet.fromMincInfo(associatedMincInfo);
			if (storeDataSetAsBoolean) DataSetDataProvider.insertNewDataSets(dataSet);
		}
		// throw an error if we could not create a data set out of the given minc file
		if (dataSet == null || associatedMincInfo == null) {
			throw new RuntimeException("Could not create data set from given minc file: " + mincFile + ". Check if valid...");
		}
		
		// augment the path by the data set id
		tileDir = new File(tileDir, String.valueOf(dataSet.getId()));
		
		// check rest of params now
		final String dims[] = StringUtils.convertCommaSeparatedQueryParamsIntoStringArray(dimensions, true);
		int dimensionsArray[] = new int[] {0,0,0,0,0,0};
		// fill array with given values
		if (dims != null) {
			if (dims.length > 6) {
				throw new RuntimeException("Dimension parameter can at most have 6 comma separated values (== 3 dimensions)");
			}

			// convert strings and populate dimensionsArray
			int counter = 0;
			for (String d : dims) {
				try {
					dimensionsArray[counter] = Integer.parseInt(d);
				} catch (Exception e) {
					// propagate
					throw new RuntimeException("Dimension parameter has to be numeric (only exception: comma)");
				}
				counter++;
			}
			
			// double check bounds
			for (int i = 0;i<dimensionsArray.length;i++) {
				int arrayValue = dimensionsArray[i];
				if (arrayValue < -1 || ((i / 2) < associatedMincInfo.getSizes().length && arrayValue > associatedMincInfo.getSizes()[i / 2] - 1)) {
					throw new RuntimeException("A given dimension value is outside the min/max range");
				}
				// check if start is not exceeding end
				if ((i % 2) == 0 && dimensionsArray[i+1] != 0 && arrayValue > dimensionsArray[i+1]) {
					throw new RuntimeException("The start for a given dimension value is larger than the end value for that dimension");
				}
			}
		}
		
		int zoomLevel = 0;
		try {
			zoomLevel = Integer.parseInt(zoom);
		} catch (Exception e) {
			// fall back onto default
		}
		
		double zoomFactor = 1;
		try {
			final String zoomLevelJson = dataSet.getPlanes().get(0).getZoomLevels();
			final JSONArray zoomLevels = JSONArray.fromObject(zoomLevelJson, JSONBodyWriter.getJsonConfig());
			if (zoomLevel < 0 || zoomLevel >= zoomLevels.size()) {
				throw new RuntimeException("Zoom Level '" + zoomLevel + "' is out of bounds.");
			}
			Object zoomFactorObject = zoomLevels.get(zoomLevel); 
			if (zoomFactorObject instanceof Double) {
				zoomFactor = (Double) zoomFactorObject;				
			} else if (zoomFactorObject instanceof Integer) {
				zoomFactor = ((Integer) zoomFactorObject).doubleValue();				
			} else {
				throw new RuntimeException("Zoom Factor in json is of an incompatible type: " + zoomFactorObject.toString());
			}
		} catch (RuntimeException e) {
			// propagate if not json or null pointer exception
			if (!(e instanceof JSONException) && !(e instanceof NullPointerException)) {
				throw e;
			}
		}

		// augment the path by the zoom level
		tileDir = new File(tileDir, String.valueOf(zoomLevel));

		boolean previewAsBoolean = false; 
		try {
			previewAsBoolean = Boolean.parseBoolean(preview);
		} catch (Exception e) {
			// fall back onto default
		}
		
		int tileSizeAsInt = 256;
		try {
			tileSizeAsInt = Integer.parseInt(tileSize);
		} catch (Exception e) {
			// fall back onto default
		}
		
		// now call the native tilling method and return
		return new RestfulResource(
				new Response(
						jniTissueStack.tileMincVolume(
								dataSet.getFilename(),
								tileDir.getAbsolutePath(),
								dimensionsArray,
								tileSizeAsInt,
								zoomFactor,
								imageType.trim(),
								colorMap.trim(),
								previewAsBoolean)));
	}
	
	@Path("/meta-info")
	@Description("Shows the Tissue Stack Admin's Meta Info.")
	public RestfulResource getAdminResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}
}
