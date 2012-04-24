package au.edu.uq.cai.TissueStack.exceptions;

import org.jboss.resteasy.spi.ApplicationException;

public class RestfulApplicationException extends ApplicationException {

	private static final long serialVersionUID = -5385803989710375180L;

	public RestfulApplicationException(String s, Throwable throwable) {
		super(s, throwable);
	}

	public RestfulApplicationException(String s) {
		super(s, null);
	}

}
