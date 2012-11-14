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

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;

@Path("/configuration")
@Description("Tissue Stack Configuration Resources")
public final class ConfigurationResources extends AbstractRestfulMetaInformation {

	@Path("/")
	public RestfulResource getDefault() {
		return this.getConfigurationResourcesMetaInfo();
	}

	@Path("/all")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns the all configuration info for the instance")
	public Response getAllConfigurationDefault() {
		return this.getAllConfigurationJson();
	}

	@Path("/all/json")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns the all configuration info for the instance")
	public Response getAllConfigurationJson() {
		final List<Configuration> all = this.getAllConfigurationInternal();
		if (all.isEmpty()) {
			return new Response(new NoResults());
		}
		return new Response(all);
	}

	@Path("/all/xml")
	@GET
	@Produces(MediaType.APPLICATION_XML)
	@Description("Returns the all configuration info for the instance")
	public List<Configuration> getAllConfigurationXML() {
		return this.getAllConfigurationInternal();
	}

	private List<Configuration> getAllConfigurationInternal() {
		return ConfigurationDataProvider.queryAllConfigurationValues();
	}
	
	@Path("/version")
	@Description("Returns the instance's version number")
	public RestfulResource getVersion() {
		return this.getConfiguration("version");
	}

	@Path("/supports_tile_service")
	@Description("Returns whether the instance supports tiled datasets")
	public RestfulResource isTileService() {
		return this.getConfiguration("tiles");
	}

	@Path("/supports_image_service")
	@Description("Returns whether the instance supports direct image querying for datasets")
	public RestfulResource isImageService() {
		return this.getConfiguration("images");
	}

	@Path("/query")
	@Description("Returns the configuration info for a given configuration key")
	public RestfulResource getConfiguration(
			@Description("The Parameter 'key': specifies the configuration key for a certain configuration value")
			@QueryParam("key")
			String key) {
		if (key == null) {
			throw new IllegalArgumentException("Parameter 'key' is mandatory!");
		}
		
		final Configuration conf = ConfigurationDataProvider.queryConfigurationById(key.trim());
		if (conf == null) {
			return new RestfulResource(new Response(new NoResults()));
		}
		return new RestfulResource(new Response(conf));
	}

	@Path("/meta-info")
	@Description("Shows the Tissue Stack Configuration's Meta Info.")
	public RestfulResource getConfigurationResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
