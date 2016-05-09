package au.edu.cai.cl.actions;

import java.net.URL;

public interface ClAction {
	enum STATUS {
		UNDEFINED,
		SUCCESS,
		PENDING,
		ERROR
	}
	
	boolean setMandatoryParameters(String[] args);
	String getUsage();

	ClActionResult performAction(final URL TissueStTissueStackServerURL);
}
