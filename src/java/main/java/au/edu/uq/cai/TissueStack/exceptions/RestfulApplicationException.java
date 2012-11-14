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
package au.edu.uq.cai.TissueStack.exceptions;

import org.jboss.resteasy.spi.ApplicationException;

public class RestfulApplicationException extends ApplicationException {

	private static final long serialVersionUID = -5385803989710375180L;

	public RestfulApplicationException(String s, Throwable throwable) {
		super(s, throwable);
	}

	public RestfulApplicationException(String s) {
		super(s, null);
	}

}
