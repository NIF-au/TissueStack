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
package au.edu.uq.cai.TissueStack.ands;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.net.URI;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;

import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;

import com.sun.org.apache.xerces.internal.parsers.DOMParser;
import com.sun.org.apache.xml.internal.serialize.XMLSerializer;


public final class AndsDataSetRegistration {
	
	public static final String ANDS_DATASET_XML_KEY = "ands_harvest";
	public static final String ANDS_DATASET_XML_DEFAULT = "/opt/tissuestack/ands/datasets.xml";
	public static final String MINIMAL_ANDS_DATASET_XML_CONTENT = 
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			+ "<registryObjects xmlns=\"http://ands.org.au/standards/rif-cs/registryObjects\""
			+ " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
			+ " xsi:schemaLocation=\"http://ands.org.au/standards/rif-cs/registryObjects"
			+ " http://services.ands.org.au/documentation/rifcs/schema/registryObjects.xsd\">\n"
			+ "</registryObjects>";
	
	final Logger  logger = Logger.getLogger(AndsDataSetRegistration.class);
	// singleton
	private static AndsDataSetRegistration myself;
	// class members
	private Document andsDataSetXML;
	
	private AndsDataSetRegistration(File dataSetXML) throws Exception {
	   final DOMParser parser = new DOMParser();
	   parser.parse(dataSetXML.getAbsolutePath());
	   this.andsDataSetXML = parser.getDocument();
	}
	
	public static File getAndsDataSetXML() {
		final Configuration andsDataSetXL = ConfigurationDataProvider.queryConfigurationById("ands_dataset_xml");
		return new File(andsDataSetXL == null || andsDataSetXL.getValue() == null ? ANDS_DATASET_XML_DEFAULT : andsDataSetXL.getValue());
	}

	public static AndsDataSetRegistration instance() {
		if (AndsDataSetRegistration.myself == null) {
			final File andsDataSetXML = AndsDataSetRegistration.getAndsDataSetXML();
			
			try {
				if (!andsDataSetXML.exists()) 
						AndsDataSetRegistration.createMinimalAndsDataSetXML(andsDataSetXML);
			} catch (Exception e) {
				throw new RuntimeException("Failed to create ands data set template!", e);
			}
			
			if (!andsDataSetXML.canWrite()) 
				throw new RuntimeException(
						"Ands DataSet XML: File " + andsDataSetXML.getAbsolutePath() + " is not writable!"); 
			try {
				AndsDataSetRegistration.myself = new AndsDataSetRegistration(andsDataSetXML);
			} catch (Exception e) {
				throw new RuntimeException("Ands DataSet XML: Failed to parse document: " + andsDataSetXML.getAbsolutePath() + "!", e);
			}
		}
		return AndsDataSetRegistration.myself;
	}

	public void registerDataSet(long id, String name, String group, String location, String description) {
		// get data set node
		NodeList registryObjects = this.andsDataSetXML.getElementsByTagName("registryObjects");
		if (registryObjects == null || registryObjects.getLength() > 1) 
			throw new RuntimeException("Ands DataSet XML: Failed to add new data set. Reason: There has to be 1 and only 1 'registryObjects' root element");
		Node root = registryObjects.item(0);
		
		int latestCollectionNumber = 0;
		// find latest registry object for increment number
		if (root.hasChildNodes()) {
			NodeList registryObjectsList = root.getChildNodes();
			for (int i=0;i<registryObjectsList.getLength();i++) {
				Node regObject = registryObjectsList.item(i);
				if (!regObject.hasChildNodes())
					continue;

				// look for key element
				NodeList regObjectChildren = regObject.getChildNodes();
				for (int j=0;j<regObjectChildren.getLength();j++) {
					Node potentialKey = regObjectChildren.item(j);
					if (potentialKey.getNodeName().equalsIgnoreCase("key")) {
						String keyValue = potentialKey.getTextContent();
						int pos = -1;
						if (keyValue != null && (pos = keyValue.toLowerCase().lastIndexOf("collection-")) != -1) {
							try {
								latestCollectionNumber = Integer.parseInt(keyValue.substring(pos + "collection-".length()));
							} catch (NumberFormatException e) {
								// can be ignored
							}
							break;
						}
					}
				}
			}
		}
		// add 1 to the latest collection number
		latestCollectionNumber++;
		
		Element newDataSet = this.andsDataSetXML.createElement("registryObject");
		newDataSet.setAttribute("group", (group == null || group.trim().isEmpty()) ? "Anonymous" : group);
		Element newElement = this.andsDataSetXML.createElement("key");
		
		String keyValue = "collection-" + latestCollectionNumber;
		URI address = null;
		
		try {
			address = URI.create(location);
			keyValue = address.getHost() + "/" + keyValue;
		} catch (Exception e) {
			// can be ignored safely
		}
		
		// add the key element
		newElement.setTextContent(keyValue);
		newDataSet.appendChild(newElement);
		
		// add the originatingSource element
		newElement = this.andsDataSetXML.createElement("originatingSource");
		newElement.setTextContent("http://services.ands.org.au/home/orca/register_my_data");
		newDataSet.appendChild(newElement);

		// add the collection element
		Element newCollection = this.andsDataSetXML.createElement("collection");
		newCollection.setAttribute("type", "dataset");
		String modifiedDate = null;
		try {
			final Date now = new Date();
			SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd");
			modifiedDate = formatter.format(now);
			modifiedDate += "T";
			formatter = new SimpleDateFormat("kk:mm:ss");
			modifiedDate += formatter.format(now);
			modifiedDate += "Z";
		} catch (Exception e) {
			// can be ignored
		}
		
		if (modifiedDate != null) newCollection.setAttribute("dateModified", modifiedDate);
		newDataSet.appendChild(newCollection);

		// query the dataset
		final DataSet ds = DataSetDataProvider.queryDataSetById(id);
		
		// add the name element
		newElement = this.andsDataSetXML.createElement("name");
		newElement.setAttribute("type", "primary");
		Element subElement = this.andsDataSetXML.createElement("namePart");
		subElement.setTextContent((name == null || name.trim().isEmpty()) ? ds.getFilename() : name);
		newElement.appendChild(subElement);
		newCollection.appendChild(newElement);
		
		// add the location element
		if (location != null && !location.trim().isEmpty()) {
			newElement = this.andsDataSetXML.createElement("value");
			newElement.setTextContent(location);
			subElement = this.andsDataSetXML.createElement("electronic");
			subElement.setAttribute("type", "url");
			subElement.appendChild(newElement);
			newElement = this.andsDataSetXML.createElement("address");
			newElement.appendChild(subElement);
			subElement = this.andsDataSetXML.createElement("location");
			subElement.appendChild(newElement);
			newCollection.appendChild(subElement);
		}
		
		// add the description element
		newElement = this.andsDataSetXML.createElement("description");
		newElement.setAttribute("type", "full");
		newElement.setTextContent((description == null || description.trim().isEmpty()) ? ds.getDescription() : description);
		newCollection.appendChild(newElement);

		
		// add collection
		newDataSet.appendChild(newCollection);
		
		root.appendChild(newDataSet);
		
		try {
			// write back changes
			XMLSerializer serializer = new XMLSerializer();
			serializer.setOutputCharStream(new FileWriter(AndsDataSetRegistration.getAndsDataSetXML()));
			serializer.serialize(this.andsDataSetXML);
		} catch (Exception e) {
			throw new RuntimeException("Ands DataSet XML: Failed to persist new data set!", e);
		}
	}
	
	private static void createMinimalAndsDataSetXML(File file) throws IOException {
		final File parentDirectory = new File(file.getParent()); 
		if (parentDirectory != null && !parentDirectory.exists()) // create parent directory if it does not exist
			if (!parentDirectory.mkdirs())
				throw new RuntimeException("Could not create directory '" + parentDirectory.getAbsolutePath() + "'");
		
		// try to write a minimal ands data set registration file
		BufferedWriter writer = null; 
		try {
			writer = new BufferedWriter(new FileWriter(file));
			writer.write(MINIMAL_ANDS_DATASET_XML_CONTENT);
		} finally {
			try {
				writer.close();
			} catch (Exception e) {
				// we can ignore that safely
			}
		}
	}
}
