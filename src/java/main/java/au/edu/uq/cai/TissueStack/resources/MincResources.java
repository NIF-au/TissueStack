package au.edu.uq.cai.TissueStack.resources;

import java.io.File;

import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;

import au.edu.uq.cai.TissueStack.dataobjects.MincInfo;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.jni.TissueStack;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;

@Path("/minc")
@Description("MINC Resources")
public final class MincResources extends AbstractRestfulMetaInformation {

	@Path("/info")
	@Description("Test Case")
	public RestfulResource getInfo(
			@Description("Paramter 'file': specify the file to query")
			@QueryParam("file")
			String mincFile){
		if (mincFile == null) {
			return new RestfulResource(new Response(new NoResults()));
		} else if (!new File(mincFile).exists()) {
			throw new IllegalArgumentException("File " + mincFile + " could not be found");
		}
		MincInfo results = new TissueStack().getMincInfo(mincFile);
		return new RestfulResource(new Response(results));
	}

	@Path("/meta-info")
	@Description("Shows MINC Resources Meta Info.")
	public RestfulResource getMINCResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
