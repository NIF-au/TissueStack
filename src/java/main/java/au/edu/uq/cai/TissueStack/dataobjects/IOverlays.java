package au.edu.uq.cai.TissueStack.dataobjects;

public interface IOverlays {
	public enum OverlayType {
		CANVAS, SVG, DATASET;
	}

	long getId();
	OverlayType getOverlayType();
}
