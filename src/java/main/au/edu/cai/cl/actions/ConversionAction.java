package au.edu.cai.cl.actions;

import java.net.URL;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import au.edu.cai.cl.TissueStackCLCommunicator;
import au.edu.cai.cl.json.JsonParser;


public class ConversionAction implements ClAction {
	private String option = null;
	private String list_option = "ALL";
	private String session = null;
	private String task_id = null;
	private String in_file = null;
	private String out_file = null;

	public boolean setMandatoryParameters(String[] args) {
		if (args.length < 2 ) return false;
		this.session = args[0];
		this.option = args[1].toUpperCase();
		if (this.option.equals("LIST")) {
			if (args.length > 2) {
				this.list_option = args[2].toUpperCase();
				if (!list_option.equals("DONE") && !list_option.equals("CANCELLED")
						&& !list_option.equals("ERROR") && !list_option.equals("ACTIVE"))
					return false;
			}
			return true;
		}
		if (this.option.equals("START")) {
			if (args.length < 4) return false;
			this.in_file =  args[2];
			this.out_file =  args[3];
			return true;
		}
		if (this.option.equals("CANCEL")) {
			if (args.length < 3) return false;
			this.task_id =  args[2];
			return true;
		}
		if (this.option.equals("STATUS")) {
			if (args.length < 3) return false;
			this.task_id =  args[2];
			return true;
		}

		return false;
	}

	public boolean needsSession() {
		return true;
	}

	public String getRequestUrl() {

		if (this.option.equals("LIST"))
			return "/server/?service=services&sub_service=metadata&session=" + this.session
					+ "&action=list_tasks&type=conversion&status=" + this.list_option;

		if (this.option.equals("START"))
			return "/server/?service=conversion&session=" + this.session + "&file=" + this.in_file
				+ "&new_raw_file=" + this.out_file;

		if (this.option.equals("CANCEL"))
			return "/server/?service=services&sub_service=admin&action=cancel&session=" +
					this.session + "&task_id=" + this.task_id;

		if (this.option.equals("STATUS"))
			return "/server/?service=services&sub_service=metadata&action=task_status&session=" +
					this.session + "&type=conversion&task_id=" + this.task_id;

		return null;
	}

	public ClActionResult performAction(final URL TissueStackServerURL) {
		String response = null;
		try {
			final URL combinedURL = new URL(TissueStackServerURL.toString() + this.getRequestUrl());
			response = TissueStackCLCommunicator.sendHttpRequest(combinedURL, null);

			final JSONObject parseResponse = JsonParser.parseResponse(response);
			if (parseResponse.get("response") != null) { // success it seems
				try {
					if (this.option.equals("LIST")) {
						JSONArray respObj = (JSONArray) parseResponse.get("response");
						if (respObj.isEmpty())
							return new ClActionResult(ClAction.STATUS.SUCCESS, "empty");

						StringBuilder formattedResponse = new StringBuilder();

						@SuppressWarnings("unchecked")
						final JSONObject [] tasks = (JSONObject[]) respObj.toArray(new JSONObject[]{});
						for (JSONObject task : tasks) {
							formattedResponse.append("\n\tTASK ID:\t" + task.get("task_id"));
							formattedResponse.append("\n\tSTATUS:\t\t" + task.get("status"));
						}
						return new ClActionResult(ClAction.STATUS.SUCCESS, formattedResponse.toString());
					}
					else if (this.option.equals("START")) {
						return new ClActionResult(ClAction.STATUS.SUCCESS,
							"Conversion started. Task id: " + (String) parseResponse.get("response"));
					}
					else if (this.option.equals("CANCEL")) {
						final JSONObject respObj = (JSONObject) parseResponse.get("response");
						if (respObj.containsKey("filename") && respObj.containsKey("progress"))
							return new ClActionResult(ClAction.STATUS.SUCCESS,
								"Conversion of '" + respObj.get("filename") + "' canceled at: " +
									respObj.get("progress") + "%");
						else
							return new ClActionResult(ClAction.STATUS.SUCCESS, "Cancellation of task failed!");
					} else if (this.option.equals("STATUS")) {
						final JSONObject respObj = (JSONObject) parseResponse.get("response");
						if (respObj.containsKey("status")) {
							StringBuilder formattedResponse = new StringBuilder();
							formattedResponse.append("\n\tTASK_ID:\t" + respObj.get("task_id"));
							formattedResponse.append("\n\tFILE:\t\t" + respObj.get("filename"));
							formattedResponse.append("\n\tSTATUS:\t\t" + respObj.get("status"));
							if (respObj.containsKey("progress"))
								formattedResponse.append("\n\tPROGRESS:\t" + respObj.get("progress") + "%");

							return new ClActionResult(ClAction.STATUS.SUCCESS, formattedResponse.toString());
						} else
							return new ClActionResult(ClAction.STATUS.SUCCESS, "Failed to query status!");
					} else
						return new ClActionResult(ClAction.STATUS.ERROR, "unkown option!");
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

			} else  // potential error
				return new ClActionResult(ClAction.STATUS.SUCCESS, JsonParser.parseError(parseResponse, response));
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
	}

	public String getUsage() {
		return "--convert start file_to_be_converted raw_file_name <== start conversion\n" +
				"\t--convert list [status (active|done|error|cancelled)] <== lists conversion tasks\n" +
			   "\t--convert status task_id <== displays status information on conversion task\n" +
			   "\t--convert cancel task_id <== cancel a conversion task";
	}

}
