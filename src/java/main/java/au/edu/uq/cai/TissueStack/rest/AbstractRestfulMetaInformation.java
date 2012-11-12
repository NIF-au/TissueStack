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
package au.edu.uq.cai.TissueStack.rest;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;

import javax.ws.rs.MatrixParam;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;

import au.edu.uq.cai.TissueStack.dataobjects.RestfulMetaInformation;
import au.edu.uq.cai.TissueStack.dataobjects.RestfulMetaInformation.RestfulResource;
import au.edu.uq.cai.TissueStack.dataobjects.RestfulMetaInformation.RestfulResourceParam;

public abstract class AbstractRestfulMetaInformation {
	
	@SuppressWarnings("unchecked")
	public RestfulMetaInformation getMetaInfo() {
		final RestfulMetaInformation metaInfo = new RestfulMetaInformation();
		
		final Class<AbstractRestfulMetaInformation> clazz = (Class<AbstractRestfulMetaInformation>) this.getClass();
		final String resourcePath = clazz.getAnnotation(Path.class).value();
		final Description resourceDescription = clazz.getAnnotation(Description.class);
		
		metaInfo.setPath(resourcePath);
		if (resourceDescription != null) {
			metaInfo.setDescription(resourceDescription.value());
		}

		final Method declaredMethods[] = clazz.getDeclaredMethods();
		for (Method method : declaredMethods) {
			final RestfulResource restResource = new RestfulResource();

			final Annotation annotations[] = method.getAnnotations();
			final Annotation parameterAnnotations[][] = method.getParameterAnnotations();
			if (this.isAnnotatedResource(annotations)) {
				restResource.setMethodName(method.getName());

				Annotation tmpAnnotation = method.getAnnotation(Description.class);
				if (tmpAnnotation != null) {
					restResource.setDescription(((Description) tmpAnnotation).value());
				}
				
				tmpAnnotation = method.getAnnotation(Path.class);
				if (tmpAnnotation != null) {
					restResource.setSubPath(((Path) tmpAnnotation).value());
				}
				
				
				final String httpMethod = this.getHttpMethod(annotations);
				if (httpMethod != null) {
					restResource.setHttpMethod(httpMethod);
				}

				String outputFormat = "";
				tmpAnnotation = method.getAnnotation(Produces.class);
				if (tmpAnnotation != null) {
 
					for (String produces : ((Produces)tmpAnnotation).value()) {
						outputFormat += (produces + ", ");  
					}
				} else {
					outputFormat = this.findOutputFormatForRetrunType(method.getReturnType(), outputFormat);
				}
				if (outputFormat.endsWith(", ")) {
					outputFormat = outputFormat.substring(0, outputFormat.length()-2);
				}
				restResource.setOutputFormat(outputFormat);

				
				final Class<?> parameters[] = method.getParameterTypes();
				if (parameters.length > 0) {
					int index = 0;
					for (Class<?> param : parameters) {
						// define 3 types of interest
						final String parameterTypes[] = new String[] {PathParam.class.getName(), MatrixParam.class.getName(), QueryParam.class.getName()};
						for (String parameterType : parameterTypes) {
							tmpAnnotation = this.getParameterAnnotationsForGivenParameter(parameterType, parameterAnnotations, index);
							if (tmpAnnotation != null) {
								final RestfulResourceParam retfulResourceParam = new RestfulResourceParam();
								if (tmpAnnotation instanceof PathParam) {
									retfulResourceParam.setName(((PathParam) tmpAnnotation).value());
								} else if (tmpAnnotation instanceof MatrixParam) {
									retfulResourceParam.setName(((MatrixParam) tmpAnnotation).value());
								} else if (tmpAnnotation instanceof QueryParam) {
									retfulResourceParam.setName(((QueryParam) tmpAnnotation).value());
								}
								
								retfulResourceParam.setParamType(tmpAnnotation.annotationType().getSimpleName());
								retfulResourceParam.setInputType(param.getName());
								
								tmpAnnotation = this.getParameterAnnotationsForGivenParameter(Description.class.getName(), parameterAnnotations, index);
								if (tmpAnnotation != null) {
									retfulResourceParam.setDescription(((Description)tmpAnnotation).value());
								}
								restResource.addParameter(retfulResourceParam);
							}
						}
						index++;
					}
				}
			}
			
			metaInfo.addRestfulResource(restResource);
		}
		
		return metaInfo;
	}
	
	private String findOutputFormatForRetrunType(Class<?> clazz, String outputFormat) {
		if (clazz == null) {
			return "";
		}
		
		if (outputFormat == null) {
			outputFormat = new String("");
		}
		
		final Method declaredMethods[] = clazz.getDeclaredMethods();
		for (Method method : declaredMethods) {
			final Annotation annotations[] = method.getAnnotations();
			if (this.isAnnotatedResource(annotations)) {
				final Annotation producesAnnotation = method.getAnnotation(Produces.class);
				if (producesAnnotation != null) {
					for (String produces : ((Produces)producesAnnotation).value()) {
						outputFormat += produces;
						
						final Annotation pathAnnotation = method.getAnnotation(Path.class);
						if (pathAnnotation != null) {
							outputFormat += " [use : " + (((Path) pathAnnotation).value()) + "]";
						}

						outputFormat += ", ";  
					}
				}
				
			}
		}

		
		return outputFormat;
	}
	
	private boolean isAnnotatedResource(Annotation annotations[]) {
		for (Annotation annotation : annotations) {
			if (RestfulHttpMethod.PATH.getMethod().equals(annotation.annotationType().getName())) {
				return true;
			} else if (this.getHttpMethod(annotation) != null) {
				return true;
			}
		}
		return false;
	}

	private String getHttpMethod(Annotation method) {
		final String annotationName = method.annotationType().getName();
		if (RestfulHttpMethod.GET.getMethod().equals(annotationName)) {
			return RestfulHttpMethod.GET.toString();
		} else if (RestfulHttpMethod.POST.getMethod().equals(annotationName)) {
			return RestfulHttpMethod.POST.toString();
		} else if (RestfulHttpMethod.PUT.getMethod().equals(annotationName)) {
			return RestfulHttpMethod.PUT.toString();
		} else if (RestfulHttpMethod.DELETE.getMethod().equals(annotationName)) {
			return RestfulHttpMethod.DELETE.toString();
		}
		return null;
	}
	
	private String getHttpMethod(Annotation annotations[]) {
		for (Annotation annotation : annotations) {
			final String httpMethod = this.getHttpMethod(annotation);
			if (httpMethod != null) {
				return httpMethod;
			}
		}
		return null;
	}
	
	private Annotation getParameterAnnotationsForGivenParameter(String annotationTypeName, Annotation parameterAnnotations[][], int index) {
		if (annotationTypeName == null || annotationTypeName.isEmpty()) {
			throw new IllegalArgumentException("Parameter 'annotationTypeName' must not be null or empty");
		}
		if (parameterAnnotations == null) {
			throw new IllegalArgumentException("Parameter 'parameterAnnotations' must not be null");
		}
		if (index < 0) {
			throw new IllegalArgumentException("Parameter 'index' must be greater than 0");
		}
		final Annotation paramAnnotations[] = parameterAnnotations[index];
		for (Annotation annotation : paramAnnotations) {
			if (annotation.annotationType().getName().equals(annotationTypeName)) {
				return annotation;
			} 
		}
		return null;
	}
}
