package au.edu.cai.cl.actions;

import java.net.URL;

public interface ClAction {
	enum STATUS {
		UNDEFINED,
		SUCCESS,
		PENDING,
		ERROR
	}
	
	String getRequestUrl();
	boolean needsSession();
	boolean setMandatoryParameters(String[] args);
	String getUsage();

	ClActionResult performAction(final URL TissueStackServerURL);
}
