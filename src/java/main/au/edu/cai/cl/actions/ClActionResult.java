package au.edu.cai.cl.actions;

public class ClActionResult {
	private ClAction.STATUS status;
	private String response;
	
	public ClActionResult() {
		this(null);
	}

	public ClActionResult(ClAction.STATUS status) {
		this(status, null);
	}

	public ClActionResult(ClAction.STATUS status, String response) {
		if (status == null)
			status = ClAction.STATUS.UNDEFINED;
		
		if (response == null || response.isEmpty())
			response = "Response: N/A";
			
		this.status = status;
		this.response = response;
	}

	public ClAction.STATUS getStatus() {
		return status;
	}

	public void setStatus(ClAction.STATUS status) {
		this.status = status;
	}

	public String getResponse() {
		return response;
	}

	public void setResponse(String response) {
		this.response = response;
	}
}
