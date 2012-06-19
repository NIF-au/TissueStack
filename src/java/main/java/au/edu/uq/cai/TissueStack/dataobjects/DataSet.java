package au.edu.uq.cai.TissueStack.dataobjects;

import java.util.List;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.FetchType;
import javax.persistence.JoinColumn;
import javax.persistence.OneToMany;
import javax.persistence.Table;
import javax.persistence.Id;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@Entity
@Table(name="dataset")
@XmlRootElement(name="DataSet", namespace=IGlobalConstants.XML_NAMESPACE)
public class DataSet {

	private long id; 
	private String filename; 
	private String description;
	
	private List<DataSetPlanes> planes;

	@Id
	@Column(name="id")
	@XmlElement(name="Id", namespace=IGlobalConstants.XML_NAMESPACE)
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
	
	@Column(name="description")
	@XmlElement(name="Description", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getDescription() {
		return description;
	}
	
	public void setDescription(String description) {
		this.description = description;
	} 
	
	@OneToMany(fetch=FetchType.LAZY)
	@JoinColumn(name="dataset_id")
	@Column(name="dataset_id")
	public List<DataSetPlanes> getPlanes() {
		return planes;
	}
	public void setPlanes(List<DataSetPlanes> planes) {
		this.planes = planes;
	}
}



