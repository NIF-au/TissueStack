package au.edu.cai.cl.actions;

import java.net.URL;
import java.util.Iterator;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;


public class ListDataSetAction implements ClAction {

	public boolean setMandatoryParameters(String[] args) {
		// TODO: implement checks
		return true;
	}

	public ClActionResult performAction(final URL TissueStackServerURL) {
		String response = null;
		StringBuilder formattedResponse = new StringBuilder();
		try {
			response = TissueStackCLCommunicator.sendHttpRequest(
				TissueStackServerURL,
				"/server/?service=services&sub_service=admin&action=data_set_raw_files",
				null);
			
			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				JSONArray respObj = (JSONArray) parseResponse.get("response");
				@SuppressWarnings("unchecked")
				final JSONObject [] datasets = (JSONObject[]) respObj.toArray(new JSONObject[]{});
				formattedResponse.append("\tID\t");
				for (JSONObject dataset : datasets) {
					formattedResponse.append(dataset + "\n");
				}
				
			} else  // potential error
				formattedResponse = JsonParser.parseError(parseResponse, response);
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
		return new ClActionResult(ClAction.STATUS.SUCCESS, formattedResponse.toString());
	}

	public String getUsage() {
		return "--list dataset_id";
	}

}
