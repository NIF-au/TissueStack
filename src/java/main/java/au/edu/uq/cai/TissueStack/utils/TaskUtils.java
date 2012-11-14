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
package au.edu.uq.cai.TissueStack.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;

import au.edu.uq.cai.TissueStack.dataobjects.Configuration;
import au.edu.uq.cai.TissueStack.dataprovider.ConfigurationDataProvider;

public final class TaskUtils {
	final static Logger logger = Logger.getLogger(TaskUtils.class);
	private static final String DEFAULT_QUEUE_FILE = "/opt/tissuestack/tasks/general";
	
	private static File getTasksQueueFile() {
		final Configuration tasksQueueFile = ConfigurationDataProvider.queryConfigurationById("tasks_queue_file");
		return new File(tasksQueueFile == null || tasksQueueFile.getValue() == null ? DEFAULT_QUEUE_FILE : tasksQueueFile.getValue());
	}

	public static Map<String, Long> getMapOfFilesPresentlyInTaskQueue() {
		final File taskQueueFile = TaskUtils.getTasksQueueFile();
		if (taskQueueFile == null || !taskQueueFile.exists() || !taskQueueFile.canRead()) {
			logger.error("Could not find task queue file!");
			return null;
		}
		
		final Map<String, Long> tasksQueued = new HashMap<String, Long>(25);
		
		BufferedReader reader = null;
		
		try {
			reader = new BufferedReader(new FileReader(taskQueueFile));
			String line = null;
			while ((line = reader.readLine()) != null) {
				final String[] associatedDataSetFiles = 
						TaskUtils.getTaskFileDataSetFiles(
								new File(taskQueueFile.getParent(),line).getAbsolutePath());
				if (associatedDataSetFiles != null)
					if (associatedDataSetFiles[0] != null)
						tasksQueued.put(associatedDataSetFiles[0], Long.parseLong(line));
					if (associatedDataSetFiles[1] != null)
						tasksQueued.put(associatedDataSetFiles[1], Long.parseLong(line));
			}
				
		} catch (Exception e) {
			logger.error("Failed to read Task Queue File!", e);
		} finally {
			try {
				reader.close();
			} catch(Exception ignored) {
				// we are save
			}
		}
		
		return tasksQueued;
	}

	public static String[] getTaskFileDataSetFiles(String file) {
		if (file == null) return null;
		
		final File taskFile = new File(file);
		if (taskFile == null || !taskFile.exists() || !taskFile.canRead()) {
			return null;
		}
		
		BufferedReader reader = null;
		
		final String ret[] = new String[2];
		
		try {
			reader = new BufferedReader(new FileReader(taskFile));
			String line = null;
			int lineNumber = 1;
			while ((line = reader.readLine()) != null) {
				if (lineNumber == 4) ret[0] = line;
				else if (lineNumber == 6) {
					ret[1] = line;
					return ret;
				}
				lineNumber++;
			}
				
		} catch (Exception e) {
			logger.error("Failed to read Task Queue File!", e);
		} finally {
			try {
				reader.close();
			} catch(Exception ignored) {
				// we are save
			}
		}
		return null;
	}
	
	public static boolean isDataSetFileInTaskUse(String file) {
		 final Map<String, Long> tasksInQueue = TaskUtils.getMapOfFilesPresentlyInTaskQueue();
		 if (tasksInQueue == null || tasksInQueue.isEmpty() || tasksInQueue.get(file) == null) return false;
		 
		 return true;
	}
}
