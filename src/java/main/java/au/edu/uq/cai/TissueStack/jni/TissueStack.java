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
package au.edu.uq.cai.TissueStack.jni;

import au.edu.uq.cai.TissueStack.dataobjects.MincInfo;
import au.edu.uq.cai.TissueStack.dataobjects.TaskStatus;

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
    public native String convertImageFormatToRaw(String imageFile, String newRawFile, short formatIdentifier);
    public native TaskStatus callTaskAction(String taskID, short action);
    
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
