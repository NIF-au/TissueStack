package au.edu.cai.cl.actions;

import java.io.File;
import java.net.URL;

import au.edu.cai.cl.TissueStackCLConfig;

public class ConfigAction implements ClAction {

	private File location = null;
	
	public boolean needsSession() {
		return false;
	}

	public String getRequestUrl() {
		return "";
	}

	public boolean setMandatoryParameters(String[] args) {
		
		if (args.length < 1) {
			System.err.println("ERROR: --ts-conf needs a configuration file as its first parameter");
			return false;
		}
		final File newConfig = new File(args[0]);
		if (!newConfig.exists() || !newConfig.canRead()) {
			System.err.println("ERROR: --ts-conf file does not exist or cannot be read");
			return false;
		}
		this.location = newConfig;
		return true;
	}

	public ClActionResult performAction(final URL TissueStackServerURL) {
		final TissueStackCLConfig config = 
			TissueStackCLConfig.instance(this.location.getAbsolutePath());
		if (config.getException() == null)

			return new ClActionResult(ClAction.STATUS.SUCCESS, "{ config: " + this.location.getAbsolutePath() + "}");

		return new ClActionResult(ClAction.STATUS.ERROR, "{ error: " + config.getException().getMessage());
	}

	public String getUsage() {
		return "--ts-conf file_location <== uses config file to get connect/session info";
	}

}
