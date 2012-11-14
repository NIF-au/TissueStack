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

import java.io.File;
import java.io.FileWriter;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;

import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;

import com.sun.org.apache.xerces.internal.parsers.DOMParser;
import com.sun.org.apache.xml.internal.serialize.XMLSerializer;


public final class AndsDataSetRegistration {
	
	public static final String ANDS_DATASET_XML_KEY = "ands_harvest";
	public static final String ANDS_DATASET_XML_DEFAULT = "/opt/tissuestack/ands/datasets.xml";
	
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
			if (!andsDataSetXML.exists() || !andsDataSetXML.canWrite()) 
				throw new RuntimeException(
						"Ands DataSet XML: File " + andsDataSetXML.getAbsolutePath() + " does not exist or is read-only!"); 
			try {
				AndsDataSetRegistration.myself = new AndsDataSetRegistration(andsDataSetXML);
			} catch (Exception e) {
				throw new RuntimeException("Ands DataSet XML: Failed to parse document: " + andsDataSetXML.getAbsolutePath() + "!", e);
			}
		}
		return AndsDataSetRegistration.myself;
	}

	// TODO: adapt to "true" xml structure
	// TODO: perhaps check for duplicates (if we can that is)
	public void registerDataSet(long id) {
		// get data set node
		NodeList dataSetNodes = this.andsDataSetXML.getElementsByTagName("DataSets");
		if (dataSetNodes == null || dataSetNodes.getLength() > 1) 
			throw new RuntimeException("Ands DataSet XML: Failed to add new data set. Reason: There has to be 1 and only 1 Data Set Element");
		Node dataSetNode = dataSetNodes.item(0);
		
		Element newDataSet = this.andsDataSetXML.createElement("DataSet");
		newDataSet.appendChild(this.andsDataSetXML.createTextNode("Test " + System.currentTimeMillis()));
		dataSetNode.appendChild(newDataSet);
		
		try {
			// write back changes
			XMLSerializer serializer = new XMLSerializer();
			serializer.setOutputCharStream(new FileWriter(AndsDataSetRegistration.getAndsDataSetXML()));
			serializer.serialize(this.andsDataSetXML);
		} catch (Exception e) {
			throw new RuntimeException("Ands DataSet XML: Failed to persist new data set!", e);
		}
	}
}
