package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;


public class ListDataSetAction implements ClAction {
	
	public boolean setMandatoryParameters(String[] args) {
		return true;
	}

	public boolean needsSession() {
		return false;
	}

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=data&action=all&include_planes=false";
	}
	
	public ClActionResult performAction(final URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(TissueStackServerURL.toString() + this.getRequestUrl());
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			StringBuilder formattedResponse = new StringBuilder();
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				try {
					JSONArray respObj = (JSONArray) parseResponse.get("response");
					
					@SuppressWarnings("unchecked")
					final JSONObject [] datasets = (JSONObject[]) respObj.toArray(new JSONObject[]{});
					for (JSONObject dataset : datasets) {
						formattedResponse.append("\n\tID:\t\t");
						formattedResponse.append(dataset.get("id"));
						formattedResponse.append("\n");
						formattedResponse.append("\tFILENAME:\t");
						formattedResponse.append(dataset.get("filename"));
						formattedResponse.append("\n");
						if (dataset.containsKey("description")) {
							formattedResponse.append("\tDESCRIPTION:\t");
							formattedResponse.append(dataset.get("description"));
							formattedResponse.append("\n");
						}
					}
				} catch(Exception parseError) {
					if (parseError instanceof ClassCastException) {
						JSONObject respObj = (JSONObject) parseResponse.get("response");
						if (respObj.containsKey("noResults"))
							return new ClActionResult(ClAction.STATUS.SUCCESS, (String) respObj.get("noResults"));
						else
							return new ClActionResult(ClAction.STATUS.ERROR, parseError.toString());
					} else
						return new ClActionResult(ClAction.STATUS.ERROR, parseError.toString());						
				}
				return new ClActionResult(ClAction.STATUS.SUCCESS, formattedResponse.toString());
			} else  // potential error
				return new ClActionResult(ClAction.STATUS.SUCCESS, JsonParser.parseError(parseResponse, response));	
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

	public String getUsage() {
		return "--list <== lists all datasets, for details use --query";
	}

}
