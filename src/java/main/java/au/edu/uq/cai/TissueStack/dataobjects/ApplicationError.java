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

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(namespace=IGlobalConstants.XML_NAMESPACE)
public class ApplicationError {
	private String description = "Unspecified Error";
	private String exception = "";
	private String message = "";

	private ApplicationError() {}
	
	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getDescription() {
		return this.description;
	}

	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getException() {
		return this.exception;
	}

	@XmlElement(namespace=IGlobalConstants.XML_NAMESPACE)
	public String getMessage() {
		return this.message;
	}

	public static ApplicationError createMinimalApplicationError() {
		return new ApplicationError();
	}

	public static ApplicationError createApplicationError(String description, Throwable exception) {
		final ApplicationError jsonError = ApplicationError.createMinimalApplicationError();
		if (description != null && description.trim().length()  > 0) {
			jsonError.description = description;
		}
		if (exception != null) {
			jsonError.exception = exception.getClass().getName();
			if (exception.getMessage() != null) {
				jsonError.message = exception.getMessage();
			}
		}
		
		return jsonError;
	}
}