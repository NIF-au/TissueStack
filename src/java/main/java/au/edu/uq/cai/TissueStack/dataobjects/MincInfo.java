package au.edu.uq.cai.TissueStack.dataobjects;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(namespace=IGlobalConstants.XML_NAMESPACE)
public final class MincInfo {
	private String filename;
	private String dimensions[];
	private long sizes[];
	
	public MincInfo() {	}
	public MincInfo(String filename) {
		this.setFilename(filename);
	}
	
	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getFilename() {
		return this.filename;
	}
	
	public void setFilename(String filename) {
		this.filename = filename;
	}
	
	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String[] getDimensions() {
		return this.dimensions;
	}
	
	public void setSizes(long sizes[]) {
		this.sizes = sizes;
	}

	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public long[] getSizes() {
		return this.sizes;
	}
	
	public void setDimensions(String[] dimensions) {
		this.dimensions = dimensions;
	}
}
