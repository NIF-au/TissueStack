package au.edu.uq.cai.TissueStack.jni;

public class TissueStack {
    static {
        try {
            System.loadLibrary("TissueStack");
        } catch(UnsatisfiedLinkError failedToLoadNativeLibrary) {
            throw new RuntimeException("NativeGdalDataProvider: Failed to load native library!", failedToLoadNativeLibrary);
        }
    }
	
    public native void test(String fileName);

    // main method was for testing only
    public static void main(String[] args) {
      new TissueStack().test("/data/home/harald/data/mouse/00-normal-model-nonsym.mnc");
    }
}
