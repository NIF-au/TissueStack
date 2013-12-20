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

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@Entity
@Table(name="atlas_info")
@XmlRootElement(name="AtlasInfo", namespace=IGlobalConstants.XML_NAMESPACE)
public class AtlasInfo {
	private long id;
	private String prefix;
	private String description;
	private String queryUrl;
	
	@Id
	@Column(name="id")
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@XmlElement(name="Id", namespace=IGlobalConstants.XML_NAMESPACE)
	public long getId() {
		return id; 
	}
	public void setId(long id) {
		this.id = id;
	}

	@Column(name="atlas_prefix")
	@XmlElement(name="AtlasPrefix", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getPrefix() {
		return this.prefix;
	}
	
	public void setPrefix(String prefix) {
		this.prefix = prefix;
	}
	
	@Column(name="atlas_query_url")
	@XmlElement(name="QueryUrl", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getQueryUrl() {
		return this.queryUrl;
	}
	
	public void setQueryUrl(String queryUrl) {
		this.queryUrl = queryUrl;
	}
	
	@Column(name="atlas_description")
	@XmlElement(name="AtlasDescription", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getDescription() {
		return description;
	}
	
	public void setDescription(String description) {
		this.description = description;
	}
	
	public boolean equals(Object obj) {
		if (!(obj instanceof AtlasInfo)) return false;
		
		final AtlasInfo castObj = (AtlasInfo) obj;
		if (this.id == castObj.id) return true;
		
		return false;
	}
	
	public int hashCode() {
		return new Long(this.id).intValue();
	}
}
