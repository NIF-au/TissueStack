package au.edu.cai.cl.actions;

import java.net.URL;

import au.edu.cai.cl.TissueStackCLCommunicator;

public class ListDataSetAction implements ClAction {

	public boolean setMandatoryParameters(String[] args) {
		// TODO: implement checks
		return true;
	}

	public ClActionResult performAction(final URL TissueStackServerURL) {
		String response = null;
		try {
			response = TissueStackCLCommunicator.sendHttpRequest(
				TissueStackServerURL,
				"/server/?service=services&sub_service=admin&action=data_set_raw_files",
				null);
		} catch(Exception any) {
			return new ClActionResult(ClAction.STATUS.ERROR, any.toString());
		}
		return new ClActionResult(ClAction.STATUS.SUCCESS, response);
	}

	public String getUsage() {
		return "--list dataset_id";
	}

}
