package au.edu.cai.cl;

import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
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
		
		// look for action specifier
		String action = args[0].trim().toLowerCase();
		if (action.startsWith("--"))
			action = action.substring(2);
		if (action.startsWith("-1"))
			action = action.substring(1);
		
		final ClAction actionChosen = TissueStackCLTool.clOpts.get(action);
		if (actionChosen == null) {
			System.out.println(TissueStackCLTool.assembleUsage());
			System.exit(0);
		}
		
		final boolean areParamsOk = actionChosen.setMandatoryParameters(args);
		if (!areParamsOk) System.exit(-1);
		
		final ClActionResult result = actionChosen.performAction();
		System.out.println("Status: " + result.getStatus());
		System.out.println("Response: " + result.getResponse());
		
	}
}