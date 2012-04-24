package au.edu.uq.cai.TissueStack.resources;

import java.io.File;

import javax.servlet.http.HttpServletResponse;
import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Context;

import au.edu.uq.cai.TissueStack.dataobjects.MincTest;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.jni.TissueStack;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;

@Path("/minc")
@Description("MINC Resources")
public final class MincResources extends AbstractRestfulMetaInformation {

	@Path("/test")
	@Description("Test Case")
	public RestfulResource test(
			@Description("Paramter 'minc-file': specify the file to query")
			@QueryParam("minc-file")
			String mincFile,
			@Description("Not handed over. Internal param.")					
			@Context HttpServletResponse response){
		if (mincFile == null) {
			return new RestfulResource(new Response(new NoResults()));
		} else if (!new File(mincFile).exists()) {
			throw new IllegalArgumentException("File " + mincFile + " could not be found");
		}
		MincTest results = new TissueStack().test(mincFile);
		return new RestfulResource(new Response(results));
	}

	@Path("/meta-info")
	@Description("Shows MINC Resources Meta Info.")
	public RestfulResource getMINCResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
