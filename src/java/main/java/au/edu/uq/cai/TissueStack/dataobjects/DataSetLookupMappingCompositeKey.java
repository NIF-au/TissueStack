package au.edu.uq.cai.TissueStack.dataobjects;

import java.io.Serializable;

public class DataSetLookupMappingCompositeKey implements Serializable {
	private static final long serialVersionUID = 7973083306588155714L;

	private long datasetId; 
	private DataSet associatedDataSet;

	public long getDatasetId() {
		return datasetId;
	}
	public void setDatasetId(long datasetId) {
		this.datasetId = datasetId;
	}
	public DataSet getAssociatedDataSet() {
		return associatedDataSet;
	}
	public void setAssociatedDataSet(DataSet associatedDataSet) {
		this.associatedDataSet = associatedDataSet;
	}
	
	public boolean equals(Object obj) {
		if (obj == null || this.associatedDataSet == null) return false;
		if (!(obj instanceof DataSetLookupMappingCompositeKey)) return false;
		
		final DataSetLookupMappingCompositeKey castObj = (DataSetLookupMappingCompositeKey) obj;
		if (this.datasetId == castObj.datasetId && this.associatedDataSet.equals(castObj.associatedDataSet)) return true;
		
		return false;
	}
	
	public int hashCode() {
		if (this.associatedDataSet == null) return new Long(this.datasetId).intValue();

		return new Long(this.datasetId + this.associatedDataSet.getId()).intValue();
	}
}
