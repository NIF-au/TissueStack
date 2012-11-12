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
package au.edu.uq.cai.TissueStack.utils;

import java.io.File;
import java.io.RandomAccessFile;

public final class ImageUtils {
	
		public static final String RAW_HEADER = "@IaMraW@";

		public static final boolean isRawFormat(File potentialRawFile) {
			if (!potentialRawFile.isFile() || !potentialRawFile.exists()) return false;
			
			if (potentialRawFile.length() < RAW_HEADER.length()) return false;
			
			String header = null;
			
			try {
				byte content[] = new byte[RAW_HEADER.length()];
				final RandomAccessFile raf = new RandomAccessFile(potentialRawFile, "r");
				raf.read(content);
				
				header = new String(content);
			} catch(Exception any) {
				return false;
			}
			
			if (header == null || !header.equals(RAW_HEADER)) return false; 

			return true;
		}

		public static final boolean isRawFormat(String filename) {
			if (filename == null || filename.trim().isEmpty()) return false;

			return ImageUtils.isRawFormat(new File(filename)); 
		}
}