package au.edu.uq.cai.TissueStack.resources;

import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
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
	
	@Path("/")
	public RestfulResource getDefault() {
		return this.getAdminResourcesMetaInfo();
	}

	@Path("/upload")
	@Description("Uploads files")
	public RestfulResource uploadFiles(@QueryParam("session") String session, @QueryParam("file") String file) {
		// check permissions
		if (!SecurityResources.checkSession(session)) {
			throw new RuntimeException("Your session is not valid. Log in with the admin password first!");
		}
		
		//TODO: implement me
		return new RestfulResource(new Response(new NoResults()));
	}

	@Path("/meta-info")
	@Description("Shows the Tissue Stack Admin's Meta Info.")
	public RestfulResource getAdminResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
