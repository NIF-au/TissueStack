package au.edu.uq.cai.TissueStack;

import javax.servlet.ServletContextEvent;

import org.apache.log4j.Logger;
import org.jboss.resteasy.plugins.server.servlet.ResteasyBootstrap;

public class CustomRestfulBootstrap extends ResteasyBootstrap {

	final Logger logger = Logger.getLogger(CustomRestfulBootstrap.class);
	
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
	}
	
	public void contextDestroyed(ServletContextEvent event) {
		super.contextDestroyed(event);
	}
}
