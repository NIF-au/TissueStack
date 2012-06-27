package au.edu.uq.cai.TissueStack.resources;

import java.rmi.server.UID;
import java.security.NoSuchAlgorithmException;

import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataobjects.Session;
import au.edu.uq.cai.TissueStack.dataprovider.SessionDataProvider;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;
import au.edu.uq.cai.TissueStack.security.SHA2encoder;

/*
 * A VERY minimal security model for now:
 * 
 * -) For admin "tasks" we have a global password stored as a SHA 2 hash
 * -) A user knowing the plain text password will be able to retrieve a session which is stored in a database table
 * 
 * Notes:
 * ------
 * -) The session token is not associated with a user. The system does not support logins!
 * -) The session token generation and access validation routines do not integrate environment cross-checks such as IP, user-agent, etc.
 *    to make exchange of still active tokens less likely
 * -) The SESSION_TIMEOUT (interval of inactivity) gets pushed out with every admin task called
 */
@Path("/security")
@Description("Tissue Stack Minimal Security Resources")
public final class SecurityResources extends AbstractRestfulMetaInformation {
	
	final static Logger logger = Logger.getLogger(SecurityResources.class);
	
	// the global admin password
	private static final String GLOBAL_ADMIN_PASSWORD_AS_SHA_2_HEX_STRING = 
			"101ee9fe7aceaa8bea949e75a529d796da02e08bced78c6c4dde60768183fa14";
	// the global timeout for a session in millis
	private static final long SESSION_TIMEOUT = 1000 * 60 * 5; // 5 minutes of inactivity
	
	@Path("/")
	public RestfulResource getDefault() {
		return this.getSecurityResourcesMetaInfo();
	}

	@Path("/new_session")
	@Description("Returns a new session, provided the given password matches the the global admin password")
	public RestfulResource getNewSession(@QueryParam("password") String password) throws NoSuchAlgorithmException {
		// first off, check admin password 
		if (!this.checkAdminPassword(password)) {
			// return no results for non matching passwords
			return new RestfulResource(new Response(new NoResults()));
		}

		// generate a new session token (use rmi.UID) 
		final UID uid = new UID();
		
		final Session newSession = new Session();
		newSession.setId(uid.toString());
		newSession.setExpiry(System.currentTimeMillis() + SESSION_TIMEOUT);
		
		// store it in the database
		SessionDataProvider.persistSession(newSession);
		
		// return the session token
		return new RestfulResource(new Response(newSession));
	}

	@Path("/invalidate_session")
	@Description("Invalidates a session, provided the session exists")
	public RestfulResource invalidateSession(
			@QueryParam("session") String session) {
		if (session == null || session.trim().isEmpty()) {
			throw new IllegalArgumentException("session token has to be non-empty!");
		}
		
		// delete session in database
		int rowsDeleted = SessionDataProvider.invalidateSession(session);
		
		if (rowsDeleted == 1) {
			// return success message
			return new RestfulResource(new Response("Session invalidated"));
		}
		
		// return success message
		return new RestfulResource(new Response(new NoResults()));
	}

	private boolean checkAdminPassword(String password) throws NoSuchAlgorithmException {
		final byte hashedPasswordAsBytes[] = SHA2encoder.encode(password);
		if (hashedPasswordAsBytes == null) {
			throw new IllegalArgumentException("Given password has to be a non-empty string!");
		}
		final String hashedPasswordAsHex = SHA2encoder.convertByteArrayToHexString(hashedPasswordAsBytes);
		if (hashedPasswordAsHex == null) {
			throw new IllegalArgumentException("Could not convert sha2 bytes to hex string");
		}
		if (hashedPasswordAsHex.equals(GLOBAL_ADMIN_PASSWORD_AS_SHA_2_HEX_STRING)) {
			return true;
		}
		
		return false;
	}

	/*
	 * Checks existence of session token in the data base and pushes out expiry date at the same time 
	 */
	public static boolean checkSession(String session) {
		if (session == null || session.trim().isEmpty()) {
			return false;
		}

		// check for valid session
		final Session sessionFound = SessionDataProvider.querySessionById(session);
		if (sessionFound == null || sessionFound.getExpiry() < System.currentTimeMillis()) {
			return false;
		}
		
		// push out expiry date if we are still valid
		try {
			if (SessionDataProvider.extendSessionExpiry(sessionFound.getId(), SESSION_TIMEOUT) != 1) {
				logger.warn("Could not extend session expiry for session " + session);
			}
		} catch (Exception e) {
			// log error 
			logger.warn("Could not extend session expiry for session " + session, e);
		}
		
		return true;
	}

	@Path("/meta-info")
	@Description("Shows the Tissue Stack Security's Meta Info.")
	public RestfulResource getSecurityResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
