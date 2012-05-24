package au.edu.uq.cai.TissueStack.dataobjects;

import java.util.List;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.JoinColumn;
import javax.persistence.OneToMany;
import javax.persistence.SecondaryTable;
import javax.persistence.Table;
import javax.persistence.Id;


@Entity
@Table(name="dataset")
@SecondaryTable(name="dataset_planes")
public class DataSet {

	private int id; 
	private String filename; 
	private String description;
	private List<DataSetPlanes> planes;
	

	@Id
	@Column(name="id")
	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}
	
	@Column(name="filename")
	public String getFilename() {
		return filename;
	}
	public void setFilename(String filename) {
		this.filename = filename;
	}
	
	@Column(name="description")
	public String getDescription() {
		return description;
	}
	public void setDescription(String description) {
		this.description = description;
	} 
	
	@OneToMany
	@JoinColumn(name="dataset_id")
	@Column(name="dataset_id")
	public List<DataSetPlanes> getPlanes() {
		return planes;
	}
	public void setPlanes(List<DataSetPlanes> planes) {
		this.planes = planes;
	}
}



