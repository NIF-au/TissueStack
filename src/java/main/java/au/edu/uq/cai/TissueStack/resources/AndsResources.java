package au.edu.uq.cai.TissueStack.resources;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.ws.rs.Consumes;
import javax.ws.rs.FormParam;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.ands.AndsDataSetRegistration;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;
import au.edu.uq.cai.TissueStack.utils.ServiceUtils;

@Path("/ands")
@Description("Tissue Stack ANDS Resources")
public final class AndsResources extends AbstractRestfulMetaInformation {
	final static Logger logger = Logger.getLogger(AndsResources.class);
	
	@Path("/")
	public RestfulResource getDefault() {
		return this.getAdminResourcesMetaInfo();
	}
		
	@Path("/add_data_set")
	@Consumes(MediaType.APPLICATION_FORM_URLENCODED)
	@Description("Adds a new data set to the ands data set xml")
	public RestfulResource addDataSet(
			@Description("Internal parameter")
			@Context HttpServletRequest request,
			@Description("Mandatory: The data set id")
			@FormParam("id") String id,
			@FormParam("else") String somethingElse) {
		// TODO: adapt to real parameters coming in
		
		long idAdLong = -1;
		
		try {
			idAdLong = Long.parseLong(id);
		} catch (Exception e) {
			throw new IllegalArgumentException("Given id is either null or non-numeric!");
		}
		
		AndsDataSetRegistration.instance().registerDataSet(idAdLong);
		
		return new RestfulResource(new Response("Data Set Added."));
	}
	
	@Path("/meta-info")
	@Description("Shows the Tissue Stack Admin's Meta Info.")
	public RestfulResource getAdminResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}
	
	@Path("/xml")
	@GET
	@Produces(MediaType.APPLICATION_XML)
	@Description("Streams out the xml to a client that requests it")
	public void streamAndsDataSetXML(
			@Description("Internal parameter")
			@Context HttpServletResponse response) {
		
		ServiceUtils.streamFileContent(
				response,
				MediaType.APPLICATION_XML,
				"charset=utf-8",
				AndsDataSetRegistration.getAndsDataSetXML());
	}
}
