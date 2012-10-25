package au.edu.uq.cai.TissueStack.dataobjects;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(name="Response", namespace=IGlobalConstants.XML_NAMESPACE)
public class Response implements IResponse {

	@XmlElement(name="ApplicationError", namespace=IGlobalConstants.XML_NAMESPACE)
	private ApplicationError applicationError;
	private Object response = new NoResults();

	public Response() {}
	
	public Response(Object response) {
		this.setResponse(response);
	}

	public Object getResponse() {
		return this.response;
	}

	public ApplicationError getError() {
		return this.applicationError;
	}

	public boolean hasErrorOccured() {
		if (this.getError() != null) {
			return true;
		}
		return false;
	}
	
	public void setResponse(Object response) {
		this.response = response;
	}

	public void setError(ApplicationError error) {
		this.applicationError = error;
	}
}
