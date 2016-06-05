package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;


public class ConfigurationAction implements ClAction {
	private String option = null;
	private String session = null;
	private String key = null;
	private String value = null;
	
	public boolean setMandatoryParameters(String[] args) {
		if (args.length < 2 ) return false;
		this.session = args[0];
		this.option = args[1].toUpperCase();
		if (this.option.equals("LIST")) return true;
		if (this.option.equals("QUERY")) {
			if (args.length < 3) return false;
			this.key =  args[2];
			return true;
		}
		if (this.option.equals("CHANGE")) {
			if (args.length < 4) return false;
			this.key =  args[2];
			this.value =  args[3];
			return true;
		}		
		return false;
	}

	public boolean needsSession() {
		return true;
	}

	public String getRequestUrl() {
		if (this.option.equals("LIST"))
			return "/server/?service=services&sub_service=configuration&action=all";
		if (this.option.equals("QUERY"))
			return "/server/?service=services&sub_service=configuration&action=query&key=" + this.key;
		if (this.option.equals("CHANGE")) 
			return "/server/?service=services&sub_service=configuration&action=change&session=" +
					this.session + "&key=" + this.key + "&value=" + 
					TissueStackCLCommunicator.encodeQueryStringParam(this.value);
		return null;
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
					final JSONObject [] configTriples = (JSONObject[]) respObj.toArray(new JSONObject[]{});
					for (JSONObject configTriple : configTriples) {
						formattedResponse.append("\n\tKEY:\t\t" + configTriple.get("name"));
						formattedResponse.append("\n\tVALUE:\t\t" + configTriple.get("value"));
						if (configTriple.containsKey("description")) {
							formattedResponse.append("\n\tDESCRIPTION:\t");
							formattedResponse.append(configTriple.get("description"));
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
		return "--config list <== lists configuration table contents\n" +
			   "\t--config query key <== displays a given configuration table value\n" +
			   "\t--config change key value <== changes value for a configuration table entry";
	}

}
