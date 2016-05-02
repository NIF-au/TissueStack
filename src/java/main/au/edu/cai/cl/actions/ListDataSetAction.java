package au.edu.cai.cl.actions;

import au.edu.cai.cl.TissueStackCLConfig;

public class ListDataSetAction implements ClAction {

	public boolean setMandatoryParameters(String[] args) {
		// TODO: implement checks
		return true;
	}

	public ClActionResult performAction() {
		// TODO implement sending request and retrieving response
		final TissueStackCLConfig config = TissueStackCLConfig.instance();

		return new ClActionResult(ClAction.STATUS.SUCCESS, "some list of items");
	}

	public String getUsage() {
		return "--list dataset_id";
	}

}
