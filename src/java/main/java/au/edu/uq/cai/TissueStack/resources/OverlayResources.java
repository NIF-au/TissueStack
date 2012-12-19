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

import java.util.List;

import javax.servlet.http.HttpServletResponse;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.AbstractDataSetOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.IOverlays.OverlayType;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetOverlaysProvider;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;
import au.edu.uq.cai.TissueStack.utils.ServiceUtils;

@Path("/overlays")
@Description("Tissue Stack Overlays Resources")
public final class OverlayResources extends AbstractRestfulMetaInformation {
	final static Logger logger = Logger.getLogger(OverlayResources.class);
	
	@Path("/")
	public RestfulResource getDefault() {
		return this.getOverlaysResourcesMetaInfo();
	}
		
	@Path("/id_mapping_for_slice/{dataset_id}/{planes_id}/{type}")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns a list of overlay ids for existing slice overlays (as json)")
	public void getListOfOverlayIdsForExistingSlices(
			@Description("Internal parameter")
			@Context HttpServletResponse response,
			@Description("Mandatory: The data set id")
			@PathParam("dataset_id") String dataset_id,
			@Description("Mandatory: The data set planes id")
			@PathParam("planes_id") String planes_id,
			@Description("Mandatory: The overlay type")
			@PathParam("type") String type
			) {
		if (dataset_id == null || planes_id == null || type == null)
			throw new IllegalArgumentException("Not all mandatory arguments were supplied!");

		final OverlayType overlayType = OverlayType.valueOf(type.toUpperCase());
		
		long dataSetId = 0;
		long planesId = 0;
		try {
			dataSetId = Integer.parseInt(dataset_id);
			planesId = Integer.parseInt(planes_id);
		} catch (NumberFormatException notAnumber) {
			throw new IllegalArgumentException("all ids have to be numeric!");
		}

		final List<Object[]> results = DataSetOverlaysProvider.findOverlayIdsForSlices(dataSetId, planesId, overlayType);

		StringBuffer resp = new StringBuffer(1000);
		resp.append("{\"response\":");

		if (results == null || results.isEmpty()) {
			resp.append("{\"noResults\": \"No results found\"}}");
			ServiceUtils.streamResponse(response, resp.toString());
			return;
		}

		resp.append("{");
		for (Object[] res : results) {
			resp.append("\"" + res[0] + "\": ");
			resp.append(res[1]);
			resp.append(",");
		}
		if (resp.charAt(resp.length() - 1) == ',')
			resp.deleteCharAt(resp.length()-1);
		resp.append("}}");
			
		ServiceUtils.streamResponse(response, resp.toString());
	}

	@Path("/overlay/{id}/{type}")
	@Description("Returns the overlay data (as json)")
	public RestfulResource getOverlayData(
			@Description("Mandatory: The overlay id")
			@PathParam("id") String id,
			@Description("Mandatory: The overlay type")
			@PathParam("type") String type) {
		if (id == null || type == null)
			throw new IllegalArgumentException("Not all mandatory arguments were supplied!");

		long idAsLong = 0;
		try {
			idAsLong = Integer.parseInt(id);
		} catch (NumberFormatException notAnumber) {
			throw new IllegalArgumentException("id was not numeric!");
		}

		final OverlayType overlayType = OverlayType.valueOf(type.toUpperCase());

		final List<AbstractDataSetOverlay> results = DataSetOverlaysProvider.findOverlayById(idAsLong, overlayType);

		if (results == null || results.isEmpty())
			return new RestfulResource(new Response(new NoResults()));
		
		return new RestfulResource(new Response(results.get(0)));
	}

	
	@Path("/meta-info")
	@Description("Shows the Tissue Stack Overlays Meta Info.")
	public RestfulResource getOverlaysResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}
}
