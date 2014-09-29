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
#include "networking.h"

tissuestack::networking::RawHttpRequest::~RawHttpRequest()
{
	// not doing anything at the moment
};

tissuestack::networking::RawHttpRequest::RawHttpRequest(const std::string raw_content) : _content(raw_content)
{
	this->setType(tissuestack::common::Request::Type::RAW_HTTP);
};

const std::string tissuestack::networking::RawHttpRequest::getContent() const
{
	return this->_content;
};

const bool tissuestack::networking::RawHttpRequest::isObsolete() const
{
	// at this level we are false by default
	return false;
}
