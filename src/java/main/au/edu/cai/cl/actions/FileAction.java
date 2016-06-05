package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;

public class FileAction implements ClAction {
	private String option=null;
	private String session=null;
	private String oldLocation=null;
	private String newLocation=null;

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=admin&action=" + 
				(this.option.equals("DELETE") ? "file_delete" : "file_rename") +
				"&session=" + this.session + "&file=" + this.oldLocation + 
				(this.option.equals("RENAME") ?
					("&new_file=" + this.newLocation) : "");
	}

	public boolean needsSession() {
		return true;
	}

	public boolean setMandatoryParameters(String[] args) {
		if (args.length < 2) return false;
		this.session = args[0];
		this.option = args[1].toUpperCase();
		if ((this.option.equals("DELETE") && args.length > 2) || 
			(this.option.equals("RENAME") && args.length > 3)) {
			this.oldLocation = args[2];
			if (args.length > 3) this.newLocation = args[3];
			return true;
		}

		return false;
	}

	public String getUsage() {
		return "--file delete file <== deletes a file on the server\n" +
				"\t--file rename old_location new_location <== renames a file on the server";
	}

	public ClActionResult performAction(URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(
				TissueStackServerURL.toString() + this.getRequestUrl());
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				JSONObject respObj = (JSONObject) parseResponse.get("response");
				if (respObj.containsKey("noResults"))
					return new ClActionResult(ClAction.STATUS.SUCCESS,
						(this.option.equals("DELETE") ? "file deleted" : "file renamed"));
				else 
					return new ClActionResult(ClAction.STATUS.ERROR, 
						(this.option.equals("DELETE") ? "file delete" : "file rename") + " failed");
			} else  // potential error
				return new ClActionResult(ClAction.STATUS.SUCCESS, JsonParser.parseError(parseResponse, response));
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

}
