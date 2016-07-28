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
TissueStack.SVGOverlay = function(id, canvas, protocol, host, dataset_id, dataset_plane_id) {
	if (typeof(id) != 'number')
		throw new Error("SVGOverlay: argument id is not a number!");

	if (typeof(canvas) == 'undefined')
		throw new Error("SVGOverlay: argument canvas is undefined!");

	if (typeof(protocol) != 'string' && typeof(host) != 'string')
		throw new Error("SVGOverlay: protocol and host have to be strings!");

	if (typeof(dataset_id) != 'number' && typeof(dataset_plane_id) != 'number')
		throw new Error("SVGOverlay: ids have to be numeric!");

	this.pure_id = id;
	this.id = canvas.canvas_id + "_overlay_" + id;
	this.canvas = canvas;
	this.dataset_id = dataset_id;
	this.dataset_plane_id = dataset_plane_id;
	this.mappingsUrl = protocol + "://" + host + "/" + TissueStack.configuration['server_proxy_path'].value
	+ "/overlays/id_mapping_for_slice/" + this.dataset_id + "/" + this.dataset_plane_id + "/" + this.type;
	this.overlayUrl =
		protocol + "://" + host + "/" + TissueStack.configuration['server_proxy_path'].value + "/overlays/overlay/";

	// create canvas element
	this.createCanvasElement();

	// retrieve all overlays ids and their mapping to each slice
	this.queryOverlayMappingsForSlices();
};

TissueStack.SVGOverlay.prototype = {
	pure_id: null,
	id: null,
	type: "SVG",
	canvas: null,
	mappingsUrl : null,
	overlayUrl : null,
	dataset_id : null,
	dataset_plane_id: null,
	slices: null,
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
	queryOverlayMappingsForSlices : function() {
		(function(__this) {
			TissueStack.Utils.sendAjaxRequest(
					__this.mappingsUrl, 'GET',
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
		var sliceMap = this.slices[''+slice];
		if (typeof(sliceMap) === 'undefined')
			return;

		var url = this.overlayUrl;
		url += sliceMap;
		url += ("/" + this.type + "/json");

		TissueStack.Utils.sendAjaxRequest(
				url, 'GET',
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

		if (!this.slices && this.error) // retry if we had an error
			this.queryOverlayMappingsForSlices();

		if (!this.slices)
			return;

		// TODO: implement me
		var handler = function() {
			// draw me
		};
		this.fetchOverlayForSlice(this.canvas.data_extent.slice, handler);
	}
};
