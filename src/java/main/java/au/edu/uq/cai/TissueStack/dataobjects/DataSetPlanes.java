package au.edu.uq.cai.TissueStack.dataobjects;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name="dataset_planes")
public class DataSetPlanes{
	  private int id;
	  private int dataset_id;
	  private String name;
	  private int max_x; 
	  private int max_y; 
	  private int max_sclices; 
	  private int zoom_levels; 
	  private int one_to_one_zoom_level; 
	  private String transformation_matrix;

	@Id
	@Column(name="id")
	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}
	
	@Column(name="dataset_id")
	public int getDataset_id() {
		return dataset_id;
	}
	public void setDataset_id(int dataset_id) {
		this.dataset_id = dataset_id;
	}
	@Column(name="name")
	public String getName() {
		return name;
	}
	public void setName(String name) {
		this.name = name;
	}
	@Column(name="max_x")
	public int getMax_x() {
		return max_x;
	}
	public void setMax_x(int max_x) {
		this.max_x = max_x;
	}
	@Column(name="max_y")
	public int getMax_y() {
		return max_y;
	}
	public void setMax_y(int max_y) {
		this.max_y = max_y;
	}
	@Column(name="max_sclices")
	public int getMax_sclices() {
		return max_sclices;
	}
	public void setMax_sclices(int max_sclices) {
		this.max_sclices = max_sclices;
	}
	@Column(name="zoom_levels")
	public int getZoom_levels() {
		return zoom_levels;
	}
	public void setZoom_levels(int zoom_levels) {
		this.zoom_levels = zoom_levels;
	}
	@Column(name="one_to_one_zoom_level")
	public int getOne_to_one_zoom_level() {
		return one_to_one_zoom_level;
	}
	public void setOne_to_one_zoom_level(int one_to_one_zoom_level) {
		this.one_to_one_zoom_level = one_to_one_zoom_level;
	}
	@Column(name="transformation_matrix")
	public String getTransformation_matrix() {
		return transformation_matrix;
	}
	public void setTransformation_matrix(String transformation_matrix) {
		this.transformation_matrix = transformation_matrix;
	}
}

