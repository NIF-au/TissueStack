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

@XmlRootElement(name="Response", namespace=IGlobalConstants.XML_NAMESPACE)
public class Response implements IResponse {

	@XmlElement(name="ApplicationError", namespace=IGlobalConstants.XML_NAMESPACE)
	private ApplicationError applicationError;
	private Object response = new NoResults();

	public Response() {}
	
	public Response(Object response) {
		this.setResponse(response);
	}

	public Object getResponse() {
		return this.response;
	}

	public ApplicationError getError() {
		return this.applicationError;
	}

	public boolean hasErrorOccured() {
		if (this.getError() != null) {
			return true;
		}
		return false;
	}
	
	public void setResponse(Object response) {
		this.response = response;
	}

	public void setError(ApplicationError error) {
		this.applicationError = error;
	}
}
