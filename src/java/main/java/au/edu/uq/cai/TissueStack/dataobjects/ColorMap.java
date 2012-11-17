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

@XmlRootElement(name="ColorMap", namespace=IGlobalConstants.XML_NAMESPACE)
public class ColorMap {

	private String name; 
	private String file; 
	private String json;
	private String indexedJson;

	@XmlElement(name="Name", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getName() {
		return this.name; 
	}
	public void setName(String name) {
		this.name = name;
	}

	@XmlElement(name="File", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getFile() {
		return this.file; 
	}
	public void setFile(String file) {
		this.file = file;
	}

	@XmlElement(name="Json", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getJson() {
		return this.json; 
	}
	public void setJson(String json) {
		this.json = json;
	}

	@XmlElement(name="IndexedJson", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getIndexedJson() {
		return this.indexedJson; 
	}
	public void setIndexedJson(String indexedJson) {
		this.indexedJson = indexedJson;
	}

}