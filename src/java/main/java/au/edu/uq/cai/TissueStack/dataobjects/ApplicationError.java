package au.edu.uq.cai.TissueStack.dataobjects;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(namespace=IGlobalConstants.XML_NAMESPACE)
public class ApplicationError {
	private String description = "Unspecified Error";
	private String exception = "";
	private String message = "";

	private ApplicationError() {}
	
	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getDescription() {
		return this.description;
	}

	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getException() {
		return this.exception;
	}

	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getMessage() {
		return this.message;
	}

	public static ApplicationError createMinimalApplicationError() {
		return new ApplicationError();
	}

	public static ApplicationError createApplicationError(String description, Throwable exception) {
		final ApplicationError jsonError = ApplicationError.createMinimalApplicationError();
		if (description != null && description.trim().length()  > 0) {
			jsonError.description = description;
		}
		if (exception != null) {
			jsonError.exception = exception.getClass().getName();
			if (exception.getMessage() != null) {
				jsonError.message = exception.getMessage();
			}
		}
		
		return jsonError;
	}
}