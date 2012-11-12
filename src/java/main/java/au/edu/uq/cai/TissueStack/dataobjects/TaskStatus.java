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
package au.edu.uq.cai.TissueStack.dataobjects;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(namespace=IGlobalConstants.XML_NAMESPACE)
public final class TaskStatus {
	private String filename;
	private double progress;
	
	public TaskStatus (){}
	public TaskStatus (String filename, String progress){
		this.setFilename(filename);
		this.setProgress(new Double(progress));
	}
	
	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getFilename() {
		return this.filename;
	}
	
	public void setFilename(String filename) {
		this.filename = filename;
	}
	
	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public double getProgress() {
		return this.progress;
	}
	
	public void setProgress(double progress) {
		this.progress = progress;
	}
}
