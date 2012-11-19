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

import javax.servlet.http.HttpServletResponse;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;

import au.edu.uq.cai.TissueStack.dataobjects.ColorMap;
import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.ColorMapsProvider;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;
import au.edu.uq.cai.TissueStack.utils.ServiceUtils;

@Path("/colormaps")
@Description("Color Maps")
public final class ColorMapResources extends AbstractRestfulMetaInformation {
	@Path("/")
	public RestfulResource getDefault() {
		return this.getColormapsResourcesMetaInfo();
	}

	@Path("/all/json")
	@Description("Returns the all color maps as json")
	public void getAllColorMapsAsJson(
			@Description("Internal parameter")
			@Context HttpServletResponse response) {
		String resp = "";
		if (ColorMapsProvider.instance() == null) { // no color maps loaded for some reason (see log)
			// fall back onto default ones from the database
			final Configuration defaultColorMaps = ConfigurationDataProvider.queryConfigurationById("color_maps");
			if (defaultColorMaps == null || defaultColorMaps.getValue() == null) 
				throw new RuntimeException("Could not find default color maps in the database!");
			
			resp = defaultColorMaps.getValue();
		} else {
			resp = ColorMapsProvider.instance().getColorMapsAsJson();
		}
		this.streamResponse(response, resp);
	}

	@Path("/{name}/json")
	@Description("Returns a given color map as json")
	public void getColorMapByIdName(
			@Description("Internal parameter")
			@Context HttpServletResponse response,			
			@Description("Mandatory Paramter 'name': name of the colormap")
			@PathParam("name") String name) {
		if (name == null || name.trim().isEmpty())
			throw new IllegalArgumentException("Parameter 'name' is mandatory!");
		
		if (ColorMapsProvider.instance() == null) 
				throw new RuntimeException("Could not find color map in the database!");
		
		final ColorMap map = ColorMapsProvider.instance().getColorMap(name);
		this.streamResponse(response, map.getJson());
	}

	private void streamResponse(HttpServletResponse response, String resp) {
		try {
			ServiceUtils.streamString(
					response,
					MediaType.APPLICATION_JSON,
					"charset=utf-8",
					resp);
		} catch (RuntimeException e) {
			try {
				response.sendError(500, "Failed to stream json!");
			} catch (IOException e1) {
				// we don't care
			}
		}
	}
	
	@Path("/meta-info")
	@Description("Shows the Tissue Stack Color Maps Resource's Meta Info.")
	public RestfulResource getColormapsResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}
}
