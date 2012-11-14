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
package au.edu.uq.cai.TissueStack.exceptions;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.Response;
import javax.ws.rs.ext.Provider;

import org.apache.log4j.Logger;
import org.jboss.resteasy.spi.ApplicationException;
import org.jboss.resteasy.spi.Failure;
import org.jboss.resteasy.spi.MethodNotAllowedException;
import org.jboss.resteasy.spi.NotFoundException;


@Provider
public class ExceptionMapper implements javax.ws.rs.ext.ExceptionMapper<Throwable> {

	private Logger logger = Logger.getLogger(ExceptionMapper.class);
	
	private @Context HttpServletRequest httpRequest;
	
	public Response toResponse(Throwable exception) {
		// first of all: log exception to not let it slip through ...
		logger.error("Exception:", exception);
		
		// wrap the exception for whatever format was requested
		final ExceptionMapperResponseProducer mediaTypeExceptionWrapper =
			new ExceptionMapperResponseProducer(this.httpRequest,exception); 
		
		if (exception instanceof NotFoundException) {
			return mediaTypeExceptionWrapper.produceResponse("Resource does not exist");
		} else if (exception instanceof MethodNotAllowedException) {
			return mediaTypeExceptionWrapper.produceResponse("Method not allowed");
		} else if (exception instanceof Failure) {
			return  mediaTypeExceptionWrapper.produceResponse("Failure");
		} else if (exception instanceof ApplicationException) {
			return  mediaTypeExceptionWrapper.produceResponse("Application Exception");
		}
		return Response.status(500).build();
	}

}
