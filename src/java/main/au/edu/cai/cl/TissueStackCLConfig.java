package au.edu.cai.cl;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;

public final class TissueStackCLConfig {
	private static final String DEFAULT_CONFIG = 
		System.getProperty("user.home") + File.separatorChar + "tissuestack_config";
	private static TissueStackCLConfig myself;
	
	private File config_file = null;
	private Properties configuration = null;
	private Exception exceptionLoadingConfig = null;
	
	private TissueStackCLConfig(String location) {
		if (location == null || location.isEmpty())
			location = TissueStackCLConfig.DEFAULT_CONFIG;
		this.config_file = new File(location);
		this.configuration = new Properties();
		if (this.config_file.exists() && this.config_file.canRead()) {
			InputStream in = null;
			try {
				in = new FileInputStream(this.config_file);
		        this.configuration.load(in);
			} catch(Exception ignored) {
		    	// nothing we can do but make a note
				this.exceptionLoadingConfig = ignored;
		    } finally {
				try {
					in.close();
				} catch (Exception e) {}
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
		if (key == null || value == null) return;
		this.configuration.setProperty(key, value);
	}
	
	public String get(final String key) {
		return this.configuration.getProperty(key);
	}

	public void saveConfig() {
		if (!this.config_file.canWrite()) {
			System.err.println("Cannot write to configuration file!");
			return;
		}
	
		OutputStream out = null;
		try {
			out = new FileOutputStream(this.config_file);
	        this.configuration.store(out, null);
		} catch(Exception ignored) {
			System.err.println("Failed to store changes to configuration file!");
	    } finally {
	    	try {
	    		out.close();
			} catch (Exception e) {}
	    }
	}

}
