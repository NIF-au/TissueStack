package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;

public class DataSetImportAction implements ClAction {
	private String session = null; 
	private String filename = null; 
	
	public boolean setMandatoryParameters(String[] args) {
		if (args.length != 2 ) {
			System.out.println("--import needs a filename");
			return false;
		}
		this.session = args[0];
		this.filename = args[1];
		return true;
	}

	public boolean needsSession() {
		return true;
	}

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=admin&action=add_dataset&session=" +
			this.session + "&filename=" + this.filename;
	}
	
	public ClActionResult performAction(final URL TissueStackServerURL) {
		String response = null;
		StringBuilder formattedResponse = new StringBuilder();
		try {
			final URL combinedURL = new URL(TissueStackServerURL.toString() + this.getRequestUrl());
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				JSONObject respObj = (JSONObject) parseResponse.get("response");
				if (respObj.containsKey("id") && respObj.containsKey("filename")) {
					formattedResponse.append("\n\tIMPORTED:\t");
					formattedResponse.append(respObj.get("filename"));
					formattedResponse.append("\n\tID:\t\t");
					formattedResponse.append(respObj.get("id"));
					formattedResponse.append("\n");
				}
				return new ClActionResult(ClAction.STATUS.SUCCESS, formattedResponse.toString());
			} else  {// potential error
				formattedResponse = JsonParser.parseError(parseResponse, response);
				return new ClActionResult(ClAction.STATUS.ERROR, formattedResponse.toString());
			}
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

	public String getUsage() {
		return "--import filename";
	}

}
