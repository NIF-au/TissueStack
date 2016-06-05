package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;


public class QueryDataSetAction implements ClAction {
	private long id = -1;
	private boolean detailed = true;
	public boolean setMandatoryParameters(String[] args) {
		if (args.length < 1 ) return false;
		try {
			this.id = Long.parseLong(args[0]);
		} catch(Exception notnumeric) {
			System.err.println("ID is not numeric!");
			return false;
		}
		try {
			if (args.length > 1)
				this.detailed = Boolean.parseBoolean(args[1]);
		} catch(Exception notboolean) {
			return false;
		}
		return true;
	}

	public boolean needsSession() {
		return false;
	}

	public String getRequestUrl() {
		return "/server/?service=services&sub_service=data&action=query&id=" + 
			this.id + "&include_planes=" + this.detailed;
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
						formattedResponse.append("\tID:\t\t");
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
						// detailed
						if (this.detailed && dataset.containsKey("planes")) {
							JSONArray planeData = (JSONArray) dataset.get("planes");
							@SuppressWarnings("unchecked")
							final JSONObject [] planes = (JSONObject[]) planeData.toArray(new JSONObject[]{});
							if (planes.length == 0) continue;
							StringBuilder planeInfo = new StringBuilder("\n");
							boolean commonInfo = false;
							for (JSONObject plane : planes) {
								if (!commonInfo) {
									formattedResponse.append("\tIS_TILED:\t");
									formattedResponse.append(plane.get("isTiled"));
									formattedResponse.append("\n\tZOOM_LEVELS:\t");
									formattedResponse.append(plane.get("zoomLevels"));
									formattedResponse.append("\n\tONE_TO_ONE_ZOOM_LEVEL:\t");
									formattedResponse.append(plane.get("oneToOneZoomLevel"));
									if (plane.containsKey("resolutionMm") &&
											((double) plane.get("resolutionMm")) != 0) {
											formattedResponse.append("\n\tRESOLUTION_MM:\t");
											formattedResponse.append(plane.get("resolutionMm"));
									}
									commonInfo = true;
								}
								planeInfo.append("\n\tPLANE:\t" + plane.get("name"));
								planeInfo.append("\n\t[X/Y]:\t" 
									+ plane.get("maxX") + " x " + plane.get("maxY"));
								planeInfo.append("\n\tSLICES:\t" + plane.get("maxSlices"));
								if (plane.containsKey("transformationMatrix"))
									planeInfo.append("\n\tMATRIX:\t" + plane.get("transformationMatrix"));
							}
							formattedResponse.append(planeInfo.toString());
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
		return "--query id [detailed (value: true/false)] <== lists details for an individual dataset";
	}

}
