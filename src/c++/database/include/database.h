#ifndef	__DATABASE_H__
#define __DATABASE_H__

#include "logging.h"
#include <iostream>
#include <pqxx/pqxx>

namespace tissuestack
{
	namespace database
	{
		class TissueStackPostgresConnector final
		{
			public:
				TissueStackPostgresConnector & operator=(const TissueStackPostgresConnector&) = delete;
				TissueStackPostgresConnector(const TissueStackPostgresConnector&) = delete;
				~TissueStackPostgresConnector();
				static TissueStackPostgresConnector * instance(
						const std::string host = "localhost",
						const short port = 5432,
						const std::string password = "tissuestack");
		    	void purgeInstance();
		    	const bool isConnected() const;
			private:
		    	TissueStackPostgresConnector(
		    			const std::string host,
		    			const short port,
		    			const std::string password,
		    			const std::string database = "tissuestack",
		    			const std::string user = "tissuestack");
				static TissueStackPostgresConnector * _instance;
				pqxx::connection * _connection = nullptr;
	 	};
	}
}

#endif	/* __DATABASE_H__ */
