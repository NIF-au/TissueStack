TissueStack.DataSetStore = function() {
	// initialize store
	this.datasets = {};
	
	// fetch all data for the local configuration
	this.fetchDataSetsFromServer("localhost");
};

TissueStack.DataSetStore.prototype = {
	datasets : {},
	datasetCount : 0,
	getSize : function() {
		return this.datasetCount;
	},
	addDataSetToStore : function(dataSet, host) {
		if (!dataSet || !dataSet.planes) {
			return;
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
			return;
		}
		
		this.datasets[id] = {};
		this.datasets[id].id = id;
		this.datasets[id].description = dataSet.description ? dataSet.description : "";
		// this is the data for initialization
		this.datasets[id].data = dataSet.planes;
		// this is the map node where we store the actual runtime canvases which won't be set until we decide to display the data set
		this.datasets[id].planes = {};
		// this is the map node where we store the real world extents for the actual runtime canvases once they have been created
		this.datasets[id].realWorldCoords = {};
	}, fetchDataSetsFromServer : function(host, id, customSuccessHandler) {
		if (!host) {
			host = "localhost";
		}
		var url = "http://" + host + "/backend/data";
		if (!id && host == "localhost") {
			url += "/list?include_plane_data=true";
		} else if (!id && host != "localhost") {
			url += "/list";
		} else if (id) {
			url += ("/" + id);
		}
		
		$.ajax({
			url : url,
			data : "json",
			success: function(data, textStatus, jqXHR) {
				// TODO: handle success
				// loop through results and call addDataSetToStore()
				
				if (customSuccessHandler) {
					customSuccessHandler();
				}
			},
			error: function(jqXHR, textStatus, errorThrown) {
				// TODO: handle error
			}
		});
	}
};
