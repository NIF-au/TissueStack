/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
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
