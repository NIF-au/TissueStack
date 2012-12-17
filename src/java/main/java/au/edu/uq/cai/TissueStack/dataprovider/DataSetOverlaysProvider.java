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

import java.math.BigInteger;
import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;
import javax.persistence.Query;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.AbstractDataSetOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.IOverlays.OverlayType;

public class DataSetOverlaysProvider {
	final static Logger logger = Logger.getLogger(DataSetOverlaysProvider.class);
	final static String OVERLAY_SEQUENCE_NAME = "dataset_overlays_custom_seq";

	@SuppressWarnings("unchecked")
	public static List<AbstractDataSetOverlay> findOverlayByType(OverlayType type) {
		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			
			final Query query = em.createQuery("SELECT overlay FROM AbstractDataSetOverlay overlay WHERE overlay.overlayType = :type");
			query.setParameter("type", type);		
			
			return (List<AbstractDataSetOverlay>) query.getResultList();
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

	@SuppressWarnings("unchecked")
	public static List<AbstractDataSetOverlay> findOverlayByDataSetPlanesId(long planesId) {
		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			
			final Query query = em.createQuery("SELECT overlay FROM AbstractDataSetOverlay overlay WHERE overlay.dataSetPlaneId = :planesId");
			query.setParameter("planesId", planesId);		
			
			return (List<AbstractDataSetOverlay>) query.getResultList();
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

	public static long getNextOverlayIdSequenceNumber() {
		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			
			final Query query = em.createNativeQuery("SELECT nextval('" + OVERLAY_SEQUENCE_NAME + "');");
			
			return ((BigInteger) query.getResultList().get(0)).longValue();
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
	

	public static void insertOverlays(AbstractDataSetOverlay overlays[]) {
		if (overlays == null || overlays.length == 0) return;
		
		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();

			final EntityTransaction insert = em.getTransaction();
			insert.begin();
			
			for (AbstractDataSetOverlay ol : overlays)
				em.persist(ol);
			
			insert.commit();
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

	
	public static void insertOverlay(AbstractDataSetOverlay overlay) {
		if (overlay == null) return;

		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			
			final EntityTransaction insert = em.getTransaction();
			insert.begin();
			em.persist(overlay);
			insert.commit();
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}
}
