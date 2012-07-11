package au.edu.uq.cai.TissueStack.dataprovider;

import java.util.ArrayList;
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
	public static DataSet queryDataSetById (long id){
		EntityManager em = null; 
		try {
			
			em = JPAUtils.instance().getEntityManager();
			
			Query query = em.createQuery("SELECT DISTINCT dataset FROM DataSet dataset LEFT JOIN FETCH dataset.planes WHERE dataset.id = :id");	
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
		EntityManager em = null; 
		EntityTransaction write = null;
		try {			
			em = JPAUtils.instance().getEntityManager();
			write = em.getTransaction();
			write.begin();
			em.persist(dataset);
			write.commit();
			em.close();
		} catch(Exception any) {
			JPAUtils.instance().rollbackTransaction(write);
			logger.error("Failed to add new Data Set: " + dataset, any);
			throw new RuntimeException("Already Exist in Data Set",any);
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
				dataSet.setPlanes(new ArrayList<DataSetPlanes>());
			}
			
			return result;	
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
