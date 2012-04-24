package au.edu.uq.cai.TissueStack.dataobjects;

public class Response implements IResponse {

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
