package au.edu.uq.cai.TissueStack.jni;

import au.edu.uq.cai.TissueStack.dataobjects.MincInfo;

public class TissueStack {
    static {
        try {
            System.loadLibrary("TissueStack");
        } catch(UnsatisfiedLinkError failedToLoadNativeLibrary) {
            throw new RuntimeException("Failed to load native library!", failedToLoadNativeLibrary);
        }
    }

    public native MincInfo getMincInfo(String fileName);
    public native String tileMincVolume(
    		String fileName, String baseDirectory, int dimensions[], int tileSize, double zoomFactor, String imageType, boolean preview);
    
    // main method was for testing only
    public static void main(String[] args) {
    	
     if (args.length <=0) {
    	 return;
     }
      
      MincInfo result = new TissueStack().getMincInfo(args[0]);
      assert result != null;
      System.out.println(result.getFilename());
      for (int i=0;i<result.getDimensions().length;i++) {
    	  System.out.println(result.getDimensions()[i] + " =>" + result.getSizes()[i]);
      }
    }
}
