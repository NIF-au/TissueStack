package au.edu.uq.cai.TissueStack.resources;

import java.util.ArrayList;
import java.util.List;

import javax.servlet.ServletContext;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;

import org.jboss.resteasy.annotations.providers.jaxb.Wrapped;

import au.edu.uq.cai.TissueStack.dataobjects.IGlobalConstants;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataobjects.RestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;

@Path("/")
public final class RootResource extends AbstractRestfulMetaInformation {
	private static RestfulMetaInformation[] metaInfoForServices;

	@SuppressWarnings("unchecked")
	private void initRootService(ServletContext servletContext) {
		if (metaInfoForServices ==	 null) {
			try {
				final List<String> resources = (List<String>) servletContext.getAttribute("resource_classes");
				final List<RestfulMetaInformation> services = 
					new ArrayList<RestfulMetaInformation>(resources.size());
				
				for (String resource : resources) {
					final AbstractRestfulMetaInformation metaInfoForService = 
						(AbstractRestfulMetaInformation)Class.forName(resource).newInstance();
					
					if (metaInfoForService instanceof RootResource) { // skip the meta info service itself
						continue;
					}
					services.add(metaInfoForService.getMetaInfo());
				}
				
				metaInfoForServices = services.toArray(new RestfulMetaInformation[]{});
			} catch (Exception any) {
				throw new RuntimeException("failed to create meta info...", any);
			}
		}
	}
	

	@GET
	@Path("/")
	@Produces(MediaType.TEXT_PLAIN)
	public String getDefault(@Context ServletContext servletContext) {
		this.initRootService(servletContext);
		return this.getServiceOverviewAsPlainText(servletContext);
	}

	@GET
	@Path("meta-info/xml")
	@Produces(MediaType.APPLICATION_XML)
	@Wrapped(element="RestfulServicesMetaInformationSummary", namespace=IGlobalConstants.XML_NAMESPACE)
	public RestfulMetaInformation[] getServiceOverviewAsXML(@Context ServletContext servletContext) {
		this.initRootService(servletContext);
		return metaInfoForServices;
	}
	
	@GET
	@Path("meta-info/json")
	@Produces(MediaType.APPLICATION_JSON)
	public Response getServiceOverviewAsJson(@Context ServletContext servletContext) {
		this.initRootService(servletContext);
		return new Response(metaInfoForServices);
	}

	@GET
	@Path("meta-info/text")
	@Produces(MediaType.TEXT_PLAIN)
	public String getServiceOverviewAsPlainText(@Context ServletContext servletContext) {
		this.initRootService(servletContext);
		final StringBuilder response = new StringBuilder("");
		for (RestfulMetaInformation metaInfo : metaInfoForServices) {
			response.append(metaInfo.toString());
			response.append("\n\n");
		}
		return response.toString();
	}
}
