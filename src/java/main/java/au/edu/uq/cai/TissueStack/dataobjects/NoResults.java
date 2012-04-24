package au.edu.uq.cai.TissueStack.dataobjects;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(namespace=IGlobalConstants.XML_NAMESPACE)
public final class NoResults {
	private String noResults = "No results found"; 
	
	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getNoResults() {
		return this.noResults;
	}
}
