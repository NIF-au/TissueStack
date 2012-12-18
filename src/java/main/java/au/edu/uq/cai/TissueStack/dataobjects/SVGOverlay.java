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
import javax.persistence.DiscriminatorValue;
import javax.persistence.Entity;
import javax.persistence.PrimaryKeyJoinColumn;
import javax.persistence.SecondaryTable;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

@Entity
@DiscriminatorValue(SVGOverlay.OVERLAY_TYPE)
@SecondaryTable(name="dataset_svg_overlay", pkJoinColumns = @PrimaryKeyJoinColumn(name = "id"))
@XmlRootElement(name="SVGOverlay", namespace=IGlobalConstants.XML_NAMESPACE)
@XmlSeeAlso({AbstractDataSetOverlay.class})
public class SVGOverlay extends AbstractDataSetOverlay {

	public final static String OVERLAY_TYPE = "SVG";	
	
	private String content;
	
	public SVGOverlay() {
		super(OverlayType.valueOf(SVGOverlay.OVERLAY_TYPE));
	}

	@XmlElement(name = "Content", namespace = IGlobalConstants.XML_NAMESPACE)
	@Column(name = "content", table="dataset_svg_overlay")
	public String getContent() {
		return this.content;
	}
	
	public void setContent(String content) {
		this.content = content;
	}
}
