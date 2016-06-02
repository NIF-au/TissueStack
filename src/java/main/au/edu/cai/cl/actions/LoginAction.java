package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;

public class LoginAction implements ClAction {
	private String password=null;

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=security&action=new_session";
	}

	public boolean needsSession() {
		return false;
	}

	public boolean setMandatoryParameters(String[] args) {
		if (args.length != 1 || args[0] == null) {
			System.err.println("Login requires a password parameter!");
			return false;
		}
		this.password = args[0];
		return true;
	}

	public String getUsage() {
		return "--login password <== logs you in returning a session";
	}

	public ClActionResult performAction(URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(
				TissueStackServerURL.toString() + this.getRequestUrl() + 
				"&password=" + this.password);
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				JSONObject respObj = (JSONObject) parseResponse.get("response");
				if (respObj.containsKey("expiry"))
					return new ClActionResult(ClAction.STATUS.SUCCESS, (String) respObj.get("id"));
				
			}
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
		return new ClActionResult(ClAction.STATUS.ERROR, "Login Failed!");
	}

}
