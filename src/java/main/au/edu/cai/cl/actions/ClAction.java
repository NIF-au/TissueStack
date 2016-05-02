package au.edu.cai.cl.actions;


public interface ClAction {
	enum STATUS {
		UNDEFINED,
		SUCCESS,
		PENDING,
		ERROR
	}
	
	boolean setMandatoryParameters(String[] args);
	String getUsage();

	ClActionResult performAction();
}
