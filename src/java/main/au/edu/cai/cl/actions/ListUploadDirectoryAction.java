package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;


public class ListUploadDirectoryAction implements ClAction {
	private boolean display_raw_only = false;
	
	public boolean setMandatoryParameters(String[] args) {
		if (args.length > 0) {
			this.display_raw_only = Boolean.parseBoolean(args[0]);
		}
		
		return true;
	}

	public boolean needsSession() {
		return false;
	}

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=admin&action=upload_directory&display_raw_only=" +
				this.display_raw_only;
	}
	
	public ClActionResult performAction(final URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(TissueStackServerURL.toString() + this.getRequestUrl());
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);
			StringBuilder formattedResponse = new StringBuilder();
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				JSONArray respObj = (JSONArray) parseResponse.get("response");
				
				if (respObj.isEmpty()) {
					return new ClActionResult(ClAction.STATUS.SUCCESS, "empty");
				}
				
				@SuppressWarnings("unchecked")
				final String [] files = (String[]) respObj.toArray(new String[]{});
				for (String file : files) {
					formattedResponse.append("\n\tFILE:\t");
					formattedResponse.append(file);
				}
				return new ClActionResult(ClAction.STATUS.SUCCESS, formattedResponse.toString());
			} else  // potential error
				return new ClActionResult(ClAction.STATUS.SUCCESS, JsonParser.parseError(parseResponse, response));	
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

	public String getUsage() {
		return "--upload-dir [raw-files-only (value: true/false)] <== lists contents of upload directory";
	}

}
