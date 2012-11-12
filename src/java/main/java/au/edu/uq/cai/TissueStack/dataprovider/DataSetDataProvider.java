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

import java.util.Arrays;
import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;
import javax.persistence.Query;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetPlanes;

public final class DataSetDataProvider {
	final static Logger logger = Logger.getLogger(DataSetDataProvider.class); 

	public static DataSet queryDataSetByFileName (String fileName){
		if (fileName == null || fileName.trim().isEmpty()) {
			return null;
		}
		
		EntityManager em = null; 
		try {
			
			em = JPAUtils.instance().getEntityManager();
			
			Query query = em.createQuery("SELECT DISTINCT dataset FROM DataSet dataset LEFT JOIN FETCH dataset.planes WHERE dataset.filename = :filename");	
			query.setParameter("filename", fileName.trim());
			
			@SuppressWarnings("unchecked")
			List<DataSet> result = query.getResultList();

			if (result == null || result.size() != 1) {
				return null;
			}
			
			return result.get(0);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static DataSet queryDataSetById (long id){
		EntityManager em = null; 
		try {
			
			em = JPAUtils.instance().getEntityManager();
			
			Query query = em.createQuery("" +
					"SELECT DISTINCT dataset FROM DataSet dataset LEFT JOIN FETCH dataset.planes WHERE dataset.id = :id");	
			query.setParameter("id", id);
			
			@SuppressWarnings("unchecked")
			List<DataSet> result = query.getResultList();

			if (result == null || result.size() != 1) {
				return null;
			}
			
			return result.get(0);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static void insertNewDataSets(DataSet dataset) {
		if (dataset == null) {
			return;
		}
		
		EntityManager em = null; 
		EntityTransaction write = null;

		try {			
			em = JPAUtils.instance().getEntityManager();

			DataSetPlanes copy[] = null;
			// do we have planes attached?
			if (dataset.getPlanes() != null) {
				// copy planes
				copy = dataset.getPlanes().toArray(new DataSetPlanes[dataset.getPlanes().size()]);
				dataset.setPlanes(null);
			}
				
			write = em.getTransaction();
			write.begin();
			// persist dataset
			em.persist(dataset);
			write.commit();

			// persist associated planes now
			if (copy != null) {
				write = em.getTransaction();
				write.begin();

				for (DataSetPlanes p : copy) {
					p.setDatasetId(dataset.getId()); // set parent id as fk reference
					em.persist(p);
				}
				write.commit();
			}
			
			// append the newly persisted planes again to its parent
			dataset.setPlanes(Arrays.asList(copy));
		} catch(Exception any) {
			// undo if we were able to add the data set master
			try {
				if (dataset.getId() != 0) {
					try {
						write.rollback();
					} catch (Exception e) {
						// we can safely ignore that
					}
					dataset = em.find(DataSet.class, dataset.getId());
					write = em.getTransaction();
					write.begin();
					em.remove(dataset);
					write.commit();
				}
			} catch(Exception ignored) {
				// ignored
			}
			logger.error("Failed to add new Data Set: " + dataset, any);
			throw new RuntimeException("Failed to add new Data Set", any);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	@SuppressWarnings("unchecked")
	public static List<DataSet> getDataSets(int offset, int maxRows, String description, boolean includePlaneData){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
		    String sql = "SELECT DISTINCT dataset FROM DataSet AS dataset";
		    if (includePlaneData) {
		    	sql += " LEFT JOIN FETCH dataset.planes ";
		    }
		    
		    if (description != null && !description.isEmpty()) {
		    	sql += " WHERE LOWER(dataset.description) LIKE :description";
		    }
		    
		    sql += " ORDER BY dataset.id ASC";
		    
			Query query = em.createQuery(sql);
			if (description != null && !description.isEmpty()) {
				query.setParameter("description", "%" + description.toLowerCase() + "%");
			}
			query.setFirstResult(offset);
			query.setMaxResults(maxRows);

			final List<DataSet> result = query.getResultList();

			if (includePlaneData) {
				return result;
			}
			
			// avoid lazy loading exceptions when marshaller does its dirty deed. NOTE: don't remove this code!!!
			for (DataSet dataSet : result) {
				dataSet.setPlanes(null);
				dataSet.setLookupValues(null);
			}
			
			return result;	
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
