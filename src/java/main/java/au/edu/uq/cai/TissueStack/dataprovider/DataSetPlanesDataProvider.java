package au.edu.uq.cai.TissueStack.dataprovider;

import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.Query;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetPlanes;

public final class DataSetPlanesDataProvider {

	public static DataSetPlanes queryDataSetPlanesById(int id){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			return em.find(DataSetPlanes.class, id);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	@SuppressWarnings("unchecked")
	public static List<DataSetPlanes> queryFindMaxXYById(String name){
		EntityManager em = null;
		try{
			em = JPAUtils.instance().getEntityManager();
			
			Query query = em.createQuery("SELECT datasetplanes FROM DataSetPlanes AS datasetplanes WHERE datasetplanes.name LIKE :name", DataSetPlanes.class);
			query.setParameter("name",name);
			return (List<DataSetPlanes>)query.getResultList();
			
		}finally{
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	@SuppressWarnings("unchecked")
	public static List<DataSetPlanes> queryDataSetPlanesAllValue(){
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			Query query = em.createQuery("SELECT datasetplanes FROM DataSetPlanes AS datasetplanes");	
			
			return query.getResultList();	
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
