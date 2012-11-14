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
package au.edu.uq.cai.TissueStack.dataobjects;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(name="RestfulMetaInformation", namespace=IGlobalConstants.XML_NAMESPACE)
public class RestfulMetaInformation {
	
	private List<RestfulResource> restfulResources = new ArrayList<RestfulResource>();
	private String path; 
	private String description;
	
	@XmlAttribute(name="Description", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getDescription() {
		return this.description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	public void addRestfulResource(RestfulResource restfulResource) {
		this.restfulResources.add(restfulResource);
	}
	
	@XmlAttribute(name="Path", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getPath() {
		return this.path;
	}

	public void setPath(String path) {
		this.path = path;
	}

	@XmlElement(name="RestfulResource", namespace=IGlobalConstants.XML_NAMESPACE)
	@XmlElementWrapper(name="RestfulResources", namespace=IGlobalConstants.XML_NAMESPACE)
	public List<RestfulResource> getRestfulResources() {
		return this.restfulResources;
	}

	public static class RestfulResource {
		private String methodName;
		private String subPath;
		private String httpMethod;
		private String outputFormat;
		private List<RestfulResourceParam> parameters = new ArrayList<RestfulResourceParam>();
		private String description;
		

		@XmlAttribute(name="SubPath", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getSubPath() {
			return this.subPath;
		}

		public void setSubPath(String subPath) {
			this.subPath = subPath;
		}

		@XmlAttribute(name="MethodName", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getMethodName() {
			return this.methodName;
		}

		public void setMethodName(String methodName) {
			this.methodName = methodName;
		}

		public void setHttpMethod(String httpMethod) {
			this.httpMethod = httpMethod;
		}

		@XmlAttribute(name="httpMethod", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getHttpMethod() {
			return this.httpMethod;
		}

		@XmlAttribute(name="OutputFormat", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getOutputFormat() {
			return this.outputFormat;
		}

		public void setOutputFormat(String outputFormat) {
			this.outputFormat = outputFormat;
		}

		
		@XmlAttribute(name="description", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getDescription() {
			return this.description;
		}

		public void setDescription(String description) {
			this.description = description;
		}

		public void addParameter(RestfulResourceParam parameter) {
			this.parameters.add(parameter);
		}

		@XmlElement(name="Parameter", namespace=IGlobalConstants.XML_NAMESPACE)
		@XmlElementWrapper(name="Parameters", namespace=IGlobalConstants.XML_NAMESPACE)
		public List<RestfulResourceParam> getParameters() {
			return this.parameters;
		}
	}
	
	@XmlRootElement(name="Parameter", namespace=IGlobalConstants.XML_NAMESPACE)
	public static class RestfulResourceParam {
		private String name;
		private String description;
		private String paramType;
		private String inputType;
		
		@XmlAttribute(name="Name", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getName() {
			return this.name;
		}

		public void setName(String name) {
			this.name = name;
		}
	
		@XmlAttribute(name="description", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getDescription() {
			return this.description;
		}

		public void setDescription(String description) {
			this.description = description;
		}
		
		@XmlElement(name="InputType", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getInputType() {
			return this.inputType;
		}

		public void setInputType(String inputType) {
			this.inputType = inputType;
		}
		
		@XmlElement(name="ParamType", namespace=IGlobalConstants.XML_NAMESPACE)
		public String getParamType() {
			return this.paramType;
		}

		public void setParamType(String paramType) {
			this.paramType = paramType;
		}

	}
	
	public String toString() {
		final StringBuilder metaInfo = new StringBuilder(1000);
		if (this.getDescription() != null) {
			metaInfo.append("Resource Description: " + this.getDescription() + "\n");
		}

		if (this.getPath() != null) {
			metaInfo.append("Resource Path: '" + this.getPath() + "'\n");
		}

		final List<RestfulResource> restfulResources = this.getRestfulResources();
		for (RestfulResource resource : restfulResources) {
			if (resource.getMethodName() != null) {
				metaInfo.append("\tResource Method: '" + resource.getMethodName() + "'\n");
			}
			if (resource.getDescription() != null) {
				metaInfo.append("\tDescription: " + resource.getDescription() + "\n");
			}
			if (resource.getHttpMethod() != null) {
				metaInfo.append("\tHttp: '" + resource.getHttpMethod() + "'\n");
			}
			if (resource.getSubPath() != null && !resource.getSubPath().equals("/")) {
				metaInfo.append("\tSub Path: '" + resource.getSubPath() + "'\n");
			}
			if (resource.getOutputFormat() != null) {
				metaInfo.append("\tOutput: '" + resource.getOutputFormat() + "'\n");
			}
			
			List<RestfulResourceParam> restfulResourceParams = resource.getParameters();
			for (RestfulResourceParam resourceParam : restfulResourceParams) {
				if (resourceParam.getParamType() != null) {
					metaInfo.append("\t\t" + resourceParam.getParamType() + ": ");
				} else {
					metaInfo.append("\t\tParameter: ");
				}

				if (resourceParam.getName() != null) {
					metaInfo.append("'" + resourceParam.getName() + "'\n");
				}
				
				if (resourceParam.getInputType() != null) {
					metaInfo.append("\t\tType: '" + resourceParam.getInputType() + "'\n");
				}

				if (resourceParam.getDescription() != null) {
					metaInfo.append("\t\tDescription: '" + resourceParam.getDescription() + "'\n");
				}
			}
			metaInfo.append("\n");
		}
		
		return metaInfo.toString();
	}
}
