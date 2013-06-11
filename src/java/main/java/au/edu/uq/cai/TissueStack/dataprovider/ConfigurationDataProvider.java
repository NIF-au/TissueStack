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
import javax.persistence.EntityTransaction;
import javax.persistence.Query;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.Configuration;

public final class ConfigurationDataProvider {

	public static Configuration queryConfigurationById(String name) {
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			return em.find(Configuration.class, name);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	@SuppressWarnings("unchecked")
	public static List<Configuration> queryAllConfigurationValues() {
		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			Query query = em.createQuery("SELECT configuration FROM Configuration AS configuration");	
			
			return query.getResultList();	
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static void addOrUpdateConfigurationValue(Configuration conf) {
		if (conf == null || conf.getName() == null || conf.getValue() == null)
			return;
		
		final Configuration existing = ConfigurationDataProvider.queryConfigurationById(conf.getName());

		EntityManager em = null; 
		EntityTransaction update = null;
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			update = em.getTransaction();
			update.begin();
			
			if (existing == null)
				em.persist(conf);
			else 
				em.merge(conf);
			
			update.commit();
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
