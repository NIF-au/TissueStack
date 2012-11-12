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

import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.Query;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetPlanes;

public final class DataSetPlanesDataProvider {

	public static DataSetPlanes queryDataSetPlanesById(long id){
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
			
		} finally{
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
