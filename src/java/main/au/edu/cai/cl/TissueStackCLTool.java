package au.edu.cai.cl;

import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import au.edu.cai.cl.actions.ClAction;
import au.edu.cai.cl.actions.ClAction.STATUS;
import au.edu.cai.cl.actions.ClActionResult;
import au.edu.cai.cl.actions.ConfigAction;
import au.edu.cai.cl.actions.DataSetImportAction;
import au.edu.cai.cl.actions.ListDataSetAction;
import au.edu.cai.cl.actions.LoginAction;

public class TissueStackCLTool {
	
	private static final Map<String, ClAction> clOpts;
	static {
        Map<String, ClAction> aMap = new LinkedHashMap<String, ClAction>();
        aMap.put("config", new ConfigAction());
        aMap.put("list", new ListDataSetAction());
        aMap.put("login", new LoginAction());
        aMap.put("import", new DataSetImportAction());
        clOpts = Collections.unmodifiableMap(aMap);
	}
	
	private static String assembleUsage() {
		StringBuffer usage = new StringBuffer("Available options:\n\t--v [verbose] \n"); 
		for (String opt : TissueStackCLTool.clOpts.keySet())
			usage.append("\t" + TissueStackCLTool.clOpts.get(opt).getUsage() + "\n");
		return usage.toString();
	}	
	
	private static String performExplicitLogin(final URL tissueStackServerUrl, final String password) {
		final LoginAction login = new LoginAction();
		login.setMandatoryParameters(new String[] {password});
		final ClActionResult result = login.performAction(tissueStackServerUrl);
		if (result.getStatus() != STATUS.SUCCESS) return null;

		return result.getResponse();
	}

	private static Map<String, List<String>> initParamsMap(String[] args) {
		if (args.length == 0) {
			System.out.println(TissueStackCLTool.assembleUsage());
			System.exit(0);
		}
		
		Map<String, List<String>> paramsMap = new HashMap<String, List<String>>();
		String action = null;
		for (String arg : args) {
			String lowerCaseArg = arg.trim().toLowerCase();
			if (lowerCaseArg.startsWith("--")) {
				action = lowerCaseArg.substring(2); 
				paramsMap.put(action, new ArrayList<String>());
			}
			else if (lowerCaseArg.startsWith("-")) {
				action = lowerCaseArg.substring(1);
				paramsMap.put(action, new ArrayList<String>());
			} else if (action != null) {
				List<String> params = paramsMap.get(action);
				if (params != null) 
					params.add(arg);
			}
		}
		
		return paramsMap;
	}
	
	public static void main(String[] args) {
		final Map<String, List<String>> paramsMap = TissueStackCLTool.initParamsMap(args);
		
		boolean verbose = false;
		final List<String> verbosity = paramsMap.get("v");
		if (verbosity != null) {
			verbose = true;
			paramsMap.remove("v");
		}
		
		// there is an order => we can have a config at any rate
		// so extract that one first
		// in the end we need a tissuestack server at a minimum
		String tissueStackInstance = null;
		String sessionToken = null;
		final List<String> configParams = paramsMap.get("config");
		if (configParams != null) {
			if (verbose) System.out.print("Reading handed in config ..." );

			final ClAction configAction = TissueStackCLTool.clOpts.get("config");
			if (configAction.setMandatoryParameters(configParams.toArray(new String[] {})) &&
					configAction.performAction(null).getStatus() == ClAction.STATUS.SUCCESS) {
				tissueStackInstance = TissueStackCLConfig.instance().get("tissuestack.instance");
				sessionToken = TissueStackCLConfig.instance().get("tissuestack.last_session_token");
						
				if (verbose) System.out.println("read.");
			} 
			if (verbose) System.out.println("");
			
			// remove config action from map
			paramsMap.remove("config");
		} else {
			if (verbose) System.out.println("Using default config file ..." );
			tissueStackInstance = TissueStackCLConfig.instance().get("tissuestack.instance");
			sessionToken = TissueStackCLConfig.instance().get("tissuestack.last_session_token");
		}
		
		// do we have a --server parameter
		final List<String> serverParams = paramsMap.get("server");
		if (serverParams != null && !serverParams.isEmpty()) {
			tissueStackInstance = serverParams.get(0);
			if (verbose) System.out.println("Overriding TissueStack instance with server value: " + tissueStackInstance);
			// remove server param from map
			paramsMap.remove("server");
		}

		// at this point we need to have the url for a tissuestack instance, otherwise we are screwed
		if (tissueStackInstance == null) {
			System.err.println(
				"Please provide tissue stack server via the --config OR --server parameter!");
			System.exit(0);
		}

		// check url
		URL tissueStackServerUrl = null; 
		try {
			tissueStackServerUrl = new URL(tissueStackInstance);
		} catch(Exception malformedUrl) {
			System.err.println("The given URL to the TissueStack instance seems to be incorrect: " + malformedUrl);
		}
		if (tissueStackServerUrl == null)
			System.exit(-1);

		// do we have a --login parameter
		final List<String> loginParams = paramsMap.get("login");
		if (loginParams != null) {
			if (!loginParams.isEmpty()) { 
				sessionToken = TissueStackCLTool.performExplicitLogin(tissueStackServerUrl, loginParams.get(0)) ;
				// remove server param from map
				paramsMap.remove("login");
				if (sessionToken != null) {
					System.out.println("\n\tSESSION:\t" + sessionToken + "\n");
					TissueStackCLConfig.instance().setProperty("tissuestack.last_session_token", sessionToken);
					TissueStackCLConfig.instance().saveConfig();
				} else {
					System.err.println("Login failed!");
					System.exit(-1);
				}
			} else {
				System.err.println("Provide a password after --login!");
				System.exit(-1);
			}
		}
		
		while (!paramsMap.isEmpty()) { // while we have actions handed in, execute them
			String action = paramsMap.keySet().iterator().next();
			final List<String> actionParams = paramsMap.get(action);
			paramsMap.remove(action);
			final ClAction actionChosen = TissueStackCLTool.clOpts.get(action);

			if (actionChosen == null) {
				System.out.println(TissueStackCLTool.assembleUsage());
				System.exit(0);
			}

			// action requires login => do so
			if (actionChosen.needsSession()) {
				if (sessionToken == null || sessionToken.trim().isEmpty()) {
					System.out.println("Action '" + action + "' requires a session!");
					System.out.println("Please type in the admin password: ");
					char [] password = System.console().readPassword();
					sessionToken = TissueStackCLTool.performExplicitLogin(tissueStackServerUrl, new String(password));
					if (sessionToken == null) {
						System.err.println("Login failed");
						System.exit(-1);
					}
					
					TissueStackCLConfig.instance().setProperty("tissuestack.last_session_token", sessionToken);
					TissueStackCLConfig.instance().saveConfig();
				}
				// inject session token
				actionParams.add(0, sessionToken);
			}
			
			// check action params
			if (verbose) System.out.print("\nChecking handed in parameters for action '" + action + "' ..." );
			final boolean areParamsOk = actionChosen.setMandatoryParameters(actionParams.toArray(new String[] {}));
			if (!areParamsOk)  {
				System.err.println("Usage: " + actionChosen.getUsage());
				System.exit(-1);
			}
			
			if (verbose) System.out.println(" OK");
			
			// perform action
			if (verbose) System.out.println("Requesting '" + actionChosen.getRequestUrl() + "' ...");
			final ClActionResult result = actionChosen.performAction(tissueStackServerUrl);
			
			// display action results
			if (result.getStatus() == STATUS.UNDEFINED)
				System.err.println("\nTissue Stack Response has status 'UNDEFINED': " + result.getResponse());
			else 
				System.out.println("\n" + result.getResponse());
		}
	}
}