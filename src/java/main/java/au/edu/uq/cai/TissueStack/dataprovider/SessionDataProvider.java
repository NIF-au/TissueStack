package au.edu.uq.cai.TissueStack.dataprovider;

import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;
import javax.persistence.Query;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.Session;

public final class SessionDataProvider {

	final static Logger logger = Logger.getLogger(SessionDataProvider.class); 
			
	public static Session querySessionById(String id) {
		if (id == null) {
			return null;
		}

		EntityManager em = null; 
		try {
			em = JPAUtils.instance().getEntityManager(); 
			
			return em.find(Session.class, id);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static void persistSession(Session session) {
		if (session == null || session.getId() == null || session.getId().trim().isEmpty() || session.getExpiry() < 0) {
			throw new IllegalArgumentException("New Session has to be not null with a non-empty id and a valid expiry date");
		}

		EntityManager em = null;
		EntityTransaction write = null;
		try {
			em = JPAUtils.instance().getEntityManager();
			write = em.getTransaction();
			
			write.begin();
			em.persist(session);
			write.commit();
		} catch(Exception any) {
			// roll back
			JPAUtils.instance().rollbackTransaction(write);
			// log and propagate
			logger.error("Failed to persist session", any);
			throw new RuntimeException(any);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static int deleteAllExpiredSessions() {
		EntityManager em = null; 
		EntityTransaction write = null;

		try {
			em = JPAUtils.instance().getEntityManager(); 
			write = em.getTransaction();
			
			final Query query = em.createQuery("DELETE FROM Session session WHERE session.expiry < :expiry");
			query.setParameter("expiry", System.currentTimeMillis());
			
			write.begin();
			int rowsDeleted = query.executeUpdate();
			write.commit();
			
			return rowsDeleted;
		} catch(Exception any) {
			// roll back
			JPAUtils.instance().rollbackTransaction(write);
			// log and propagate
			logger.error("Failed to delete expired session records", any);
			throw new RuntimeException(any);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static int invalidateSession(String id) {
		if (id == null) {
			return 0;
		}
		
		EntityManager em = null; 
		EntityTransaction write = null;

		try {
			em = JPAUtils.instance().getEntityManager(); 
			write = em.getTransaction();

			final Query query = em.createQuery("DELETE FROM Session session WHERE session.id = :id");
			query.setParameter("id", id);

			write.begin();
			int rowsDeleted = query.executeUpdate();
			write.commit();
			
			return rowsDeleted;
		} catch(Exception any) {
			// roll back
			JPAUtils.instance().rollbackTransaction(write);
			// log and propagate
			logger.error("Failed to delete session record" + id, any);
			throw new RuntimeException(any);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	
	public static int extendSessionExpiry(String id, long timeout) {
		if (id == null || timeout < 0) {
			return 0;
		}
		
		EntityManager em = null; 
		EntityTransaction write = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			write = em.getTransaction();

			final Query query = em.createQuery("UPDATE Session session SET session.expiry = :expiry WHERE session.id = :id");
			query.setParameter("id", id);
			query.setParameter("expiry", System.currentTimeMillis() + timeout);

			write.begin();
			int rowsUpdated = query.executeUpdate();
			write.commit();
			
			return rowsUpdated;
		} catch(Exception any) {
			// roll back
			JPAUtils.instance().rollbackTransaction(write);
			// log and propagate
			logger.error("Failed to update session record " + id, any);
			throw new RuntimeException(any);
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

}
