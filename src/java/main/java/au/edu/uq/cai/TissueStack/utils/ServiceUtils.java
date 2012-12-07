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

import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;

import javax.servlet.http.HttpServletResponse;

public final class ServiceUtils {
	public static void streamString(
			HttpServletResponse httpResponse,
			String contentType,
			String charSet,
			String content) {
		if (httpResponse == null) return;
		
		if (content == null || content.trim().isEmpty()) {
			httpResponse.setContentLength(0);
			return;
		}

		try {
			if (contentType != null && !contentType.trim().isEmpty()) 
					httpResponse.setContentType(
							contentType + ((charSet == null || charSet.trim().isEmpty()) ? "" : ("; " + charSet)));

			httpResponse.setContentLength(content.length());
			
			final BufferedWriter writer = new BufferedWriter(
					new OutputStreamWriter(httpResponse.getOutputStream()));
			
			writer.write(content);
			writer.flush();
		} catch (Exception e) {
			// propagate
			throw new RuntimeException("Failed to stream response", e);
		}
	}
	
	public static void streamFileContent(
			HttpServletResponse httpResponse,
			String contentType,
			String charSet,
			File content) throws IOException {
		if (httpResponse == null) return;

		if (content == null || !content.exists()) {
			httpResponse.sendError(404);
			return;
		}
		
		// read the entire file which won't be too big
		StringBuffer buffer = new StringBuffer((int)content.length());
		DataInputStream reader = null;
		
		try {
			reader = new DataInputStream(new FileInputStream(content));
			byte buf[] = new byte[1024];
			int bytesRead = 0;
			
			do {
				buffer.append(new String(buf, 0, bytesRead));
			} while ((bytesRead = reader.read(buf)) > 0);
			
			// delegate rest of work
			ServiceUtils.streamString(httpResponse, contentType, charSet, buffer.toString());
		} catch (Exception e) {
			// propagate
			throw new RuntimeException("Failed to read file", e);
		} finally {
			try {
				reader.close();
			} catch (Exception e) {
				// can be safely ignored
			}
		}
	}
}
