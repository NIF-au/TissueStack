package au.edu.uq.cai.TissueStack.exceptions;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import au.edu.uq.cai.TissueStack.dataobjects.ApplicationError;

/**
 * This class wraps a response in the format it was initially requested
 * e.g. somebody requesting a resource in json will receive the exception response in json      
 *
 */
public class ExceptionMapperResponseProducer {
	private Throwable wrappedException;
	private HttpServletRequest httpRequest;
	
	public ExceptionMapperResponseProducer(HttpServletRequest httpRequest, Throwable exception) {
		this.httpRequest = httpRequest;
		this.wrappedException = exception;
	}

	public Response produceResponse(String customErrorMessage) {
		final ApplicationError error = ApplicationError.createApplicationError(customErrorMessage, this.wrappedException);
		
		if (httpRequest == null) { // there is no other way than to return plain text
			return Response.ok(
					error.getDescription() + ": " + error.getException() + " (" + error.getMessage() + ")",
					MediaType.TEXT_PLAIN).build(); 
		}

		// we check the original url first for whether it contains clues on the requested output
		String requestedResourceURI = httpRequest.getPathInfo();
		if (requestedResourceURI != null) {
			requestedResourceURI = 
				requestedResourceURI.endsWith("/") ? requestedResourceURI.substring(0,requestedResourceURI.length() -1) : requestedResourceURI;
			requestedResourceURI = requestedResourceURI.toLowerCase();
			
			if (requestedResourceURI.endsWith("/xml")) { // xml was requested
				return Response.ok(error, MediaType.APPLICATION_XML).build();
			} else if (requestedResourceURI.endsWith("/json") || requestedResourceURI.endsWith("/geojson")) { // json or geojson was requested
				final au.edu.uq.cai.TissueStack.dataobjects.Response errorResponse =
					new au.edu.uq.cai.TissueStack.dataobjects.Response();
				errorResponse.setError(error);
				return Response.ok(errorResponse, MediaType.APPLICATION_JSON).build();
			}
		}
		
		String acceptHeader = httpRequest.getHeader("Accept");
		// nothing so far ... try the Accept header 
		if (acceptHeader != null) {
			acceptHeader = acceptHeader.toLowerCase();
			
			if (acceptHeader.indexOf("json") > 0) { // json in the accept header
				final au.edu.uq.cai.TissueStack.dataobjects.Response errorResponse =
					new au.edu.uq.cai.TissueStack.dataobjects.Response();
				errorResponse.setError(error);

				return Response.ok(errorResponse, MediaType.APPLICATION_JSON).build();
			} else if (acceptHeader.indexOf("xml") > 0) { // xml in the accept header
				return Response.ok(error, MediaType.APPLICATION_XML).build();
			}
		}

		// if all else fails ... back to plain text
		return Response.ok(
				error.getDescription() + ": " + error.getException() + " (" + error.getMessage() + ")",
				MediaType.TEXT_PLAIN).build(); 
	}
}

