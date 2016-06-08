package au.edu.cai.cl;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;

public final class TissueStackCLCommunicator {
	
	public static String sendHttpRequest(final URL url, final String request) throws Exception {
	
		try {
			final HttpURLConnection connection = (HttpURLConnection) url.openConnection();
	
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
		} catch(java.io.IOException io) {
			if (io.getMessage().indexOf("HTTP response code: 408") != -1) {
				return
					"{\"error\": {\"description\": \"Request obsolete, i.e. it's been/being processed already\"}}";
			} else throw io;
		}
	}
	

	public static String encodeQueryStringParam(String param) {
		 try {
	        return URLEncoder.encode(param, "UTF-8")
	                .replaceAll("\\+", "%20")
	                .replaceAll("\\%21", "!")
	                .replaceAll("\\%27", "'")
	                .replaceAll("\\%28", "(")
	                .replaceAll("\\%29", ")")
	                .replaceAll("\\%7E", "~")
	                .replaceAll("\\%2C", ",")
	 	            .replaceAll("\\%5B", "[")
	                .replaceAll("\\%5D", "]")
	                .replaceAll("\\%5C", "\\")
	                .replaceAll("\\%40", "@")
	                .replaceAll("\\%7C", "|")
	                .replaceAll("\\%5E", "^");
		    } catch (Exception e) {
		    	System.err.println("Failed to encode query string param");
		    }
		 return null;
	}
}
