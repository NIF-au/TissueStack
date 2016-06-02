package au.edu.cai.cl.json;

import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

public final class JsonParser {
	public static JSONObject parseResponse(final String response) {
		Object responseObject = null;
		
		try {
			responseObject = JSONValue.parse(response);
		} catch (Exception e) {
			// rethrow
			throw new RuntimeException("Failed to parse json response!", e);
		}
		
		if (responseObject == null) 
			throw new RuntimeException("Json Parse gave us a null object!");
		else if (responseObject instanceof JSONObject) 
			return (JSONObject) responseObject;
		else
			throw new RuntimeException("Json Parse didn't give us the expected json object!");
	}
	
	public static String parseError(JSONObject parseResponse, String response) {
		if (parseResponse == null)
			return null;

		final StringBuilder formattedResponse = new StringBuilder();
		if (parseResponse.get("error") != null) {
			formattedResponse.append("Error: " );

			JSONObject respObj = (JSONObject) parseResponse.get("error");

			String cause = "unknown";
			if (respObj.get("description") != null)
				cause = (String) respObj.get("description");
			else if (respObj.get("exception") != null)
				cause = (String) respObj.get("exception");
			
			formattedResponse.append(cause);
		} else formattedResponse.append("Unexpected Response:\n" + response); 

		return formattedResponse.toString();
	}
}
