package au.edu.uq.cai.TissueStack.dataobjects;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlTransient;

@Entity
@Table(name="dataset_values_lookup")
@XmlRootElement(name="DataSetValuesLookupTable", namespace=IGlobalConstants.XML_NAMESPACE)
public class DataSetValuesLookupTable {
	private long id;
	private String filename; 
	private String content;
	  

	@Id
 	@Column(name="id")
	@XmlTransient
	public long getId() {
		return id;
	}
	public void setId(long id) {
		this.id = id;
	}
	
	@Column(name="filename")
	@XmlElement(name="FileName", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getFilename() {
		return filename;
	}
	
	public void setFilename(String filename) {
		this.filename = filename;
	}
	
	@Column(name="content")
	@XmlElement(name="Content", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getContent() {
		return this.content;
	}
	
	public void setContent(String content) {
		this.content = content;
	} 
}
