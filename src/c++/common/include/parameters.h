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
