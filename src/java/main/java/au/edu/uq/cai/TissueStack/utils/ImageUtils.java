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