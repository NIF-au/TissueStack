package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;

public class DataSetModifyAction implements ClAction {
	private String session=null;
	private long id=-1;
	private String column=null;
	private String value=null;

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=metadata&action=dataset_modify&session="
				+ this.session + "&id=" + this.id + "&column=" + this.column + "&value=" +
			TissueStackCLCommunicator.encodeQueryStringParam(this.value);
	}

	public boolean needsSession() {
		return true;
	}

	public boolean setMandatoryParameters(String[] args) {
		if (args.length < 4) return false;
		this.session = args[0];
		try {
			this.id = Long.parseLong(args[1]);
		} catch(Exception notnumeric) {
			System.err.println("ID is not numeric!");
			return false;
		}
		this.column = args[2];
		this.value = args[3];
		return true;
	}

	public String getUsage() {
		return "--modify dataset_id column value <== modifies dataset table";
	}

	public ClActionResult performAction(URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(
				TissueStackServerURL.toString() + this.getRequestUrl());
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems				
				String respObj = (String) parseResponse.get("response");
					return new ClActionResult(ClAction.STATUS.SUCCESS, respObj);
			} else  // potential error
				return new ClActionResult(ClAction.STATUS.SUCCESS, JsonParser.parseError(parseResponse, response));
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

}
