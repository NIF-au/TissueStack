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
#ifndef	__TS_PARAMETERS_H__
#define __TS_PARAMETERS_H__

namespace tissuestack
{
	namespace  database
	{
		class Configuration; // forward declaration
	}
	class TissueStackConfigurationParameters final
	{
		public:
			const std::string getParameter(const std::string & name) const;
			static const bool doesInstanceExist();
			static TissueStackConfigurationParameters * instance();
			void readInConfigurationFile(const std::string & configuration_file);
			void purgeInstance();
			void dumpConfigurationToDebugLog() const ;
		private:
			void addOrReplaceConfiguration(tissuestack::database::Configuration * conf);
			inline tissuestack::database::Configuration * findConfiguration(const std::string & name) const;
			TissueStackConfigurationParameters();
			const std::string eliminateWhitespaceAndUnwantedEscapeCharacters(const std::string & someString) const;
			std::unordered_map<std::string, tissuestack::database::Configuration *> _parameters;
			static TissueStackConfigurationParameters * _instance;
	};
}

#endif	/* __TS_PARAMETERS_H__ */
