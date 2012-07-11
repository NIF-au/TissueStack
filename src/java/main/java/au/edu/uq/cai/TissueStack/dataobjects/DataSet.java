package au.edu.uq.cai.TissueStack.dataobjects;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.FetchType;
import javax.persistence.JoinColumn;
import javax.persistence.OneToMany;
import javax.persistence.Table;
import javax.persistence.Id;
import javax.persistence.Transient;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@Entity
@Table(name="dataset")
@XmlRootElement(name="DataSet", namespace=IGlobalConstants.XML_NAMESPACE)
public class DataSet {

	@Transient
	private static String PLANE_NAMES_IN_ORDER[] = {"x","y","z"};
	
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
	
	@Transient
	public static DataSet fromMincInfo(MincInfo info) {
		if (info == null) {
			return null;
		}
		
		final DataSet newDataSet = new DataSet();
		if (info.getFilename() == null || info.getFilename().trim().isEmpty()) {
			return null;
		}
		newDataSet.setFilename(info.getFilename());
		
		// the actual dimensions aka sizes need to present
		if (info.getSizes() == null || info.getSizes().length == 0) {
			return newDataSet;
		}
		
		// loop over dimensions and create a plane for every one
		int numberOfDimensions = info.getSizes().length;
		if (numberOfDimensions == 0) {
			return newDataSet;
		}
		
		final Map<String, Integer> mapPLaneToArrayIndex = new HashMap<String, Integer>(numberOfDimensions);
		final List<DataSetPlanes> planes = new ArrayList<DataSetPlanes>(numberOfDimensions);
		
		for (int i=0; i<numberOfDimensions; i++) {
			final DataSetPlanes plane = new DataSetPlanes();
			
			// IS TILED ?
			plane.setInternalIsTiled("F"); // default is we're not tiled yet
			
			// DIM NAMES 
			// just to make super sure that we have enough names for all the dimensions ...
			if (info.getDimensions() != null && info.getDimensions().length == numberOfDimensions) {
				// this is a bit of a hack, rather make sure that name is 'x', 'y' or 'z'
				if (info.getDimensions()[i].equalsIgnoreCase("xspace")
						|| info.getDimensions()[i].indexOf("x") >= 0) {
					plane.setName("x");
				} else if (info.getDimensions()[i].equalsIgnoreCase("yspace")
						|| info.getDimensions()[i].indexOf("y") >= 0) {
					plane.setName("y");
				} else if (info.getDimensions()[i].equalsIgnoreCase("zspace")
						|| info.getDimensions()[i].indexOf("z") >= 0) {
					plane.setName("z");
				} else { 
					// we use default x,y,z order fallback
					plane.setName(PLANE_NAMES_IN_ORDER[i]);
				}
			} else {
				// we use default x,y,z order fallback
				plane.setName(PLANE_NAMES_IN_ORDER[i]);
			}
			
			// map name to array index so that we can afterwards get the dimension size and transformations right
			mapPLaneToArrayIndex.put(plane.getName(), i);
			
			// SET DEFAULT ZOOM LEVELS
			plane.setZoomLevels("[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75]");
			plane.setOneToOneZoomLevel(3);

			planes.add(plane);
		}

		// get indexes that tell us where we in the array we can find the values for a given plane
		Integer indexOfX = mapPLaneToArrayIndex.get("x");
		Integer indexOfY = mapPLaneToArrayIndex.get("y");
		Integer indexOfZ = mapPLaneToArrayIndex.get("z");

		// now loop over the new planes to assign the right max numbers and coordinate transformations (if exist)
		for (DataSetPlanes p : planes) {
			if (p.getName().equals("x")) {
				// set dimension sizes
				if (indexOfY != null) p.setMaxX(info.getSizes()[indexOfY]);
				if (indexOfZ != null) p.setMaxY(info.getSizes()[indexOfZ]);
				if (numberOfDimensions == 1) {
					p.setMaxSclices(0);
				} else {
					p.setMaxSclices(info.getSizes()[indexOfX] - 1);
				}
				
				// set transformation matrix
				if (info.getStarts() != null && info.getSteps() != null 
						&& info.getStarts().length == numberOfDimensions
						&& info.getSteps().length == numberOfDimensions) {
					final StringBuffer matrix = new StringBuffer(numberOfDimensions * 5);
					int pos = 0;
					matrix.append("[");
					if (indexOfY != null) {
						matrix.append("[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfY]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfY]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					if (indexOfZ != null) {
						matrix.append(",[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfZ]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfZ]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					if (indexOfX != null) {
						matrix.append(",[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfX]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfX]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					// add 'adjoint' row 
					matrix.append(",[");
					for (int y=0;y<numberOfDimensions;y++) {
						matrix.append("0,");
					}
					matrix.append("1]");
					matrix.append("]");
				
					// set transformation matrix
					p.setTransformationMatrix(matrix.toString());
				}
			} else if (p.getName().equals("y")) {
				// set dimension sizes
				if (indexOfX != null) p.setMaxX(info.getSizes()[indexOfX]);
				if (indexOfZ != null) p.setMaxY(info.getSizes()[indexOfZ]);
				if (numberOfDimensions == 1) {
					p.setMaxSclices(0);
				} else {
					p.setMaxSclices(info.getSizes()[indexOfY] - 1);
				}
				
				// set transformation matrix
				if (info.getStarts() != null && info.getSteps() != null 
						&& info.getStarts().length == numberOfDimensions
						&& info.getSteps().length == numberOfDimensions) {
					final StringBuffer matrix = new StringBuffer(numberOfDimensions * 5);
					int pos = 0;
					matrix.append("[");
					if (indexOfX != null) {
						matrix.append("[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfX]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfX]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					if (indexOfZ != null) {
						matrix.append(",[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfZ]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfZ]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					if (indexOfY != null) {
						matrix.append(",[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfY]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfY]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					// add 'adjoint' row 
					matrix.append(",[");
					for (int y=0;y<numberOfDimensions;y++) {
						matrix.append("0,");
					}
					matrix.append("1]");
					matrix.append("]");

					// set transformation matrix
					p.setTransformationMatrix(matrix.toString());
				}
			} else if (p.getName().equals("z")) {
				// set dimension sizes
				if (indexOfX != null) p.setMaxX(info.getSizes()[indexOfX]);
				if (indexOfY != null) p.setMaxY(info.getSizes()[indexOfY]);
				if (numberOfDimensions == 1) {
					p.setMaxSclices(0);
				} else {
					p.setMaxSclices(info.getSizes()[indexOfZ] - 1);
				}
				
				// set transformation matrix
				if (info.getStarts() != null && info.getSteps() != null 
						&& info.getStarts().length == numberOfDimensions
						&& info.getSteps().length == numberOfDimensions) {
					final StringBuffer matrix = new StringBuffer(numberOfDimensions * 5);
					int pos = 0;
					matrix.append("[");
					if (indexOfX != null) {
						matrix.append("[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										pos != 0 ? "," : "" + info.getSteps()[indexOfX]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfX]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					if (indexOfY != null) {
						matrix.append(",[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfY]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfY]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					if (indexOfZ != null) {
						matrix.append(",[");
						for (int y=0;y<=numberOfDimensions;y++) {
							if (y == pos) {
								matrix.append(
										(pos != 0 ? "," : "") + info.getSteps()[indexOfZ]);
							} else if (y == numberOfDimensions){
								matrix.append("," + info.getStarts()[indexOfZ]);
							} else {
								matrix.append((y != 0 ? "," : "") + "0");
							}
						}
						matrix.append("]");
						++pos;
					}
					// add 'adjoint' row 
					matrix.append(",[");
					for (int y=0;y<numberOfDimensions;y++) {
						matrix.append("0,");
					}
					matrix.append("1]");
					matrix.append("]");

					// set transformation matrix
					p.setTransformationMatrix(matrix.toString());
				}
			}
		}
		
		// add plane
		newDataSet.setPlanes(planes);
		
		return newDataSet;
	}
}



