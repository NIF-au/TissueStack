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
import javax.persistence.Id;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@Entity
@Table(name="session")
@XmlRootElement(name="Session", namespace=IGlobalConstants.XML_NAMESPACE)
public class Session {
	private String id;
	private long expiry;
	
	@Id
	@Column(name="id")
	@XmlElement(name="Id", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getId() {
		return id;
	}
	
	public void setId(String id) {
		this.id = id;
	}
	
	@Column(name="expiry")
	@XmlElement(name="Expiry", namespace=IGlobalConstants.XML_NAMESPACE)
	public long getExpiry() {
		return expiry;
	}
	
	public void setExpiry(long expiry) {
		this.expiry = expiry;
	}
	
	public boolean equals(Object obj) {
		if (!(obj instanceof Session) || this.id == null) return false;
		
		final Session castObj = (Session) obj;
		return (this.id.equals(castObj.id));
	}
	
	public int hashCode() {
		if (this.id == null) return Session.class.getName().length();

		return this.id.length();
	}
}
