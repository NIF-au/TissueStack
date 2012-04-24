package au.edu.uq.cai.TissueStack.jni;

import au.edu.uq.cai.TissueStack.dataobjects.MincTest;

public class TissueStack {
    static {
        try {
            System.loadLibrary("TissueStack");
        } catch(UnsatisfiedLinkError failedToLoadNativeLibrary) {
            throw new RuntimeException("NativeGdalDataProvider: Failed to load native library!", failedToLoadNativeLibrary);
        }
    }
	
    public native MincTest test(String fileName);

    // main method was for testing only
    public static void main(String[] args) {
    	
     if (args.length <=0) {
    	 return;
     }
      
      MincTest result = new TissueStack().test(args[0]);
      assert result != null;
      System.out.println(result.getFilename());
      for (int i=0;i<result.getDimensions().length;i++) {
    	  System.out.println(result.getDimensions()[i] + " =>" + result.getSizes()[i]);
      }
    }
}
