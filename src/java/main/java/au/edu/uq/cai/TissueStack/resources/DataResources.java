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

import java.util.ArrayList;
import java.util.List;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import org.jboss.resteasy.annotations.providers.jaxb.Wrapped;

import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetLookupMapping;
import au.edu.uq.cai.TissueStack.dataobjects.IGlobalConstants;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetLookupMappingDataProvider;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;

@Path("/data")
@Description("Data Resources")
public final class DataResources extends AbstractRestfulMetaInformation {

	private static final int MAX_RECORDS = 100;

	@Path("/")
	public RestfulResource getDefault() {
		return this.getDataResourcesMetaInfo();
	}

	@Path("/{id}")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns the data set information for a given id (if exists) or a not found response")
	public Response getDataSetByIdDefault(
			@Description("Mandatory Paramter 'id': points towards the record id in the database")
			@PathParam("id") String id) {
		return this.getDataSetByIdAsJson(id);
	}

	@Path("/{id}/json")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns the data set information for a given id (if exists) or a not found response")
	public Response getDataSetByIdAsJson(
			@Description("Mandatory Paramter 'id': points towards the record id in the database")
			@PathParam("id") String id) {
		final DataSet dataSet = this.getDataSetByIdInternal(id);
		if (dataSet == null) {
			return new Response(new NoResults());
		}
		return new Response(dataSet);
	}

	@Path("/{id}/xml")
	@GET
	@Produces(MediaType.APPLICATION_XML)
	@Description("Returns the data set information for a given id (if exists) or a not found response")
	public Object getDataSetByIdAsXML(
			@Description("Mandatory Paramter 'id': points towards the record id in the database")
			@PathParam("id") String id) {
		final DataSet result = this.getDataSetByIdInternal(id);
		if (result == null) {
			return new NoResults();
		}
		
		return result;
	}

	@Path("/{id}/mapping/json")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns the data set lookup mapping information for a given id (if exists) or a not found response (as json)")
	public Response getDataSetMappingByIdAsJson(
			@Description("Mandatory Paramter 'id': points towards the record id in the database")
			@PathParam("id") String id,
			@Description("Optional Paramter 'include_plane_data': queries and returns associated plane data as well. Defaults to false.")
			@QueryParam("include_plane_data") String includePlaneData) {
		final List<DataSetLookupMapping> result = this.getDataSetMappingsByIdInternal(id, includePlaneData);
		if (result == null) {
			return new Response(new NoResults());
		}
		return new Response(result);
	}

	@Path("/{id}/mapping/xml")
	@GET
	@Produces(MediaType.APPLICATION_XML)
	@Wrapped(element="DataSetMappings", prefix=IGlobalConstants.XML_PREFIX, namespace=IGlobalConstants.XML_NAMESPACE)
	@Description("Returns the data set lookup mapping information for a given id (if exists) or a not found response (as xml)")
	public List<DataSetLookupMapping> getDataSetMappingByIdAsXML(
			@Description("Mandatory Paramter 'id': points towards the record id in the database")
			@PathParam("id") String id,
			@Description("Optional Paramter 'include_plane_data': queries and returns associated plane data as well. Defaults to false.")
			@QueryParam("include_plane_data") String includePlaneData) {
		final List<DataSetLookupMapping> result = this.getDataSetMappingsByIdInternal(id, includePlaneData);
		if (result == null) {
			return new ArrayList<DataSetLookupMapping>(0);
		}
		
		return result;
	}

	private DataSet getDataSetByIdInternal(String id) {
		long idAsLong = -1;
		if (id == null) {
			throw new IllegalArgumentException("Parameter 'id' is mandatory!");
		}
		try {
			idAsLong = Long.parseLong(id);
		} catch (Exception e) {
			throw new IllegalArgumentException("Parameter 'id' is not numeric!");
		}
		return DataSetDataProvider.queryDataSetById(idAsLong);
	}

	private List<DataSetLookupMapping> getDataSetMappingsByIdInternal(String id, String includePlaneData) {
		long idAsLong = -1;
		if (id == null) {
			throw new IllegalArgumentException("Parameter 'id' is mandatory!");
		}
		try {
			idAsLong = Long.parseLong(id);
		} catch (Exception e) {
			throw new IllegalArgumentException("Parameter 'id' is not numeric!");
		}
		boolean includePlaneDataAsBoolean = Boolean.parseBoolean(includePlaneData);

		final List<DataSetLookupMapping> mappings = 
				DataSetLookupMappingDataProvider.queryDataSetLookupMappingForGivenDataSetId(idAsLong, includePlaneDataAsBoolean);
		if (mappings == null || mappings.isEmpty()) return null;
		
		return mappings;
	}

	@Path("/list")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns all data sets in a paginated fashion, i.e offset=0&max_records=10")
	public Response getDataSetsDefault(
			@Description("Optional Paramter 'offset': start record for pagination. Defaults to 0.")
			@QueryParam("offset") String offset,
			@Description("Optional Paramter 'max_records': start record for pagination. Defaults to" + MAX_RECORDS + ".")
			@QueryParam("max_records") String maxRecords,
			@Description("Optional Paramter 'description': a filter expression which is searched for in the records' description column")
			@QueryParam("description") String description,
			@Description("Optional Paramter 'include_plane_data': queries and returns associated plane data as well. Defaults to false.")
			@QueryParam("include_plane_data") String includePlaneData) {
		return this.getDataSetsAsJson(offset, maxRecords, description, includePlaneData);
	}

	@Path("/list/json")
	@GET
	@Produces(MediaType.APPLICATION_JSON)
	@Description("Returns all data sets in a paginated fashion, i.e offset=0&max_records=10")
	public Response getDataSetsAsJson(
			@Description("Optional Paramter 'offset': start record for pagination. Defaults to 0.")
			@QueryParam("offset") String offset,
			@Description("Optional Paramter 'max_records': start record for pagination. Defaults to" + MAX_RECORDS + ".")
			@QueryParam("max_records") String maxRecords,
			@Description("Optional Paramter 'description': a filter expression which is searched for in the records' description column")
			@QueryParam("description") String description,
			@Description("Optional Paramter 'include_plane_data': queries and returns associated plane data as well. Defaults to false.")
			@QueryParam("include_plane_data") String includePlaneData) {
		final List<DataSet> result = this.getAllDataSetsPaginated(offset, maxRecords, description, includePlaneData);
		if (result.isEmpty()) {
			return new Response(new NoResults());
		}
		return new Response(result);
	}

	@Path("/list/xml")
	@GET
	@Produces(MediaType.APPLICATION_XML)
	@Wrapped(element="DataSets", prefix=IGlobalConstants.XML_PREFIX, namespace=IGlobalConstants.XML_NAMESPACE)
	@Description("Returns all data sets in a paginated fashion, i.e offset=0&max_records=10")
	public List<DataSet> getDataSetsAsXML(
			@Description("Optional Paramter 'offset': start record for pagination. Defaults to 0.")
			@QueryParam("offset") String offset,
			@Description("Optional Paramter 'max_records': start record for pagination. Defaults to" + MAX_RECORDS + ".")
			@QueryParam("max_records") String maxRecords,
			@Description("Optional Paramter 'description': a filter expression which is searched for in the records' description column")
			@QueryParam("description") String description,
			@Description("Optional Paramter 'include_plane_data': queries and returns associated plane data as well. Defaults to false.")
			@QueryParam("include_plane_data") String includePlaneData) {
		return this.getAllDataSetsPaginated(offset, maxRecords, description, includePlaneData);
	}

	private List<DataSet> getAllDataSetsPaginated(String offset, String maxRecords, String description, String includePlaneData) {
		int offsetAsInt = 0;
		if (offset != null) {
			try {
				offsetAsInt = Integer.parseInt(offset);
				if (offsetAsInt < 0) {
					// trigger exception
					throw new IllegalArgumentException();
				}
			} catch (Exception e) {
				throw new IllegalArgumentException("Parameter 'offset' has to be numeric and not negative!");
			}
		}

		int maxRecordsAsInt = MAX_RECORDS;
		if (maxRecords != null) {
			try {
				maxRecordsAsInt = Integer.parseInt(maxRecords);
				if (maxRecordsAsInt < 0) {
					// trigger exception
					throw new IllegalArgumentException();
				}
			} catch (Exception e) {
				throw new IllegalArgumentException("Parameter 'max_records' has to be numeric and not negative!");
			}
		}
		
		if (description != null) {
			description = description.trim();
		}
		
		boolean includePlaneDataAsBoolean = Boolean.parseBoolean(includePlaneData);
		
		return DataSetDataProvider.getDataSets(offsetAsInt, maxRecordsAsInt, description, includePlaneDataAsBoolean);
	}

	
	@Path("/meta-info")
	@Description("Shows the Tissue Stack Data Resource's Meta Info.")
	public RestfulResource getDataResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}
}
