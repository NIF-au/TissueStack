package au.edu.uq.cai.TissueStack.dataobjects;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@Entity
@Table(name="configuration")
@XmlRootElement(name="Configuration", namespace=IGlobalConstants.XML_NAMESPACE)
public class Configuration {
	private String name;
	private String value;
	private String description;
	
	@Id
	@Column(name="name")
	@XmlElement(name="Name", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	@Column(name="value")
	@XmlElement(name="Value", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getValue() {
		return value;
	}
	
	public void setValue(String value) {
		this.value = value;
	}
	
	@Column(name="description")
	@XmlElement(name="Description", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getDescription() {
		return description;
	}
	
	public void setDescription(String description) {
		this.description = description;
	}
}
