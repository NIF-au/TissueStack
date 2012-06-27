package au.edu.uq.cai.TissueStack.resources;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Context;

import org.apache.commons.fileupload.FileItemIterator;
import org.apache.commons.fileupload.FileItemStream;
import org.apache.commons.fileupload.ProgressListener;
import org.apache.commons.fileupload.servlet.ServletFileUpload;
import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;

/*
 * TODO: create some admin functionality
 * !!!!!!! IMPORTANT : always call SecurityResources.checkSession(session) to check for session validity !!!!!!
 * 
 */
@Path("/admin")
@Description("Tissue Stack Admin Resources")
public final class AdminResources extends AbstractRestfulMetaInformation {
	
	final static Logger logger = Logger.getLogger(AdminResources.class);
	
	private final static String DEFAULT_UPLOAD_DIRECTORY = "/opt/upload";
	private final static long DEFAULT_MAX_UPLOAD_SIZE = 1024 * 1024 * 1024;
			
	@Path("/")
	public RestfulResource getDefault() {
		return this.getAdminResourcesMetaInfo();
	}
	
	@Path("/upload_directory")
	@Description("Shows contents of upload directory")
	public RestfulResource showContentOfUploadDirectory(@QueryParam("session") String session) {
		// check permissions
		/*
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Your session is not valid. Log in with the admin password first!");
		}
		*/
		return new RestfulResource(new Response(new NoResults()));
	}
	
	@Path("/upload")
	@Description("Uploads a file ")
	public RestfulResource uploadFile(@Context HttpServletRequest request, @QueryParam("session") String session) {
		// check permissions
		/*
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Your session is not valid. Log in with the admin password first!");
		}
		*/

		boolean isMultipart = ServletFileUpload.isMultipartContent(request);
		
		// preliminary check
		if (!isMultipart) {
			throw new RuntimeException("Not a file upload");
		}

		// query file upload directory
		final File uploadDirectory = this.getUploadDirectory();
		if (!uploadDirectory.exists()) {
			uploadDirectory.mkdir();
		}
		// check if we have write permission
		if (!uploadDirectory.canWrite()) {
			throw new RuntimeException("Cannot write to upload directory");
		}

		// query maximum upload size
		long maxUploadInBytes = DEFAULT_MAX_UPLOAD_SIZE;
		final Configuration maxUpSize = ConfigurationDataProvider.queryConfigurationById("max_upload_size");
		try {
			maxUploadInBytes = Long.parseLong(maxUpSize.getValue());
		} catch(Exception nullOrFailedNumberConversion) {
			// use default
		}
		
		// a custom progress listener
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
		};
		
		// Create a new file upload handler
		ServletFileUpload upload = new ServletFileUpload();
		upload.setProgressListener(progressListener);
		
		FileItemIterator files;
		InputStream in = null;
		File fileToBeUploaded = null;

		try {
			// 'loop' through results (we only want the first!) 
			files = upload.getItemIterator(request);

			while (files.hasNext()) {
			    FileItemStream file = files.next();
			    
			    if (file == null || file.getName().isEmpty()) {
			    	throw new IllegalArgumentException("No File was selected!");
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
		    					   "File " + file.getName() + " exists already in the upload destination!");
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
			    	  throw new RuntimeException("Failed to save uploaded file", writeToDiskException);
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
	
	@Path("/filename_list")
	@Description("List all file names ")
	public RestfulResource readFile(@Context HttpServletRequest request, @QueryParam("session") String session) {
		final File fileDirectory = this.getUploadDirectory();
		
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
	
	private File getUploadDirectory() {
		final Configuration upDir = ConfigurationDataProvider.queryConfigurationById("upload_directory");
		return new File(upDir == null || upDir.getValue() == null ? DEFAULT_UPLOAD_DIRECTORY : upDir.getValue());
	}
	
	@Path("/meta-info")
	@Description("Shows the Tissue Stack Admin's Meta Info.")
	public RestfulResource getAdminResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
