package au.edu.uq.cai.TissueStack.dataobjects;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.persistence.Transient;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlTransient;

@Entity
@Table(name="dataset_planes")
@XmlRootElement(name="DataSetPlanes", namespace=IGlobalConstants.XML_NAMESPACE)
public class DataSetPlanes{
	  private long id;
	  private int datasetId;
	  String internalIsTiled;
	  private String name;
	  private int maxX; 
	  private int maxY; 
	  private int maxSclices; 
	  private String zoomLevels; 
	  private int oneToOneZoomLevel; 
	  private String transformationMatrix;

	@Id 
	@Column(name="id")
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@XmlTransient
	public long getId() {
		return id;
	}
	public void setId(long id) {
		this.id = id;
	}
	
	@Column(name="dataset_id")
	@XmlTransient
	public int getDatasetId() {
		return datasetId;
	}
	
	public void setDatasetId(int datasetId) {
		this.datasetId = datasetId;
	}
	
	@XmlTransient
	@Column(name="is_tiled")
	public String getInternalIsTiled() {
	return this.internalIsTiled;
	}

	public void setInternalIsTiled(String internalIsTiled) {
		this.internalIsTiled = internalIsTiled;
	}
		   
	@XmlElement(name="IsTiled", namespace=IGlobalConstants.XML_NAMESPACE)
	@Transient
	public boolean getIsTiled() {
		if (this.internalIsTiled == null || this.internalIsTiled.trim().isEmpty()) {
			return false;
		}
		
		return this.internalIsTiled.trim().equalsIgnoreCase("T") || this.internalIsTiled.trim().equalsIgnoreCase("Y");
	}
	
	@Column(name="name")
	@XmlElement(name="Name", namespace=IGlobalConstants.XML_NAMESPACE)	
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	@Column(name="max_x")
	@XmlElement(name="MaxX", namespace=IGlobalConstants.XML_NAMESPACE)	
	public int getMaxX() {
		return maxX;
	}
	
	public void setMaxX(int maxX) {
		this.maxX = maxX;
	}
	
	@Column(name="max_y")
	@XmlElement(name="MaxY", namespace=IGlobalConstants.XML_NAMESPACE)	
	public int getMaxY() {
		return maxY;
	}
	
	public void setMaxY(int maxY) {
		this.maxY = maxY;
	}
	
	@Column(name="max_sclices")
	@XmlElement(name="MaxSlices", namespace=IGlobalConstants.XML_NAMESPACE)	
	public int getMaxSclices() {
		return maxSclices;
	}
	
	public void setMaxSclices(int maxSclices) {
		this.maxSclices = maxSclices;
	}
	
	@Column(name="zoom_levels")
	@XmlElement(name="ZoomLevels", namespace=IGlobalConstants.XML_NAMESPACE)	
	public String getZoomLevels() {
		return zoomLevels;
	}
	
	public void setZoomLevels(String zoomLevels) {
		this.zoomLevels = zoomLevels;
	}
	
	@Column(name="one_to_one_zoom_level")
	@XmlElement(name="OneToOneZoomLevel", namespace=IGlobalConstants.XML_NAMESPACE)	
	public int getOneToOneZoomLevel() {
		return oneToOneZoomLevel;
	}
	
	public void setOneToOneZoomLevel(int oneToOneZoomLevel) {
		this.oneToOneZoomLevel = oneToOneZoomLevel;
	}
	
	@Column(name="transformation_matrix")
	@XmlElement(name="TransformationMatrix", namespace=IGlobalConstants.XML_NAMESPACE)	
	public String getTransformationMatrix() {
		return transformationMatrix;
	}
	
	public void setTransformationMatrix(String transformationMatrix) {
		this.transformationMatrix = transformationMatrix;
	}
}

