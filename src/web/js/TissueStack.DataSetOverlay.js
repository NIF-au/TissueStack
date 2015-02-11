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
TissueStack.DataSetOverlay = function(id, canvas, protocol, host, dataset_id, dataset_plane_id, dataSetHost) {
	if (typeof(id) != 'number')
		throw new Error("DataSetOverlay: argument id is not a number!");
	
	if (typeof(canvas) == 'undefined')
		throw new Error("DataSetOverlay: argument canvas is undefined!");
		
	if (typeof(protocol) != 'string' && typeof(host) != 'string')
		throw new Error("DataSetOverlay: protocol and host have to be strings!");

	if (typeof(dataset_id) != 'number' && typeof(dataset_plane_id) != 'number')
		throw new Error("DataSetOverlay: ids have to be numeric!");
	
	this.pure_id = id;
	this.id = canvas.canvas_id + "_overlay_" + id; 
	this.canvas = canvas;
	this.dataset_id = dataset_id;
	this.dataset_plane_id = dataset_plane_id;
	this.host = host;
	this.dataSetHost = dataSetHost;
	this.mappingsUrl = protocol + "://" + host + "/" + TissueStack.configuration['server_proxy_path'].value
	+ "/overlays/id_mapping_for_slice/" + this.dataset_id + "/" + this.dataset_plane_id + "/" + this.type;
	this.overlayUrl = 
		protocol + "://" + host + "/" + TissueStack.configuration['server_proxy_path'].value + "/overlays/overlay/";
	this.dataSetUrl = 
		protocol + "://" + host + "/" + TissueStack.configuration['server_proxy_path'].value + "/data/";

	// create canvas element
	this.createCanvasElement();

	// retrieve all overlays ids and their mapping to each slice
	this.queryLinkedDataSetMapping();
};

TissueStack.DataSetOverlay.prototype = {
	pure_id: null,
	dataSetHost : null,
	id: null,
	type: "DATASET",
	canvas: null,
	mappingsUrl : null,
	overlayUrl : null,
	dataSetUrl : null,
	host : null,
	dataset_id : null,
	dataset_plane_id: null,
	linkedDataSetIds: null,
	linkedCanvas: null,
	error: null,
	selected: false,
	getMyOwnCanvasElement: function() {
		return $('#' + this.id);
	},
	createCanvasElement : function() {
		var myOwnCanvasElement = this.getMyOwnCanvasElement(); 
		if (!myOwnCanvasElement || (myOwnCanvasElement && myOwnCanvasElement.length == 0)) {
			// get parent of canvas and append overlay to it
			$('#' + this.canvas.canvas_id).parent().append(
					'<canvas id="' + this.id + '" style="z-index: ' + (800 + this.pure_id) + '"'
					+ ' width="' + this.canvas.getCanvasElement().attr("width") + '" height="' + this.canvas.getCanvasElement().attr("height") + '"'
					+ ' class="overlay"></canvas>');
		}
	},
	queryLinkedDataSetMapping : function() {
		(function(__this) {
			TissueStack.Utils.sendAjaxRequest(
					__this.mappingsUrl, 'GET',	true,
					function(data, textStatus, jqXHR) {
						if (!data.response && !data.error) {
							__this.error = "Did not receive anyting, neither success nor error ....";
							return;
						}
						if (data.error) {
							var message = "Application Error: " + (data.error.description ? data.error.description : " no more info available. check logs.");
							__this.error = message;
							return;
						}
						if (data.response.noResults)
							return;
						
						// in this case we need only one record since all slices will refer to another data set
						for (var key in data.response) {
							__this.fetchLinkedDataSetIds(data.response[key]);
							break;
						}
					},
					function(jqXHR, textStatus, errorThrown) {
						__this.error =  "Error connecting to backend: " + textStatus + " " + errorThrown;
					}				
			);})(this);
	},
	fetchLinkedDataSetIds : function(id) {
		// complete request url with overlay id
		var url = this.overlayUrl;
		url += id;
		url += ("/" + this.type + "/json");

		(function(__this) {
			TissueStack.Utils.sendAjaxRequest(
					url, 'GET',	true,
					function(data, textStatus, jqXHR) {
						if (!data.response && !data.error) {
							__this.error = "Did not receive anyting, neither success nor error ...."; 
							return;
						}
	
						if (data.error) {
							var message = "Application Error: " + (data.error.description ? data.error.description : " no more info available. check logs.");
							__this.error = message;
							return;
						}
						if (data.response.noResults) {
							__this.error = null;
							return
						}
						
						__this.linkedDataSetIds = {};
						__this.linkedDataSetIds['id'] =  data.response.linkedDataSet;
						__this.linkedDataSetIds['plane_id'] =  data.response.linkedDataSetPlane;
					},
					function(jqXHR, textStatus, errorThrown) {
						__this.error =  "Error connecting to backend: " + textStatus + " " + errorThrown;
					}				
			);})(this);
	},
	queryLinkedDataSet : function() {
		// complete request url with overlay id
		var url = this.dataSetUrl;
		url += this.linkedDataSetIds['id'];
		
		(function(__this) {
			TissueStack.Utils.sendAjaxRequest(
					url, 'GET',	true,
					function(data, textStatus, jqXHR) {
						if (!data.response && !data.error) {
							__this.error = "Did not receive anyting, neither success nor error ....";
							return;
						}
						if (data.error) {
							var message = "Application Error: " + (data.error.description ? data.error.description : " no more info available. check logs.");
							__this.error = message;
							return;
						}
						if (data.response.noResults)
							return;
						
						var dataSetPlane = null; 
						
						// find mapping plane 
						if (data.response.planes)
							for (var i=0; i < data.response.planes.length; i++) {
								if (data.response.planes[i].id === __this.linkedDataSetIds['plane_id']) {
									dataSetPlane = data.response.planes[i];
									break;
								}
							}
						
						if (!dataSetPlane)
							return;

						// create canvas and its associated objects from info received
						var planeId = dataSetPlane.name;
						var zoomLevels = eval(dataSetPlane.zoomLevels);
						var transformationMatrix = eval(dataSetPlane.transformationMatrix);
						
						// create extent
						var extent = 
							new TissueStack.Extent(
								__this.dataSetHost + "_" + data.response.id,
								dataSetPlane.isTiled,
								dataSetPlane.oneToOneZoomLevel,
								planeId,
								dataSetPlane.maxSlices,
								dataSetPlane.maxX,
								dataSetPlane.maxY,
								dataForPlane.origX,
								dataForPlane.origY,
								dataForPlane.step,
								zoomLevels,
								transformationMatrix, 
								dataSetPlane.resolutionMm);

						var linkedCanvasId = 
							__this.canvas.canvas_id.substring(__this.canvas.dataset_id.length+1, __this.canvas.canvas_id.length)
							+ "_overlay_" + __this.pure_id;
						__this.linkedCanvas = new TissueStack.Canvas(extent, linkedCanvasId, __this.canvas.dataset_id, true, true);
					},
					function(jqXHR, textStatus, errorThrown) {
						__this.error =  "Error connecting to backend: " + textStatus + " " + errorThrown;
					}				
			);})(this);
	},
	select : function() {
		this.selected = true;
		this.getMyOwnCanvasElement().show();
	},
	deselect : function() {
		this.selected = false;
		this.getMyOwnCanvasElement().hide();
	},
	drawMe : function() {
		// only do work if we have been selected
		if (!this.selected)
			return;
		
		if (!this.linkedDataSetIds && this.error) // retry if we had an error 
			this.queryLinkedDataSetMapping();
		
		if (!this.linkedCanvas && this.linkedDataSetIds) // retry
			this.queryLinkedDataSet();
			
		if (this.linkedCanvas)
			this.linkedCanvas.drawMe();
	}
};
