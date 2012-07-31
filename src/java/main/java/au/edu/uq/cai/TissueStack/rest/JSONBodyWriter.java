package au.edu.uq.cai.TissueStack.rest;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.lang.annotation.Annotation;
import java.lang.reflect.Type;

import javax.ws.rs.Produces;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.MultivaluedMap;
import javax.ws.rs.ext.MessageBodyWriter;
import javax.ws.rs.ext.Provider;
import javax.xml.bind.annotation.XmlTransient;

import net.sf.json.JSONObject;
import net.sf.json.JsonConfig;
import net.sf.json.util.PropertyFilter;

@Provider
@Produces(MediaType.APPLICATION_JSON)
public class JSONBodyWriter implements MessageBodyWriter<Object> {

	public static JsonConfig jsonConfig;

	static {
		jsonConfig = new JsonConfig();
		jsonConfig.setIgnoreJPATransient(false);
		jsonConfig.addIgnoreFieldAnnotation(XmlTransient.class);
		jsonConfig.setJsonPropertyFilter( new PropertyFilter() {
		   public boolean apply( Object source, String name, Object value ) {  
		      if( value == null){  
		         return true;  
		      }  
		      return false;  
		   }  
		});
	}
	
	private String json;
	
	public long getSize(Object arg0, Class<?> arg1, Type arg2, Annotation[] arg3,
			MediaType arg4) {
		if (arg0 == null) {
			return 0;
		}
	
		this.json = JSONObject.fromObject(arg0, jsonConfig).toString();
		
		return this.json.length();
	}

	public boolean isWriteable(Class<?> arg0, Type arg1, Annotation[] arg2,
			MediaType arg3) {
		return arg3.equals(MediaType.APPLICATION_JSON_TYPE);
	}

	public void writeTo(Object arg0, Class<?> arg1, Type arg2, Annotation[] arg3,
			MediaType arg4, MultivaluedMap<String, Object> arg5,
			OutputStream arg6) throws IOException, WebApplicationException {
        final BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(arg6, "UTF-8"));
        
        arg5.add("Access-Control-Allow-Origin", "*");
        
        if (arg0 == null) {
        	throw new IOException("Response object is null");
        }

        writer.write(this.json);        
        writer.flush();
	}
	
	public static JsonConfig getJsonConfig() {
		return jsonConfig;
	}
}
