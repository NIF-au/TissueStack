package au.edu.uq.cai.TissueStack;

import java.util.HashMap;
import java.util.Map;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
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
