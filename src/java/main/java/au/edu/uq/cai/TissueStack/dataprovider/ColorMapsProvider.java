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
package au.edu.uq.cai.TissueStack.dataprovider;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.ColorMap;
import au.edu.uq.cai.TissueStack.dataobjects.Configuration;

public final class ColorMapsProvider {
	final static Logger logger = Logger.getLogger(ColorMapsProvider.class); 

	private static final String DEFAULT_COLORMAPS_DIRECTORY = "/opt/tissuestack/colormaps";	
	

	private static ColorMapsProvider myself;
	
	private Map<String, ColorMap> colorMaps;
	
	private ColorMapsProvider() {
		final File colorMapsDirectory = this.getColorMapsDirectory();
		if (!colorMapsDirectory.isDirectory() || !colorMapsDirectory.exists() || !colorMapsDirectory.canRead()) {
			logger.error("Color Maps Directory is not a directory, does not exist or cannot be read");
			return;
		}
		
		final String[] colorMapFIles = colorMapsDirectory.list();
		if (colorMapFIles.length == 0) {
			logger.warn("Color Maps Directory is empty");
			return;
		}
		
		this.colorMaps = new HashMap<String, ColorMap>(colorMapFIles.length);
		for (String file : colorMapFIles) {
			final ColorMap map = this.readColorMapFromFile(colorMapsDirectory.getAbsolutePath(), file);
			if (map != null) this.colorMaps.put(map.getName(), map);
		}
	}

	private File getColorMapsDirectory() {
		final Configuration colorMapsDir = ConfigurationDataProvider.queryConfigurationById("colormaps_directory");
		return new File(colorMapsDir == null || colorMapsDir.getValue() == null ? DEFAULT_COLORMAPS_DIRECTORY : colorMapsDir.getValue());
	}

	private ColorMap readColorMapFromFile (String path, String file) {
		if (path == null || file == null || file.trim().isEmpty()) {
			logger.error("ColorMap: File or path parameter is null or empty");
			return null;
		}
		
		final File colorMapFile = new File(path, file);
		if (!colorMapFile.exists() || !colorMapFile.canRead()) {
			logger.error("Color Map file '" + colorMapFile.getAbsolutePath() + "' does not exist or cannot be read!");
			return null;
		}
		
		ColorMap colorMap = new ColorMap();
		colorMap.setName(colorMapFile.getName());
		colorMap.setFile(colorMapFile.getAbsolutePath());
		
		BufferedReader reader = null; 
		
		try {
			reader = new BufferedReader(new FileReader(colorMapFile));
			
			String line = null;
			int lineNumber = 1;
			StringBuffer json = new StringBuffer((int)colorMapFile.length());
			json.append("\"" + colorMap.getName() + "\" : [");
			
			while ((line = reader.readLine()) != null) {
				StringTokenizer tokenizer = new StringTokenizer(line, " ", false);
				int tokenIndex = 0;

				double columns[] = new double[4]; 

				while (tokenIndex < 4) {
					try {
						String token = tokenizer.nextToken();
						token = token.trim();
								
						switch (tokenIndex) {
							case 0: // gray
								columns[tokenIndex] = Double.parseDouble(token);
								break;
							case 1: // red value of RGB
								columns[tokenIndex] = Double.parseDouble(token);
								break;
							case 2: // green value of RGB
								columns[tokenIndex] = Double.parseDouble(token);
								break;
							case 3:  // blue value of RGB
								columns[tokenIndex] = Double.parseDouble(token);
								
								// add to json
								json.append("[");
								json.append(columns[0]);
								json.append(",");
								json.append(columns[1]);
								json.append(",");
								json.append(columns[2]);
								json.append(",");
								json.append(columns[3]);
								json.append("],");
						}
						tokenIndex++;
					} catch(NoSuchElementException noMoreTokens) {
						logger.error("Line " + lineNumber + " in ColorMap file '" + colorMapFile.getAbsolutePath()
								+ "' does not contain the required 4 columns");
						return null;
					} 
				}
				lineNumber++;
			}
			if (json.charAt(json.length()-1) == ',')
				json.deleteCharAt(json.length()-1);
			
			json.append("]");
			colorMap.setJson(json.toString());
			
			return colorMap; 
		} catch (Exception any) {
			logger.error("Failed to read lookup file '" + colorMapFile.getAbsolutePath() + "'!", any);
		}
		
		return null;
	}
	
	public ColorMap getColorMap(String name) {
		return this.colorMaps.get(name);
	}

	public Collection<ColorMap> getColorMaps() {
		return this.colorMaps.values();
	}

	public String getColorMapsAsJson() {
		Collection<ColorMap> allColorMaps = this.getColorMaps();
		if (allColorMaps.isEmpty()) return null;
		
		final StringBuffer json = new StringBuffer(1000);
		json.append("{");
		for (ColorMap map : this.getColorMaps()) {
			json.append(map.getJson());
			json.append(",");
		}
		if (json.charAt(json.length()-1) == ',')
			json.deleteCharAt(json.length()-1);
		json.append("}");
		
		return json.toString();
	}

	public static ColorMapsProvider instance() {
		if (ColorMapsProvider.myself == null)
			ColorMapsProvider.myself = new ColorMapsProvider();
		
		return ColorMapsProvider.myself;
	}
}
