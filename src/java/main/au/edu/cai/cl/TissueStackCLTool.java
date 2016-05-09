package au.edu.cai.cl;

import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import au.edu.cai.cl.actions.ClAction;
import au.edu.cai.cl.actions.ClActionResult;
import au.edu.cai.cl.actions.ConfigAction;
import au.edu.cai.cl.actions.ListDataSetAction;

public class TissueStackCLTool {
	
	private static final Map<String, ClAction> clOpts;
	static {
        Map<String, ClAction> aMap = new LinkedHashMap<String, ClAction>();
        aMap.put("config", new ConfigAction());
        aMap.put("list", new ListDataSetAction());
        clOpts = Collections.unmodifiableMap(aMap);
	}
	
	public static String assembleUsage() {
		StringBuffer usage = new StringBuffer("Available options:\n"); 
		for (String opt : TissueStackCLTool.clOpts.keySet())
			usage.append("\t" + TissueStackCLTool.clOpts.get(opt).getUsage() + "\n");
			
		return usage.toString();
	}	
	
	public static void main(String[] args) {
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
		
		// there is an order => we can have a config at any rate
		// so extract that one first
		// in the end we need a tissuestack server at a minimum
		String tissueStackInstance = null;
		final List<String> configParams = paramsMap.get("config");
		if (configParams != null) {
			System.out.print("Reading handed in config ..." );

			final ClAction configAction = TissueStackCLTool.clOpts.get("config");
			if (configAction.setMandatoryParameters(configParams.toArray(new String[] {})) &&
					configAction.performAction(null).getStatus() == ClAction.STATUS.SUCCESS) {
				tissueStackInstance = TissueStackCLConfig.instance().get("tissuestack.instance");
				System.out.println("read.");
			} 
			System.out.println("");
			
			// remove config action from map
			paramsMap.remove("config");
		} else {
			System.out.println("Using default config file ..." );
			tissueStackInstance = TissueStackCLConfig.instance().get("tissuestack.instance");
		}
		
		// do we have an --server parameter
		final List<String> serverParams = paramsMap.get("server");
		if (serverParams != null && !serverParams.isEmpty()) {
			tissueStackInstance = serverParams.get(0);
			System.out.println("Overriding TissueStack instance with server value: " + tissueStackInstance);
			// remove server param from map
			paramsMap.remove("server");
		}
		
		// at this point we need to have the url for a tissuestack instance, otherwise we are screwed
		if (tissueStackInstance == null) {
			System.err.println(
				"Please provide a --config parameter pointing to a configuration file OR a --server "
				+ " parameter giving the url for the TissueStack instance!");
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
		
		while (!paramsMap.isEmpty()) { // while we have actions handed in, execute them
			action = paramsMap.keySet().iterator().next();
			final List<String> actionParams = paramsMap.get(action);
			paramsMap.remove(action);
			final ClAction actionChosen = TissueStackCLTool.clOpts.get(action);

			if (actionChosen == null) {
				System.out.println(TissueStackCLTool.assembleUsage());
				System.exit(0);
			}

			// check action params
			System.out.println("\nChecking handed in parameters for action '" + action + "' ..." );
			final boolean areParamsOk = actionChosen.setMandatoryParameters(actionParams.toArray(new String[] {}));
			if (!areParamsOk) System.exit(-1);
			
			// perform action
			System.out.println("Executing action '" + action + "' @ " +  tissueStackInstance + " ...");
			final ClActionResult result = actionChosen.performAction(tissueStackServerUrl);
			
			// display action results
			System.out.println("\nStatus: " + result.getStatus());
			System.out.println("Response: " + result.getResponse());
		}
	}
}