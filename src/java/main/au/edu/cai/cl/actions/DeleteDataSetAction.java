package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;


public class DeleteDataSetAction implements ClAction {
	private String session = null;
	private long id = -1;
	private boolean confirmed = false;

	public boolean setMandatoryParameters(String[] args) {
		if (args.length < 2) return false;
		this.session = args[0];
		try {
			id = Long.parseLong(args[1]);
		} catch(Exception notnumeric) {
			System.err.println("ID is not numeric!");
			return false;
		}
		try {
			if (args.length > 2)
				this.confirmed = Boolean.parseBoolean(args[2]);
		} catch(Exception notboolean) {
			return false;
		}

		return true;
	}

	public boolean needsSession() {
		return true;
	}

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=admin&action=delete_dataset&session=" +
			this.session + "&id=" + this.id + "&delete_file=" + this.confirmed;
	}
	
	public ClActionResult performAction(final URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(TissueStackServerURL.toString() + this.getRequestUrl());
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				JSONObject respObj = (JSONObject) parseResponse.get("response");
				return new ClActionResult(ClAction.STATUS.SUCCESS, (String) respObj.get("result"));
			} else  // potential error
				return new ClActionResult(ClAction.STATUS.SUCCESS, JsonParser.parseError(parseResponse, response));	
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

	public String getUsage() {
		return "--delete [delete_file (value: true/false)] <== deletes dataset (configuration and optionally file)";
	}

}
