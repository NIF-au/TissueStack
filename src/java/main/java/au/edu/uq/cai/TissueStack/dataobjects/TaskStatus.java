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
