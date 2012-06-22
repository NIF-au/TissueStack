package au.edu.uq.cai.TissueStack.dataobjects;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@Entity
@Table(name="session")
@XmlRootElement(name="Session", namespace=IGlobalConstants.XML_NAMESPACE)
public class Session {
	private String id;
	private long expiry;
	
	@Id
	@Column(name="id")
	@XmlElement(name="Id", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getId() {
		return id;
	}
	
	public void setId(String id) {
		this.id = id;
	}
	
	@Column(name="expiry")
	@XmlElement(name="Expiry", namespace=IGlobalConstants.XML_NAMESPACE)
	public long getExpiry() {
		return expiry;
	}
	
	public void setExpiry(long expiry) {
		this.expiry = expiry;
	}
}
