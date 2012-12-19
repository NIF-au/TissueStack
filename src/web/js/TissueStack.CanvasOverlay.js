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
TissueStack.CanvasOverlay = function(canvas, protocol, host, dataset_id, dataset_plane_id) {
	if (typeof(canvas) == 'undefined')
		throw new Error("CanvasOverlay: argument canvas is undefined!");
		
	if (typeof(protocol) != 'string' && typeof(host) != 'string')
		throw new Error("CanvasOverlay: protocol and host have to be strings!");

	if (typeof(dataset_id) != 'number' && typeof(dataset_plane_id) != 'number')
		throw new Error("CanvasOverlay: ids have to be numeric!");
	
	this.canvas = canvas;
	this.dataset_id = dataset_id;
	this.dataset_plane_id = dataset_plane_id;
	this.mappingsUrl = protocol + "://" + host + "/" + TissueStack.configuration['restful_service_proxy_path'].value
	+ "/overlays/id_mapping_for_slice/" + this.dataset_id + "/" + this.dataset_plane_id + "/" + this.type;
	this.overlayUrl = 
		protocol + "://" + host + "/" + TissueStack.configuration['restful_service_proxy_path'].value + "/overlays/overlay/";
	
	// retrieve all overlays ids and their mapping to each slice
	this.queryOverlayMappingsForSlices();
};

TissueStack.CanvasOverlay.prototype = {
	type: "CANVAS",
	canvas: null,
	mappingsUrl : null,
	overlayUrl : null,
	dataset_id : null,
	dataset_plane_id: null,
	slices: null,
	error: null,
	queryOverlayMappingsForSlices : function() {
		(function(__this) {
			TissueStack.Utils.sendAjaxRequest(
				__this.mappingsUrl, 'GET',	true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						__this.error = "Did not receive anyting, neither success nor error ....";
						return;
					}
					if (data.error) {
						var message = "Application Error: " + (data.error.message ? data.error.message : " no more info available. check logs.");
						__this.error = message;
						return;
					}
					if (data.response.noResults)
						return;
					
					__this.slices = data.response; 
				},
				function(jqXHR, textStatus, errorThrown) {
					__this.error =  "Error connecting to backend: " + textStatus + " " + errorThrown;
				}				
		);})(this);
	},
	fetchOverlayForSlice : function(slice, handler) {
		if (typeof(slice) != "number")
			return;
		
		// complete request url with overlay id
		var url = this.overlayUrl;
		url += this.slices["" + slice];
		url += ("/" + this.type + "/json");

		TissueStack.Utils.sendAjaxRequest(
				url, 'GET',	true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						// nothing we can do 
						return;
					}
					if (data.error || data.response.noResults) {
						// nothing we can do 
						return;
					}
					
					// execute success handler
					if (handler) handler(); 
				},
				function(jqXHR, textStatus, errorThrown) {
					// nothing we can do 
				}				
		);
	},
	drawMe : function() {
		if (!this.slices && this.error) // retry if we had an error 
			this.queryOverlayMappingsForSlices();
		
		if (!this.slices)
			return;
		
		// TODO: implement me
		var handler = function() {
			// draw me
		};
		this.fetchOverlayForSlice(1, handler);
	}
};
