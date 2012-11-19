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

import java.util.Timer;
import java.util.TimerTask;

import javax.servlet.ServletContextEvent;

import org.apache.log4j.Logger;
import org.jboss.resteasy.plugins.server.servlet.ResteasyBootstrap;

import au.edu.uq.cai.TissueStack.dataprovider.ColorMapsProvider;
import au.edu.uq.cai.TissueStack.dataprovider.SessionDataProvider;

public class CustomRestfulBootstrap extends ResteasyBootstrap {

	final Logger logger = Logger.getLogger(CustomRestfulBootstrap.class);
	
	private final static long DATABASE_SESSION_CLEANER_INTERVAL = 1000 * 60 * 2; // clean expired sessions every 2 minutes
	private Timer sessionCleaner = null;
	
	public void contextInitialized(ServletContextEvent event) {
		logger.info("Initializing servlet context...");
		// let the reast easy bootloader do its thing
		super.contextInitialized(event);

		// store list of resource classes to be used later on for meta data extraction
		try {
			event.getServletContext().setAttribute("resource_classes", deployment.getResourceClasses());
		} catch(Exception any) {
			logger.error("Failed to obtain rest easy resource information", any);
		}

		// instantiate entity manager for configuration connectivity
		try {
			JPAUtils.instance().getEntityManager();
		} catch(Exception any) {
			logger.error("Failed to connect to configuration database", any);
		}

		// start the session cleaner thread
		this.sessionCleaner = new Timer("Session Cleaner", true);
		try {
			this.sessionCleaner.scheduleAtFixedRate(
					new TimerTask() {
						public void run() {
							int rowsDeleted = SessionDataProvider.deleteAllExpiredSessions();
							if (rowsDeleted > 0) {
								logger.info("Deleted " + rowsDeleted + " expired sessions.");
							}
						}
					},
					60000, // initial delay
					DATABASE_SESSION_CLEANER_INTERVAL);
		} catch (Exception any) {
			logger.error("Failed to start the session cleaner thread!", any);
		}
		
		try {
			ColorMapsProvider.instance();
		} catch (Exception any) {
			logger.error("Failed to load colormaps!", any);
		}
		
	}
	
	public void contextDestroyed(ServletContextEvent event) {
		// stop session cleaner
		try {
			this.sessionCleaner.cancel();
		} catch (Exception e) {
			// at this stage we don't care any more, nor could we do something about it
		}

		// destroy entity manager
		try {
			JPAUtils.instance().resetEntityManager();
		} catch (Exception any) {
			// at this stage we don't care any more, nor could we do something about it
		}
		
		super.contextDestroyed(event);
	}
}
