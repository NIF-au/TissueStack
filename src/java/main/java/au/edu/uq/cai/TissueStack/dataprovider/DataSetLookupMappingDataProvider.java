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
package au.edu.uq.cai.TissueStack.dataprovider;

import java.util.ArrayList;
import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.Query;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetLookupMapping;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetLookupMappingCompositeKey;

public final class DataSetLookupMappingDataProvider {

	public static DataSetLookupMapping queryDataSetLookupMappingById(
			long dataSetId, long associatedDataSet, boolean includeDataSetPlanes){
		DataSetLookupMappingCompositeKey key = new DataSetLookupMappingCompositeKey();
		key.setDatasetId(dataSetId);
		key.setDatasetId(associatedDataSet);
		
		//delegate
		return DataSetLookupMappingDataProvider.queryDataSetLookupMappingById(key, includeDataSetPlanes);
	}

	public static DataSetLookupMapping queryDataSetLookupMappingById(
			DataSetLookupMappingCompositeKey compositeKey, boolean includeDataSetPlanes){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			DataSetLookupMapping result = em.find(DataSetLookupMapping.class, compositeKey);
			if (result != null) {
				List<DataSetLookupMapping> tmpList = new ArrayList<DataSetLookupMapping>(2);
				tmpList.add(result);
				DataSetLookupMappingDataProvider.processLazyLoading(includeDataSetPlanes, tmpList);
				result = tmpList.get(0);
			}
			
			return result;
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

	
	@SuppressWarnings("unchecked")
	public static List<DataSetLookupMapping> queryDataSetLookupMappingForGivenDataSetId(
			long dataSetId, boolean includeDataSetPlanes){
		EntityManager em = null;
		try{
			em = JPAUtils.instance().getEntityManager();
			
			Query query = em.createQuery(
					"SELECT datasetlookupmapping FROM DataSetLookupMapping AS datasetlookupmapping "
					+ " WHERE datasetlookupmapping.datasetId = :dataSetId", DataSetLookupMapping.class);
			query.setParameter("dataSetId",dataSetId);
			List<DataSetLookupMapping> results = (List<DataSetLookupMapping>)query.getResultList();
			DataSetLookupMappingDataProvider.processLazyLoading(includeDataSetPlanes, results);
			
			return results;
		} finally{
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	@SuppressWarnings("unchecked")
	public static List<DataSetLookupMapping> queryAllDataSetLookupMapping(){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			Query query = em.createQuery("SELECT datasetlookupmapping FROM DataSetLookupMapping AS datasetlookupmapping");	
			
			return query.getResultList();	
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	private static void processLazyLoading(boolean includeDataSetPlanes, List<DataSetLookupMapping> results) {
		for (DataSetLookupMapping dataSetLookupMapping : results) {
			if (dataSetLookupMapping.getAssociatedDataSet() == null) continue;

			if (includeDataSetPlanes) { // include planes data if needed
				dataSetLookupMapping.setAssociatedDataSet(
					DataSetDataProvider.queryDataSetById(dataSetLookupMapping.getAssociatedDataSet().getId()));
			} else {
				// avoid lazy loading issues 
				dataSetLookupMapping.getAssociatedDataSet().setPlanes(null);
				dataSetLookupMapping.getAssociatedDataSet().setOverlays(null);
			}
		}
	}
}
