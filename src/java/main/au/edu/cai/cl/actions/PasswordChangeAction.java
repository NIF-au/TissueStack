package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;

public class PasswordChangeAction implements ClAction {
	private String old_password=null;
	private String new_password=null;

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=security&action=passwd&old_passwd=";
	}

	public boolean needsSession() {
		return false;
	}

	public boolean setMandatoryParameters(String[] args) {
		if (args.length != 2) {
			return false;
		}
		this.old_password = args[0];
		this.new_password = args[1];
		return true;
	}

	public String getUsage() {
		return "--password old new <== changes the present admin password";
	}

	public ClActionResult performAction(URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(
				TissueStackServerURL.toString() + this.getRequestUrl() + 
					this.old_password + "&new_passwd=" + this.new_password);
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				String respObj = (String) parseResponse.get("response");
				return new ClActionResult(ClAction.STATUS.SUCCESS, respObj);
			} else 
				return new ClActionResult(ClAction.STATUS.SUCCESS, 
					JsonParser.parseError(parseResponse, response));
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

}
