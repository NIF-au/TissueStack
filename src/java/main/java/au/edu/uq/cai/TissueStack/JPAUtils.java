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
package au.edu.uq.cai.TissueStack;

import java.util.HashMap;
import java.util.Map;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.EntityTransaction;
import javax.persistence.Persistence;

import org.apache.log4j.Logger;
import org.hibernate.Session;

public final class JPAUtils {
	final Logger  logger = Logger.getLogger(JPAUtils.class);
	
	private static JPAUtils myself;
	
	private Map<String, EntityManagerFactory> entityManagerFactories = 
			new HashMap<String, EntityManagerFactory>(5);
	
	private JPAUtils() {}
	
	public EntityManagerFactory getEntityManagerFactory(String entityManager) {
		if (entityManager == null || entityManager.trim().isEmpty()) {
			entityManager = "default";
		}
		
		EntityManagerFactory foundFactory = this.entityManagerFactories.get(entityManager); 
		if (foundFactory != null) {
			return foundFactory;
		}
		
		logger.info("Creating entity manager...");

		try {
			if (entityManager.equals("default")) {
				logger.info("No entity manager string was provided. Looking for default one in properties file...");
				
				foundFactory =
						Persistence.createEntityManagerFactory("defaultEntityManager");
			} else {
				foundFactory = Persistence.createEntityManagerFactory(entityManager);
			}
			
			this.entityManagerFactories.put(entityManager, foundFactory);

			logger.info("Created the entity manager succcessfully...");
		} catch (Exception any) {
			logger.error("Failed to create the entity manager!", any);
			throw new RuntimeException("Failed to create the entity manager!", any);
		}

		return foundFactory;
	}

	public EntityManager getEntityManager() {
		return this.getEntityManager(null);
	}

	public EntityManager getEntityManager(String entityManager) {
		return this.getEntityManagerFactory(entityManager).createEntityManager();
	}

	public Session getHibernateSession() {
		return this.getHibernateSession(null);
	}

	public Session getHibernateSession(String entityManager) {
		return (Session) this.getEntityManager(entityManager).getDelegate();
	}

	public void resetEntityManager() {
		try {
			this.entityManagerFactories.clear();
		} catch(Exception any) {
			// we can safely ignore that
		}
	}

	public static JPAUtils instance() {
		if (JPAUtils.myself == null) {
			JPAUtils.myself = new JPAUtils();
		}
		return JPAUtils.myself;
	}

	public void rollbackTransaction(EntityTransaction trans) {
		try {
			trans.rollback();
		} catch(Exception any) {
			// we can safely ignore that
		} finally {
			trans = null;
		}
	}

	public void closeEntityManager(EntityManager em) {
		try {
			em.close();
		} catch(Exception any) {
			// we can safely ignore that
		} finally {
			em = null;
		}
	}
}
