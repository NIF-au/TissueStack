package au.edu.cai.cl;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public final class TissueStackCLCommunicator {
	
	public static String sendHttpRequest(
			final URL TissueStackServer, 
			final String actionAppendix,
			final String request) throws Exception {
	
		final URL combinedURL = new URL(TissueStackServer.toString() + actionAppendix);
		final HttpURLConnection connection = (HttpURLConnection) combinedURL.openConnection();
		
		byte [] data = (request != null) ? request.getBytes() : null; 
		
		connection.setRequestMethod("GET");
		connection.setUseCaches(false);
		
		// send request data if exists
		if (data != null) {
			connection.setRequestProperty("Content-Length", Integer.toString(data.length));
			connection.setDoOutput(true);
			
			DataOutputStream out = null;
			try {
				out = new DataOutputStream (connection.getOutputStream());
				out.write(data);
			} finally {
				try {
					out.close();
				} catch (Exception ignored) {}
			}
		}

		// receive response
		connection.setDoInput(true);
		InputStream in = null;
		StringBuilder respBuffer = new StringBuilder();
		try {
			in = connection.getInputStream();
			final BufferedReader reader = new BufferedReader(new InputStreamReader(in));
		    final char [] buffer = new char[1024];
		    int charRead = -1;
		    while((charRead = reader.read(buffer)) != -1)
		      respBuffer.append(buffer, 0, charRead);
		} finally {
			try {
				in.close();
			} catch (Exception ignored) {}
		}
		
		return respBuffer.toString();
	}
}
