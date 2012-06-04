package au.edu.uq.cai.TissueStack.dataprovider;

import java.util.ArrayList;
import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.Query;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetPlanes;

public final class DataSetDataProvider {
	
	public static DataSet queryDataSetById (long id){
		EntityManager em = null; 
		try {
			
			em = JPAUtils.instance().getEntityManager();
			
			Query query = em.createQuery("SELECT dataset FROM DataSet dataset LEFT JOIN FETCH dataset.planes WHERE dataset.id = :id");	
			query.setParameter("id", id);
			
			DataSet result = (DataSet) query.getSingleResult();

			return result;
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	

	@SuppressWarnings("unchecked")
	public static List<DataSet> getDataSets(int offset, int maxRows, String description){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
		    String sql = "SELECT dataset FROM DataSet AS dataset";
		    if (description != null && !description.isEmpty()) {
		    	sql += " WHERE LOWER(dataset.description) LIKE :description";
		    }
		    
			Query query = em.createQuery(sql);
			if (description != null && !description.isEmpty()) {
				query.setParameter("description", "%" + description.toLowerCase() + "%");
			}
			query.setFirstResult(offset);
			query.setMaxResults(maxRows);
			
			// avoid lazy loading exceptions when marshaller does its dirty deed. NOTE: don't remove this code!!!
			final List<DataSet> result = query.getResultList();
			for (DataSet dataSet : result) {
				dataSet.setPlanes(new ArrayList<DataSetPlanes>());
			}
			
			return result;	
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
