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

import java.io.IOException;

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
			@FormParam("name") String name,
			@FormParam("group") String group,
			@FormParam("location") String location,
			@FormParam("description") String description) {
		long idAdLong = -1;
		
		try {
			idAdLong = Long.parseLong(id);
		} catch (Exception e) {
			throw new IllegalArgumentException("Given id is either null or non-numeric!");
		}
		
		AndsDataSetRegistration.instance().registerDataSet(idAdLong, name, group, location, description);
		
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
		
		try {
			ServiceUtils.streamFileContent(
					response,
					MediaType.APPLICATION_XML,
					"charset=utf-8",
					AndsDataSetRegistration.getAndsDataSetXML());
		} catch (IOException e) {
			// can be ignored
		} catch (RuntimeException e) {
			try {
				response.sendError(500, "Failed to stream xml!");
			} catch (IOException e1) {
				// we don't care
			}
		}
	}
}
