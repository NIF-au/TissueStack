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

import java.util.ArrayList;
import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;
import javax.persistence.Query;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.AbstractDataSetOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.CanvasOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.IOverlays.OverlayType;
import au.edu.uq.cai.TissueStack.dataobjects.OtherDataSetOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.SVGOverlay;

public class DataSetOverlaysProvider {
	final static Logger logger = Logger.getLogger(DataSetOverlaysProvider.class);

	@SuppressWarnings("unchecked")
	public static List<DataSetOverlay> findOverlaysInformationByDataSetId(long dataSetId) {
		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			
			final Query query = em.createQuery(
					"SELECT MIN(overlay.id) AS overlay_id, MIN(overlay.name), overlay.overlayType FROM AbstractDataSetOverlay overlay" 
							+ " WHERE overlay.dataSetId = :dataSetId GROUP BY overlay.dataSetId,dataSetId.overlayType" 
							+ " ORDER BY overlay_id");
			query.setParameter("dataSetId", dataSetId);		
			
			final List<Object[]> results = query.getResultList();
			final List<DataSetOverlay> returnObject = new ArrayList<DataSetOverlay>(results.size()); 
			for (Object res[]: results) {
				if (res.length != 3)
					continue;
				final DataSetOverlay ret = new DataSetOverlay();
				if (res[1] instanceof String)
					ret.setName((String)res[1]);
				if (res[2] instanceof OverlayType)
					ret.setType(((OverlayType)res[2]).name());
				returnObject.add(ret);
			}
			
			return returnObject;
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

	@SuppressWarnings("unchecked")
	public static List<Object[]> findOverlayIdsForSlices(long dataSetId, long dataSetPlaneId, OverlayType type) {
		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			
			final Query query = 
					em.createQuery("SELECT overlay.slice, overlay.id FROM AbstractDataSetOverlay overlay "
							+ " WHERE overlay.dataSetId = :dataSetId"
							+ " AND overlay.dataSetPlaneId = :dataSetPlaneId AND overlay.overlayType = :type"
							+ " ORDER BY overlay.slice");
			query.setParameter("dataSetId", dataSetId);		
			query.setParameter("dataSetPlaneId", dataSetPlaneId);
			query.setParameter("type", type);
			
			return (List<Object[]>) query.getResultList();
		} finally {
			JPAUtils.instance().closeEntityManager(em);
		}
	}

	@SuppressWarnings("unchecked")
	public static List<AbstractDataSetOverlay> findOverlayById(long id, OverlayType type) {
		EntityManager em = null;
		
		try {
			em = JPAUtils.instance().getEntityManager();
			
			String overlayTable;
			if (OverlayType.CANVAS.equals(type))
				overlayTable = CanvasOverlay.class.getSimpleName();
			else if (OverlayType.SVG.equals(type))
				overlayTable = SVGOverlay.class.getSimpleName();
			else if (OverlayType.DATASET.equals(type))
				overlayTable = OtherDataSetOverlay.class.getSimpleName();
			else 
				return null;
			
			final Query query = em.createQuery(	"SELECT overlay FROM " + overlayTable + " overlay WHERE overlay.id = :id");
			query.setParameter("id", id);		

			return query.getResultList();
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
