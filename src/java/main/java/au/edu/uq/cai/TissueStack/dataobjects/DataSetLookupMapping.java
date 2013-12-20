/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
package au.edu.uq.cai.TissueStack.dataobjects;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.IdClass;
import javax.persistence.JoinColumn;
import javax.persistence.OneToOne;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@Entity
@Table(name="dataset_lookup_mapping")
@IdClass(DataSetLookupMappingCompositeKey.class)
@XmlRootElement(name="DataSetLookupMapping", namespace=IGlobalConstants.XML_NAMESPACE)
public class DataSetLookupMapping {

	private long datasetId; 
	private DataSet associatedDataSet;

	@Id
	@Column(name="dataset_id")
	@XmlElement(name="DataSetId", namespace=IGlobalConstants.XML_NAMESPACE)
	public long getDatasetId() {
		return datasetId;
	}

	public void setDatasetId(long datasetId) {
		this.datasetId = datasetId;
	}

	@Id
	@OneToOne
	@JoinColumn(name="associated_dataset_id")
	@XmlElement(name="AssociatedDataSet", namespace=IGlobalConstants.XML_NAMESPACE)
	public DataSet getAssociatedDataSet() {
		return this.associatedDataSet;
	}
	public void setAssociatedDataSet(DataSet associatedDataSet) {
		this.associatedDataSet = associatedDataSet;
	}
	
	public boolean equals(Object obj) {
		if (!(obj instanceof DataSetLookupMapping)) return false;
		if (this.associatedDataSet == null) return false;
		
		final DataSetLookupMapping castObj = (DataSetLookupMapping) obj;

		final DataSetLookupMappingCompositeKey thisKey = new DataSetLookupMappingCompositeKey();
		thisKey.setDatasetId(this.getDatasetId());
		thisKey.setAssociatedDataSet(this.getAssociatedDataSet());

		final DataSetLookupMappingCompositeKey key = new DataSetLookupMappingCompositeKey();
		key.setDatasetId(castObj.getDatasetId());
		key.setAssociatedDataSet(castObj.getAssociatedDataSet());
		
		return thisKey.equals(key);
	}
	
	public int hashCode() {
		if (this.associatedDataSet == null) return DataSetLookupMapping.class.getName().length();
		
		final DataSetLookupMappingCompositeKey key = new DataSetLookupMappingCompositeKey();
		key.setDatasetId(this.getDatasetId());
		key.setAssociatedDataSet(this.getAssociatedDataSet());
		
		return key.hashCode();
	}
}
