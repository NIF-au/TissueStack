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
package au.edu.uq.cai.TissueStack.security;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public final class SHA2encoder {
	// for testing only
    public static void main(String[] args)throws Exception
    {
    	if (args.length != 1) {
    		throw new IllegalArgumentException("Hand in the string to encode!");
    	}
    	
        byte byteData[] = SHA2encoder.encode(args[0]);
 
        System.out.println("SHA2 in hex format : " + SHA2encoder.convertByteArrayToHexString(byteData));
    }
    
    public static byte[] encode(String text) throws NoSuchAlgorithmException {
    	if (text == null || text.trim().length() == 0) {
    		return null;
    	}
    	
    	final MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(text.getBytes());
        
        return md.digest();
    }
    
    public static String convertByteArrayToHexString(byte[] bytes) {
    	if (bytes == null || bytes.length == 0) {
    		return null;
    	}
    	
        StringBuffer sb = new StringBuffer((int) (bytes.length * 1.5));
        for (int i = 0; i < bytes.length; i++) {
        	sb.append(Integer.toString((bytes[i] & 0xff) + 0x100, 16).substring(1));
        }

    	return sb.toString();
    }
}