package au.edu.uq.cai.TissueStack.dataprovider;

import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.Query;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.DataSet;

public final class DataSetDataProvider {
	
	public static DataSet queryDataSetById (int id){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			return em.find(DataSet.class, id);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	@SuppressWarnings("unchecked")
	public static List<DataSet> queryAllDataSetValue (){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			Query query = em.createQuery("SELECT dataset FROM DataSet AS dataset");	
			
			return query.getResultList();	
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
