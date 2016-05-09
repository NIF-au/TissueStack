package au.edu.cai.cl;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.Properties;

public final class TissueStackCLConfig {
	private static final String DEFAULT_CONFIG = 
		System.getProperty("user.home") + File.separatorChar + "tissuestack_config";
	private static TissueStackCLConfig myself;
	
	private Properties configuration = null;
	private Exception exceptionLoadingConfig = null;
	
	private TissueStackCLConfig(String location) {
		if (location == null || location.isEmpty())
			location = TissueStackCLConfig.DEFAULT_CONFIG;
		File newConfig = new File(location);
		this.configuration = new Properties();
		if (newConfig.exists() && newConfig.canRead()) {
			try {
				InputStream in = new FileInputStream(newConfig);
		        this.configuration.load(in);
		        in.close();
			} catch(Exception ignored) {
		    	// nothing we can do but make a note
				this.exceptionLoadingConfig = ignored;
		    }
		}
	};

	public static TissueStackCLConfig instance(String location) {
		if (TissueStackCLConfig.myself == null)
			TissueStackCLConfig.myself = new TissueStackCLConfig(location);
		return TissueStackCLConfig.myself;

	}

	public static TissueStackCLConfig instance() {
		return TissueStackCLConfig.instance(null);
	}
	
	public Exception getException() {
		return this.exceptionLoadingConfig;
	}
	
	public void setProperty(final String key, final String value) {
		this.configuration.setProperty(key, value);
	}
	
	public String get(final String key) {
		return this.configuration.getProperty(key);
	}

}
