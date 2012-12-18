package au.edu.uq.cai.TissueStack.dataobjects;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(name="DataSetOverlay", namespace=IGlobalConstants.XML_NAMESPACE)
public class DataSetOverlay {
	private String name = "overlay";
	private String type;
	
	@XmlElement(name="Name", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		if (name == null || name.isEmpty())
			return;
		
		this.name = name;
	}
	
	@XmlElement(name="Type", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getType() {
		return type;
	}
	
	public void setType(String type) {
		this.type = type;
	}
}
