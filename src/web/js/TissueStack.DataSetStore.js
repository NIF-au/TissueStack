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
TissueStack.DataSetStore = function(afterLoadingRoutine, omitLocalDataFetch) {
	if (!omitLocalDataFetch) {
		// fetch all data for the local configuration
		this.fetchDataSetsFromServer("localhost", null,  afterLoadingRoutine);
	}
};

TissueStack.DataSetStore.prototype = {
	datasets : {},
	datasetCount : 0,
	getSize : function() {
		return this.datasetCount;
	},
	getDataSetById : function(id) {
		if (typeof(id) != 'string' || this.getSize() == 0) {
			return null;
		}
		return this.datasets[id];
	},
	// slow !! don't use unless really necessary. rather loop with for ... in 
	getDataSetByIndex : function(index) {
		if (typeof(index) != 'number' || index < 0 || index > this.getSize() || this.getSize() == 0) {
			return null;
		}
		var counter = 0;
		for (var key in this.datasets) {
			if (counter == index) {
				return this.datasets[key];
			}
			counter++;
		}
	},
	addDataSetToStore : function(dataSet, host) {
		if (!dataSet || !dataSet.planes) {
			return null;
		}
		
		// coin id
		var id = dataSet.id;
		if (!host) {
			host = "localhost";
		}
		id = host + "_" + id;
		
		// check if we exist already in the tree, if so issue an alert
		if (this.datasets[id]) {
			alert("Data Set with Id " + id + " exists already!");
			return null;
		}
		
		this.datasets[id] = {};
		this.datasets[id].host = host;
		this.datasets[id].id = id;
		this.datasets[id].local_id = dataSet.id;
		this.datasets[id].description = dataSet.description ? dataSet.description : "";
		this.datasets[id].filename = dataSet.filename ? dataSet.filename : "";
		// this is the data for initialization
		this.datasets[id].data = dataSet.planes;
		// this is the map node where we store the actual runtime canvases which won't be set until we decide to display the data set
		this.datasets[id].planes = {};
		// this is the map node where we store the real world extents for the actual runtime canvases once they have been created
		this.datasets[id].realWorldCoords = {};
		this.datasetCount += 1;
		
		return this.datasets[id];
	}, fetchDataSetsFromServer : function(host, id, customSuccessHandler) {
		if (!host) {
			host = "localhost";
		}
		var url = (host == 'localhost' ? "" : "http://" + host) + "/" + TissueStack.configuration['restful_service_proxy_path'].value + "/data";
		if (!id && host == "localhost") {
			url += "/list?include_plane_data=true";
		} else if (!id && host != "localhost") {
			url += "/list";
		} else if (id) {
			url += ("/" + id);
		}
		
		_this = this;
		
		TissueStack.Utils.sendAjaxRequest(
				url, 'GET',	true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						alert("Did not receive anyting, neither success nor error ....");
						return;
					}
					
					if (data.error) {
						var message = "Application Error: " + (data.error.message ? data.error.message : " no more info available. check logs.");
						alert(message);
						return;
					}
					
					if (data.response.noResults) {
						alert("No data sets found in configuration database");
						if (customSuccessHandler) customSuccessHandler();
						
						return;
					}
					
					var dataSets = data.response;
					
					for (var x=0;x<dataSets.length;x++) {
						_this.addDataSetToStore(dataSets[x], "localhost");
					}

					if (customSuccessHandler) customSuccessHandler();
				},
				function(jqXHR, textStatus, errorThrown) {
					alert("Error connecting to backend: " + textStatus + " " + errorThrown);
				}				
		);
	}
};
