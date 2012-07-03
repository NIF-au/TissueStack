package au.edu.uq.cai.TissueStack;

import java.io.InputStream;
import java.util.Properties;

public final class TissueStackProperties {
	private static final String TISSUE_STACK_PROPERTIES = "TissueStack.properties";
	
	private static TissueStackProperties myself = null;

	private Properties props = new Properties();
	
	private TissueStackProperties() {
		// look for the file
		InputStream in = null;

		try {
			in = this.getClass().getResourceAsStream(TISSUE_STACK_PROPERTIES);
			props.load(in);
		} catch (Exception fileDoesNotExistOrCouldNotBeRead) {
			// fall back onto sys properties
			props = System.getProperties();
		} finally {
			// clean up
			try {
				in.close();
			} catch (Exception ignored) {
				// do nothing
			}
		}
	};
	
	public Object getProperty(String key) {
		if (key == null || key.trim().isEmpty()) {
			return null;
		}
		
		// look up key
		return this.props.get(key);
	}
	
	public static TissueStackProperties instance() {
		
		if (TissueStackProperties.myself == null) {
			TissueStackProperties.myself = new TissueStackProperties();
		}
		return TissueStackProperties.myself;
	}
}
