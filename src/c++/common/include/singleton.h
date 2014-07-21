#ifndef	__SINGLETON_H__
#define __SINGLETON_H__

namespace tissuestack
{
	tissuestack::logging::TissueStackLogger * LoggerSingleton = tissuestack::logging::TissueStackLogger::instance();
	extern tissuestack::logging::TissueStackLogger * LoggerSingleton;
}

#endif
