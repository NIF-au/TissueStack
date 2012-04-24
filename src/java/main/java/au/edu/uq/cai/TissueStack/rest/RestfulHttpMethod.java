package au.edu.uq.cai.TissueStack.rest;

public enum RestfulHttpMethod {
	GET("javax.ws.rs.GET"), POST("javax.ws.rs.POST"), PUT("javax.ws.rs.PUT"), DELETE("javax.ws.rs.DELETE"), PATH("javax.ws.rs.Path");
		
	private String method;
	
	private RestfulHttpMethod(String method) {
		if (method == null || method.isEmpty()) {
			throw new IllegalArgumentException("Parameter 'method' must not be null or empty");
		}
		this.method = method;
	}
	
	public String getMethod() {
		return this.method;
	}
}
