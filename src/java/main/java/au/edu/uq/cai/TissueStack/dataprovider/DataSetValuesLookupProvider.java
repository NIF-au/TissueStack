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
import java.util.NoSuchElementException;
import java.util.StringTokenizer;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.DataSetValuesLookupTable;

public final class DataSetValuesLookupProvider {
	final static Logger logger = Logger.getLogger(DataSetValuesLookupProvider.class); 

	public static DataSetValuesLookupTable readLookupContentFromFile (DataSetValuesLookupTable lookupTable) {
		if (lookupTable == null || lookupTable.getFilename() == null) { 
			logger.error("No file name provided for lookup!");
			return null;
		}
		
		final File lookupFile = new File(lookupTable.getFilename());
		if (!lookupFile.exists() || !lookupFile.canRead()) {
			logger.error("Lookup file '" + lookupFile.getAbsolutePath() + "' does not exist or cannot be read!");
			return null;
		}
		
		BufferedReader reader = null; 
		
		try {
			reader = new BufferedReader(new FileReader(lookupFile));
			
			String line = null;
			int lineNumber = 1;
			StringBuffer json = new StringBuffer((int)lookupFile.length());
			
			while ((line = reader.readLine()) != null) {
				StringTokenizer tokenizer = new StringTokenizer(line, "\t", false);
				int tokenIndex = 0;

				int indexes[] = new int[3]; 

				while (tokenIndex < 5) {
					try {
						String token = tokenizer.nextToken();
						token = token.trim();
								
						switch (tokenIndex) {
							case 0:
								break; // we skip that one, just autoincrement
							case 1: // red value of RGB
								indexes[tokenIndex-1] = Integer.parseInt(token);
								break;
							case 2: // green value of RGB
								indexes[tokenIndex-1] = Integer.parseInt(token);
								break;
							case 3:  // blue value of RGB
								indexes[tokenIndex-1] = Integer.parseInt(token);
								break;
							case 4:
								// that's the lookup value and store it
								//json.append(lineNumber + " => " + ((double)(indexes[0] + indexes[1] + indexes[2]) / 3) + "\n");
								//0.21 R + 0.71 G + 0.07 B
								json.append(lineNumber + " => " +
										(((double)indexes[0]*0.21 + (double)indexes[1]*0.71 + (double)indexes[2]*0.07)) + "\n");
						}
						tokenIndex++;
					} catch(NoSuchElementException noMoreTokens) {
						logger.error("Line " + lineNumber + " in Lookup file '" + lookupFile.getAbsolutePath()
								+ "' does not have the required minumum of 5 columns");
						return null;
					} 
					
					
				}
				lineNumber++;
			}
			lookupTable.setContent(json.toString());
			
			return lookupTable; 
		} catch (Exception any) {
			logger.error("Failed to read lookup file '" + lookupFile.getAbsolutePath() + "'!");
		}
		
		return null;
	}
}
