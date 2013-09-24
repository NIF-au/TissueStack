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

import java.io.File;

import javax.persistence.EntityManager;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetValuesLookupTable;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetValuesLookupProvider;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;

@Path("/lookup")
@Description("DataSet Lookup Tables")
public final class LookupResources extends AbstractRestfulMetaInformation {
	@Path("/")
	public RestfulResource getDefault() {
		return this.getLookupResourcesMetaInfo();
	}

	@Path("/{id}")
	@Description("Returns the data set's lookup table info as long as it exists")
	public RestfulResource getDataSetLookupTableByIdDefault(
			@Description("Mandatory Paramter 'id': points towards the data sets id in the database")
			@PathParam("id") String id) {
		final DataSetValuesLookupTable dataSetLookupTable = this.getDataSetLookupTableByIdInternal(id);
		if (dataSetLookupTable == null) {
			return new RestfulResource(new Response(new NoResults()));
		}
		return new RestfulResource(new Response(dataSetLookupTable));
	}

	private DataSetValuesLookupTable getDataSetLookupTableByIdInternal(String id) {
		long idAsLong = -1;
		if (id == null) {
			throw new IllegalArgumentException("Parameter 'id' is mandatory!");
		}
		try {
			idAsLong = Long.parseLong(id);
		} catch (Exception e) {
			throw new IllegalArgumentException("Parameter 'id' is not numeric!");
		}
		
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			return em.find(DataSetValuesLookupTable.class, idAsLong);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

	@Path("/content")
	@GET
	@Produces(MediaType.TEXT_PLAIN)
	@Description("Reads lookup table content from the configuration's lookup file")
	public String getDataSetLookupTableContent(
			@Description("Mandatory Paramter 'file': lookup table file name")
			@QueryParam("file") String file) {
		if (file == null || !new File(file).exists() || !new File(file).canRead())
			throw new IllegalArgumentException("File param is either null or refers to a non-existing/non-readable file!");
		
		DataSetValuesLookupTable dataSetLookupTable = new DataSetValuesLookupTable();
		dataSetLookupTable.setFilename(file);
		
		Object[] lookupData = DataSetValuesLookupProvider.readLookupContentFromFile(dataSetLookupTable); 
		if (lookupData == null || lookupData[0] == null)
			return "No Content";

		dataSetLookupTable = (DataSetValuesLookupTable) lookupData[0];
		
		return dataSetLookupTable.getContent();
	}

	@Path("/meta-info")
	@Description("Shows the Tissue Stack Lookup Resource's Meta Info.")
	public RestfulResource getLookupResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}
}
