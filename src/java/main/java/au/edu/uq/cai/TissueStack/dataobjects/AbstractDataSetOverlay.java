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
import javax.persistence.DiscriminatorColumn;
import javax.persistence.DiscriminatorType;
import javax.persistence.Entity;
import javax.persistence.EnumType;
import javax.persistence.Enumerated;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Inheritance;
import javax.persistence.InheritanceType;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.apache.log4j.Logger;

@Entity
@Inheritance(strategy=InheritanceType.SINGLE_TABLE)
@DiscriminatorColumn(discriminatorType=DiscriminatorType.STRING,name="type")
@Table(name="dataset_overlays")
@XmlRootElement(name="Overlay", namespace=IGlobalConstants.XML_NAMESPACE)
@XmlSeeAlso({CanvasOverlay.class, SVGOverlay.class})
public class AbstractDataSetOverlay implements IOverlays {

	final Logger logger = Logger.getLogger(AbstractDataSetOverlay.class);

	private long id = 0;
	private long dataSetId = 0;
	private long dataSetPlaneId = 0;
	private int slice = 0;
	private OverlayType overlayType;
	private String name;

	public AbstractDataSetOverlay() {
		this(OverlayType.CANVAS);
	}
	
	public AbstractDataSetOverlay(OverlayType overlayType) {
		if (overlayType == null) overlayType = OverlayType.CANVAS;
		this.setOverlayType(overlayType);		
	};
	
	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@Column(name = "id", unique = true)
	public long getId() {
		return this.id;
	}

	public void setId(long id) {
		this.id = id;
	}

	@Column(name = "dataset_id")
	@XmlElement(name = "DataSetId", namespace = IGlobalConstants.XML_NAMESPACE)
	public long getDataSetId() {
		return this.dataSetId;
	}

	public void setDataSetId(long dataSetId) {
		this.dataSetId = dataSetId;
	}

	@Column(name = "dataset_planes_id")
	@XmlElement(name = "DataSetPlanesId", namespace = IGlobalConstants.XML_NAMESPACE)
	public long getDataSetPlaneId() {
		return this.dataSetPlaneId;
	}

	public void setDataSetPlaneId(long dataSetPlaneId) {
		this.dataSetPlaneId = dataSetPlaneId;
	}

	@Column(name = "slice")
	@XmlElement(name = "Slice", namespace = IGlobalConstants.XML_NAMESPACE)
	public int getSlice() {
		return this.slice;
	}

	public void setSlice(int slice) {
		this.slice = slice;
	}
	
	@XmlElement(name = "OverlayType", namespace = IGlobalConstants.XML_NAMESPACE)
	@Column(name = "type", length = 10, nullable = false, insertable = false, updatable = false)
	@Enumerated(EnumType.STRING)
	public OverlayType getOverlayType() {
		return this.overlayType;
	}

	public void setOverlayType(OverlayType overlayType) {
		this.overlayType = overlayType;
	}

	@XmlElement(name = "Name", namespace = IGlobalConstants.XML_NAMESPACE)
	@Column(name = "name")
	public String getName() {
		return this.name;
	}

	public void setName(String name) {
		this.name = name;
	}
}
