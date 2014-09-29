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
#include "imaging.h"

const bool tissuestack::imaging::TissueStackDataBaseData::isRaw() const
{
	return false;
}

tissuestack::imaging::TissueStackDataBaseData::TissueStackDataBaseData(
		const unsigned long long int id, const std::string filename) :
		tissuestack::imaging::TissueStackImageData(id, filename) {}

tissuestack::imaging::TissueStackDataBaseData::~TissueStackDataBaseData()
{
	// do nothing for now
}
