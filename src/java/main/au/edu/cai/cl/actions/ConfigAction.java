package au.edu.cai.cl.actions;

import java.io.File;

import au.edu.cai.cl.TissueStackCLConfig;

public class ConfigAction implements ClAction {

	private File location = null;
	
	public boolean setMandatoryParameters(String[] args) {
		
		if (args.length < 2) {
			System.err.println("ERROR: --config needs a configuration file as its first parameter");
			return false;
		}
		final File newConfig = new File(args[1]);
		if (!newConfig.exists() || !newConfig.canRead()) {
			System.err.println("ERROR: --config file does not exist or cannot be read");
			return false;
		}
		this.location = newConfig;
		return true;
	}

	public ClActionResult performAction() {
		final TissueStackCLConfig config = 
			TissueStackCLConfig.instance(this.location.getAbsolutePath());
		if (config.getException() == null)
			return new ClActionResult(ClAction.STATUS.SUCCESS, "{ config: " + this.location.getAbsolutePath() + "}");

		return new ClActionResult(ClAction.STATUS.ERROR, "{ error: " + config.getException().getMessage());
	}

	public String getUsage() {
		return "--config file_location";
	}

}
