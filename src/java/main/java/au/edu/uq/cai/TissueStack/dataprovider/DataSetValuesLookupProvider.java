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
import java.util.List;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;

import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;
import javax.persistence.Query;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.ColorMap;
import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetValuesLookupTable;

public final class DataSetValuesLookupProvider {
	final static Logger logger = Logger.getLogger(DataSetValuesLookupProvider.class); 

	private static final String DEFAULT_LOOKUP_TABLES_DIRECTORY = "/opt/tissuestack/lookup";	
	
	private static File getLookupTablesDirectory() {
		final Configuration lookupTablesDir = ConfigurationDataProvider.queryConfigurationById("lookup_tables_directory");
		return new File(lookupTablesDir == null || lookupTablesDir.getValue() == null ? DEFAULT_LOOKUP_TABLES_DIRECTORY : lookupTablesDir.getValue());
	}

	public static Object[] readLookupContentFromFile (DataSetValuesLookupTable lookupTable) {
		if (lookupTable == null || lookupTable.getFilename() == null) { 
			logger.error("No file name provided for lookup!");
			return null;
		}
		
		final File lookupFile = new File(lookupTable.getFilename());
		if (!lookupFile.exists() || !lookupFile.canRead()) {
			logger.error("Lookup file '" + lookupFile.getAbsolutePath() + "' does not exist or cannot be read!");
			return null;
		}
		
		// we also associate a color map to the lookup data
		final ColorMap colorLookup = new ColorMap();
		colorLookup.setFile(lookupFile.getAbsolutePath());
		// a colormap with this name exists already, we introduce the hack/convention that we add _lookup to the name
		colorLookup.setName(
				ColorMapsProvider.instance().containsColorMap(lookupFile.getName()) ?
						lookupFile.getName() + "_lookup" : lookupFile.getName());
		
		BufferedReader reader = null; 
		
		try {
			reader = new BufferedReader(new FileReader(lookupFile));
			
			String line = null;
			int lineNumber = 1;
			StringBuffer json = new StringBuffer((int)lookupFile.length());
			StringBuffer colorJson = new StringBuffer(json.capacity());
			
			json.append("{");
			colorJson.append("\"" + colorLookup.getName() + "\": [");
			
			while ((line = reader.readLine()) != null) {
				StringTokenizer tokenizer = new StringTokenizer(line, "\t", false);
				int tokenIndex = 0;

				int indexes[] = new int[4]; 

				while (tokenIndex < 5) {
					try {
						String token = tokenizer.nextToken();
						token = token.trim();
								
						switch (tokenIndex) {
							case 0:
								// turns out this is the associated grayscale value, let's remeber it as well
								indexes[tokenIndex] = Integer.parseInt(token);
								break; 
								
							case 1: // red value of RGB
								indexes[tokenIndex] = Integer.parseInt(token);
								break;
							case 2: // green value of RGB
								indexes[tokenIndex] = Integer.parseInt(token);
								break;
							case 3:  // blue value of RGB
								indexes[tokenIndex] = Integer.parseInt(token);
								break;
							case 4:
								// store both grayscale and rgb lookup value, provided they are different...
								if (indexes[0]*3 != (indexes[1] + indexes[2] + indexes[3])) {
									json.append("\"" + indexes[0] + "/" + indexes[0] + "/" + indexes[0] + "\":");
									json.append("\"" + token + "\",");
								};
								json.append("\"" + indexes[1] + "/" + indexes[2] + "/" + indexes[3] + "\":");
								json.append("\"" + token + "\",");
								// also: build associated color map
								colorJson.append("[\"L\"," + indexes[0] + "," + indexes[1] + "," + indexes[2] + "," + indexes[3] + "],"); 
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
			if (json.charAt(json.length()-1) == ',')
				json.deleteCharAt(json.length()-1);
			json.append("}");
			if (colorJson.charAt(colorJson.length()-1) == ',')
				colorJson.deleteCharAt(colorJson.length()-1);
			colorJson.append("]");
			lookupTable.setContent(json.toString());
			
			final Object[] ret = new Object[] {lookupTable, null};
			if (colorJson.length() > 1) {
				colorLookup.setJson(colorJson.toString());
				ret[1] = colorLookup;
			}
			
			return ret; 
		} catch (Exception any) {
			logger.error("Failed to read lookup file '" + lookupFile.getAbsolutePath() + "'!");
		} finally {
			try {
				reader.close();
			} catch(Exception any) {
				// ignored
			}
		}
		
		return null;
	}
	
	private static boolean readAndPersistAllLookupFiles() {
		final File lookupDirectory = DataSetValuesLookupProvider.getLookupTablesDirectory();
		if (!lookupDirectory.exists() || !lookupDirectory.isDirectory() ||  !lookupDirectory.canRead()) {
			logger.error("Lookup directory does not exist or is not readable or is not a directory");
			return false;
		}
		
		final String[] lookupFiles = lookupDirectory.list();
		if (lookupFiles.length == 0) return false;
		
		EntityManager em = null; 
		
		try {
			em = JPAUtils.instance().getEntityManager(); 

			for (String file : lookupFiles) {
				final String fullFileName = new File(lookupDirectory, file).getAbsolutePath();
				
				// first check whether we have an existing record 
				DataSetValuesLookupTable row = 
						DataSetValuesLookupProvider.findDataSetValuesLookupTableByUniqueFileName(
								fullFileName);
				if (row != null) continue;
				
				// start transaction
				final EntityTransaction updateLookupRecord = em.getTransaction();	
				updateLookupRecord.begin();
				
				row = new DataSetValuesLookupTable();
				row.setFilename(fullFileName);
				em.persist(row);
				updateLookupRecord.commit();
			}
			
			return true;
		} catch (Exception any){
			logger.error("Failed to add lookup file to the table!");
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}

		return false;		
	}
	
	public static DataSetValuesLookupTable findDataSetValuesLookupTableByUniqueFileName(String filename) {
		if (filename == null) return null;
		EntityManager em = null; 

		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			Query query = em.createQuery("SELECT lookup FROM DataSetValuesLookupTable AS lookup WHERE lookup.filename LIKE :filename");
			query.setParameter("filename", filename);
			
			@SuppressWarnings("unchecked" )
			final List<DataSetValuesLookupTable> results = query.getResultList();
			
			// there should be only 1 or none
			if (results.isEmpty()) return null;
			
			return results.get(0);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static void initDataBase() {
		// read lookup directory and populate table with entries
		if (!DataSetValuesLookupProvider.readAndPersistAllLookupFiles()) 
			return;
		
		// we read all rows in the database and fill the content column with the contents of the actual file
		EntityManager em = null; 
		
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			Query query = em.createQuery("SELECT lookup FROM DataSetValuesLookupTable AS lookup");
			query.setMaxResults(5000);

			@SuppressWarnings("unchecked" )
			final List<DataSetValuesLookupTable> results = query.getResultList();
			for (DataSetValuesLookupTable look : results) {
				try {
					Object[] lookupData = DataSetValuesLookupProvider.readLookupContentFromFile(look);
					if (lookupData == null || lookupData[0] == null) continue; // error reading file
					
					look = (DataSetValuesLookupTable) lookupData[0];
					
					// if we have a colormap associated with the lookup, add it to the map
					if (lookupData[1] != null)
						ColorMapsProvider.instance().addColormap((ColorMap) lookupData[1]);
						
					// start persistance transaction
					final EntityTransaction updateLookupRecord = em.getTransaction();	
					updateLookupRecord.begin();
					em.merge(look);
					// commit 
					updateLookupRecord.commit();
				} catch (Exception e) {
					logger.error("Failed to read index file: " + look.getFilename(), e);
				}
			}
		} catch (Exception any){
			logger.error("Failed to initialize lookup table with index files!");
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
