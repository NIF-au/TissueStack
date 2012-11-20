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
TissueStack = {
	requiredLibs : [
	                	"/js/libs/sylvester/sylvester.js",
	                	"/js/TissueStack.js",
	                	"/js/TissueStack.MouseWheel.js",
	                	"/js/TissueStack.Utils.js",
	                	"/js/TissueStack.Extent.js",
	                	"/js/TissueStack.Queue.js",
	                	"/js/TissueStack.Canvas.js",
	                	"/js/TissueStack.Events.js",
	                	"/js/TissueStack.DataSetStore.js"
	                ]
};

TissueStack.Embedded = function (div, server, data_set_id, include_cross_hair, use_image_service, initOpts) {
	// evaluate input and conduct peliminary checks
	
	if (typeof(div) != 'string' || $.trim(div) == '') {
		alert("Empty div ids are invalid!");
		return;
	}
	
	this.div = $("#" + div);
	if (this.div.length == 0) {
		alert("Given div id does not exist!");
		return;
	}
	
	if (typeof(server) != 'string' || $.trim(server) == '') {
		this.writeErrorMessageIntoDiv("Empty server name was given!");
		return;
	}
	this.domain = this.extractHostNameFromUrl(server);

	if (typeof(data_set_id) != 'number' || data_set_id <= 0) {
		this.writeErrorMessageIntoDiv("Data Set id has to be greater than 0!");
		return;
	}
	this.data_set_id = data_set_id;

	if (typeof(include_cross_hair) != 'boolean' || include_cross_hair == true) {
		this.include_cross_hair = true;
	} else {
		this.include_cross_hair = false;
	}

	if (typeof(use_image_service) == 'boolean' && use_image_service == true) {
		this.use_image_service = true;
	} else {
		this.use_image_service = false;
	}

	if (typeof(initOpts) == 'object') {
		this.initOpts = initOpts;
	} else {
		this.initOpts = null;
	}
	
	// check canvas support
	if (!this.supportsCanvas()) {
		this.writeErrorMessageIntoDiv("Sorry, your browser does not support the HTML 5 feature canvas");
		return;
	}
	
	var afterLoadActions = function(_this) {
        // load server configuration values needed
        _this.loadDataBaseConfiguration();
        // load given data set configuration
        _this.loadDataSetConfigurationFromServer();
        // this is for when the window dimensions & the div dimensions may change dynamically
        _this.registerWindowResizeEvent();
	};
	
	// include the java script and css that is needed for the rest of the functionality
	this.includeJavaScriptAndCssNeeded(afterLoadActions);
};

TissueStack.Embedded.prototype = {
	librariesLoaded : 0,
	getDiv : function() {
		return this.div;
	},
	writeErrorMessageIntoDiv : function(message) {
		this.getDiv().html(message);
	},
	supportsCanvas : function() {
	  var elem = document.createElement('canvas');
	  return !!(elem.getContext && elem.getContext('2d'));
	},
	extractHostNameFromUrl : function(url) {
		if (typeof(url) != "string") {
			return null;
		}

		// trim whitespace
		url = $.trim(url);

		// take off protocol 
		if (url.indexOf("http://") == 0 || url.indexOf("file://") == 0) {
			url = url.substring(7, url.length);
		} else if (url.indexOf("https://") == 0 || url.indexOf("file:///") == 0) {
			url = url.substring(8, url.length);
		} else if (url.indexOf("ftp://") == 0) {
			url = url.substring(6, url.length);
		}
		// strip it off a potential www
		if (url.indexOf("www.") == 0) {
			url = url.substring(4, url.length);
		}
			
		//now cut off anything after the initial '/' if exists to preserve the domain
		var slashPosition = url.indexOf("/");
		if (slashPosition > 0) {
			url = url.substring(0, slashPosition);
		}

		return url;
	},	
	includeJavaScriptAndCssNeeded : function(afterLoadActions) {
		// if header does not exist => create it
		var head = $("head");
		if (head.length == 0) {
			head = $("html").append("<head></head>");
		}

		// add all the css that's necessary
		head.append(this.createElement("link", "http://" + this.domain + "/css/default.css"));

		_this = this;
		
		var checkLoadOfLibraries = function() {
			_this.librariesLoaded++;
			if (_this.librariesLoaded == TissueStack.requiredLibs.length) {
				afterLoadActions(_this);
			}
		};
		
		var error = function() {
			alert("Failed to load required library!");
		};
		
		for (var i=0;i<TissueStack.requiredLibs.length;i++) {
			(function () {
				$.ajax({
					url : "http://" + _this.domain + TissueStack.requiredLibs[i],
					async: false,
					dataType : "script",
					cache : false,
					timeout : 30000,
					success : checkLoadOfLibraries,
					error: error
				});
			})();
		}
	},
	createElement : function(tag, value) {
		if (typeof(tag) != "string" || typeof(value) != "string" || tag.length == 0 || value.length == 0) {
			return null;
		}
		
		var newEl = document.createElement(tag);
		if (tag == "script") {
			newEl.src = value;
		} else if (tag == "link") {
			newEl.rel = "stylesheet";
			newEl.href = value;
		} else {
			newEl.value = value;
		}

		return newEl;
	},
	createHTMLForDataSet : function(dataSet, include_cross_hair) {
		// outer div
		var html = '<div id="dataset_1" class="dataset">';
		
		// loop over all planes in the data
		for (var i=0; i < dataSet.data.length; i++) {
			var planeId = dataSet.data[i].name;
			
			switch(i) {
				case 0: // first is main canvas (incl. scale bar)
					html += ('<div class="scalecontrol_main"><div id="dataset_1_scale_middle" class="scalecontrol_middle">'
						+ '<div class="dataset_1_scalecontrol_image" style="left: 0px; top: -424px; width: 89px;"></div></div>'
						+ '<div id="dataset_1_scale_left" class="scalecontrol_left">'
						+ '<div class="dataset_1_scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
						+ '<div id="dataset_1_scale_center_left" class="scalecontrol_center_left">'
	    				+ '<div class="dataset_1_scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    				+ '<div id="dataset_1_scale_center_right" class="scalecontrol_center_right">'
	    				+ '<div class="dataset_1_scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    				+ '<div id="dataset_1_scale_up" class="scalecontrol_up">'
    					+ '<div class="dataset_1_scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
	    				+ '<div id="dataset_1_scale_text_up" class="scalecontrol_text_up"></div></div>');
					
					html +=
							'<div id="dataset_1_main_view_canvas" class="canvasview canvas_' + planeId + '">'
						+ 	'<canvas id="dataset_1_canvas_' + planeId + '_plane" class="plane"></canvas>'
						+ (include_cross_hair ? 
								'<canvas id="dataset_1_canvas_'  + planeId + '_plane_cross_overlay" class="cross_overlay"></canvas>'
								: ''
						   );
					break;
				case 1:
					html +=
							'<div id="dataset_1_left_side_view_canvas" class="left_side_view ui-bar-a">'
						+	'<img id="dataset_1_left_side_view_maximize" class="canvas_' + planeId
						+ 	' maximize_view_icon" src="http://' + this.domain + '/images/maximize.png" alt="Maximize View" />'
						+	'<canvas id="dataset_1_canvas_' + planeId + '_plane" class="side_canvas"></canvas>'
						+ 	(include_cross_hair ? 
								'<canvas id="dataset_1_canvas_' + planeId +
								'_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>'
								: ''
							);
					break;
				case 2:
					html +=
							'<div id="dataset_1_right_side_view_canvas" class="right_side_view ui-bar-a">'
						+	'<img id="dataset_1_right_side_view_maximize" class="canvas_' + planeId
						+ 	' maximize_view_icon" src="http://' + this.domain + '/images/maximize.png" alt="Maximize View" />'
						+	'<canvas id="dataset_1_canvas_' + planeId + '_plane" class="side_canvas"></canvas>'
						+ 	(include_cross_hair ? 
								'<canvas id="dataset_1_canvas_' + planeId +
								'_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>'
								: ''
							);
					break;
			}
			
			
			html += "</div>";
		}

		//close outer div
		this.getDiv().append(html + '</div>');
	},
	initCanvasView : function(dataSet, use_image_service) {
		// we use that for the image service to be able to abort pending requests
		var sessionId = TissueStack.Utils.generateSessionId();
		var now = new Date().getTime();
		
		// loop over all planes in the data, create canvas and extent objects, then display them
		for (var i=0; i < dataSet.data.length; i++) {
			var dataForPlane = dataSet.data[i];
			var planeId = dataForPlane.name;
			
			var zoomLevels = eval(dataForPlane.zoomLevels);
			transformationMatrix = eval(dataForPlane.transformationMatrix);
			
			// create extent
			var extent = 
				new TissueStack.Extent(
					dataSet.id,
					!use_image_service,
					dataForPlane.oneToOneZoomLevel,
					planeId,
					dataForPlane.maxSlices,
					dataForPlane.maxX,
					dataForPlane.maxY,
					zoomLevels,
					transformationMatrix,
					dataForPlane.resolutionMm);
			
			// create canvas
			var canvasElementSelector = "dataset_1"; 
			var plane = new TissueStack.Canvas(
					extent,
					"canvas_" + planeId + "_plane",
					canvasElementSelector,
					this.include_cross_hair);
			plane.sessionId = sessionId;

			// for scalebar to know its parent
			if (i == 0) plane.is_main_view = true;
			plane.updateScaleBar();
			
			// store plane  
			dataSet.planes[planeId] = plane;

			// get the real world coordinates 
			dataSet.realWorldCoords[planeId] = plane.getDataExtent().getExtentCoordinates();
			
			// display data extent info on page
			plane.updateExtentInfo(dataSet.realWorldCoords[planeId]);

			// if we have more than 1 plane => show y as the main plane and make x and z the small views
			if (i != 0) {
				plane.changeToZoomLevel(0);
			} 

			plane.queue.drawLowResolutionPreview(now);
			plane.queue.drawRequestAfterLowResolutionPreview(null, now);
		}
	}, applyUserParameters : function(dataSet) {
		if (!this.initOpts) return;
		
		var plane_id = typeof(this.initOpts['plane']) === 'string' ?  this.initOpts['plane'] : 'y';

		// plane or data set does not exist => good bye
		if (!dataSet || !dataSet.planes[plane_id]) return;
		
		// do we need to swap planes in the main view
		var maximizeIcon = $("#dataset_1 .canvas_" + plane_id + ",maximize_view_icon");
		if (maximizeIcon && maximizeIcon.length == 1) {
			// not elegant but fire event to achieve our plane swap
			maximizeIcon.click();
		}
		
		var _this = this;
		var plane = dataSet.planes[plane_id];
		
		if (_this.initOpts['zoom'] != null && _this.initOpts['zoom'] >= 0 && _this.initOpts['zoom'] < plane.data_extent.zoom_levels.length) {
			plane.changeToZoomLevel(_this.initOpts['zoom']); 
		}

		if (_this.initOpts['color'] && _this.initOpts['color'] != 'grey') {
			// change color map collectively for all planes
			for (var id in dataSet.planes) dataSet.planes[id].color_map = _this.initOpts['color']; 
		}

		var givenCoords = {};
		if (_this.initOpts['x'] != null || _this.initOpts['y'] != null || _this.initOpts['z'] != null) {
			givenCoords = {x: _this.initOpts['x'] != null ? _this.initOpts['x'] : 0,
					y: _this.initOpts['y'] != null ? _this.initOpts['y'] : 0,
					z: _this.initOpts['z'] != null ? _this.initOpts['z'] : 0};
			
			if (plane.getDataExtent().worldCoordinatesTransformationMatrix) {
				givenCoords = plane.getDataExtent().getPixelForWorldCoordinates(givenCoords);
			}
		} else {
			givenCoords = plane.getRelativeCrossCoordinates();
			givenCoords.z = plane.getDataExtent().slice;
		}
		plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(givenCoords, true, new Date().getTime());
		setTimeout(function() {
			plane.events.changeSliceForPlane(givenCoords.z);
		}, 150);
	},
	adjustCanvasSizes : function() {
		// get dimensions from parent and impose them on the canvases
		var width = this.getDiv().width();
		var height = this.getDiv().height();
		
		$('#dataset_1').css({"width" : width, "height" : height});
				
		// set main canvas dimensions
		$('#dataset_1_main_view_canvas').css({"width" : width, "height" : height});
		$('#dataset_1_main_view_canvas canvas').attr("width", width);
		$('#dataset_1_main_view_canvas canvas').attr("height", height);
		
		// set main canvas dimensions
		var sideCanvasDims = {width: Math.floor(width * 0.3), height: Math.floor(height * 0.2)};
		$('.left_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.right_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.left_side_view canvas').attr("width", sideCanvasDims.width);
		$('.left_side_view canvas').attr("height", sideCanvasDims.height);
		$('.right_side_view canvas').attr("width", sideCanvasDims.width);
		$('.right_side_view canvas').attr("height", sideCanvasDims.height);
	},
	loadDataSetConfigurationFromServer : function() {
		var _this = this;
		var url = "http://" + _this.domain + "/" + TissueStack.configuration['restful_service_proxy_path'].value + "/data/" + _this.data_set_id;
		
		// create a new data store
		TissueStack.dataSetStore = new TissueStack.DataSetStore(null, true);
		
		TissueStack.Utils.sendAjaxRequest(
			url, 'GET',	true,	
			function(data, textStatus, jqXHR) {
				if (!data.response && !data.error) {
					_this.writeErrorMessageIntoDiv("Did not receive anyting from backend, neither success nor error ....");
					return;
				}
				
				if (data.error) {
					var message = "Application Error: " + (data.error.message ? data.error.message : " no more info available. check logs.");
					_this.writeErrorMessageIntoDiv(message);
					return;
				}
				
				if (data.response.noResults) {
					_this.writeErrorMessageIntoDiv("No data set found in configuration database for given id");
					return;
				}
				
				var dataSet = data.response;
				
				// create a new data store
				TissueStack.dataSetStore = new TissueStack.DataSetStore(null, true);
				// add to data store
				dataSet = TissueStack.dataSetStore.addDataSetToStore(dataSet, _this.domain);
				
				if (!dataSet.data || dataSet.data.length == 0) {
					this.writeErrorMessageIntoDiv("Data set '" + dataSet.id + "' does not have any planes associated with it!");
					return; 
				}

				// create the HTML necessary for display and initialize the canvas objects
				_this.createHTMLForDataSet(dataSet, _this.include_cross_hair);
				_this.adjustCanvasSizes();
				_this.initCanvasView(dataSet, _this.use_image_service);
				// if we have more than 1 plane => register the maximize events
				if (dataSet.data.length > 1) {
					_this.registerMaximizeEvents();
				}
				if (_this.initOpts) _this.applyUserParameters(dataSet);
			},
			function(jqXHR, textStatus, errorThrown) {
				_this.writeErrorMessageIntoDiv("Error connecting to backend: " + textStatus + " " + errorThrown);
			}
		);
	},
	loadDataBaseConfiguration : function() {
		// we do this one synchronously
		TissueStack.Utils.sendAjaxRequest(
			"http://" + this.domain + "/" + TissueStack.configuration['restful_service_proxy_path'].value + "/configuration/all/json", 'GET', false,
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
					alert("No configuration info found in database");
					return;
				}
				var configuration = data.response;
				
				for (var x=0;x<configuration.length;x++) {
					if (!configuration[x] || !configuration[x].name || $.trim(!configuration[x].name.length) == 0) {
						continue;
					}
					TissueStack.configuration[configuration[x].name] = {};
					TissueStack.configuration[configuration[x].name].value = configuration[x].value;
					TissueStack.configuration[configuration[x].name].description = configuration[x].description ? configuration[x].description : "";
				};
				
				TissueStack.Utils.loadColorMaps();
			},
			function(jqXHR, textStatus, errorThrown) {
				alert("Error connecting to backend: " + textStatus + " " + errorThrown);
			}
		);
	},
	registerWindowResizeEvent : function() {
		var _this = this;
		// handle dynamic window & parent div resizing
		$(window).resize(function() {
			_this.adjustCanvasSizes();
			
			// set new canvas dimensions
			var dataSet = TissueStack.dataSetStore.getDataSetByIndex(0);
			for (var plane in dataSet.planes) {
				dataSet.planes[plane].resizeCanvas();
			}
		});
	},
	registerMaximizeEvents : function() {
		$('#dataset_1_left_side_view_maximize, #dataset_1_right_side_view_maximize').bind("click", function(event) {
			// what side view and canvas called for maximization
			if (!event.target.id || !$("#" + event.target.id).attr("class")) {
				return;
			}

			var dataSet = TissueStack.dataSetStore.getDataSetByIndex(0);
			
			var plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#" + event.target.id).attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}
			var startPos = "canvas_".length;
			var sideViewPlaneId = plane.substring(startPos, startPos + 1);
			
			plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#dataset_1_main_view_canvas").attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}
			var mainViewPlaneId = plane.substring(startPos, startPos + 1);
			
			// with the id we get the can get the main canvas and the side canvas and swap them, including their dimensions and zoom levels
			var mainCanvas = $("#dataset_1_main_view_canvas");
			var mainCanvasChildren = mainCanvas.children("canvas");
			if (!mainCanvasChildren || mainCanvasChildren.length == 0) {
				return;
			}
			mainCanvasChildren.detach();
			
			startPos = event.target.id.indexOf("_maximize");
			if (startPos < 0) {
				return;
			}
			var sideCanvasId = event.target.id.substring(0, startPos);

			var sideCanvas = $("#" + sideCanvasId + "_canvas");
			var sideCanvasChildren = sideCanvas.children("canvas");
			if (!sideCanvasChildren || sideCanvasChildren.length == 0) {
				return;
			}
			sideCanvasChildren.detach();
			
			// swap dimensions
			var sideCanvasRelativeCross = dataSet.planes[sideViewPlaneId].getRelativeCrossCoordinates(); 
			var mainCanvasRelativeCross = dataSet.planes[mainViewPlaneId].getRelativeCrossCoordinates();
			
			var sideCanvasDims = {x: sideCanvasChildren[0].width, y: sideCanvasChildren[0].height};
			var mainCanvasDims = {x: mainCanvasChildren[0].width, y: mainCanvasChildren[0].height};
			
			var tmpAttr = [];
			for (var i=0; i < sideCanvasChildren.length; i++) {
				tmpAttr[i] = sideCanvasChildren[i].getAttribute("class");
				sideCanvasChildren[i].setAttribute("class", mainCanvasChildren[i].getAttribute("class"));
				sideCanvasChildren[i].width = mainCanvasDims.x;
				sideCanvasChildren[i].height = mainCanvasDims.y;
			}
			dataSet.planes[sideViewPlaneId].setDimensions(mainCanvasDims.x, mainCanvasDims.y);
			// store zoom level for side view
			var zoomLevelSideView = dataSet.planes[sideViewPlaneId].getDataExtent().zoom_level;

			for (var i=0; i < mainCanvasChildren.length; i++) {
				mainCanvasChildren[i].setAttribute("class", tmpAttr[i]);
				mainCanvasChildren[i].width = sideCanvasDims.x;
				mainCanvasChildren[i].height = sideCanvasDims.y;
			}
			dataSet.planes[mainViewPlaneId].setDimensions(sideCanvasDims.x, sideCanvasDims.y);
							
			mainCanvas.append(sideCanvasChildren);
			sideCanvas.append(mainCanvasChildren);
			
			// remember change in class
			$("#" + sideCanvasId + "_maximize").addClass("canvas_" + mainViewPlaneId);
			$("#" + sideCanvasId  + "_maximize").removeClass("canvas_" + sideViewPlaneId);
			$("#dataset_1_main_view_canvas").addClass("canvas_" + sideViewPlaneId);
			$("#dataset_1_main_view_canvas").removeClass("canvas_" + mainViewPlaneId);
			
			// redraw and change the zoom level as well
			var now = new Date().getTime();
			dataSet.planes[sideViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(sideCanvasRelativeCross, false, now);
			dataSet.planes[mainViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(mainCanvasRelativeCross, false, now);
			dataSet.planes[sideViewPlaneId].events.changeSliceForPlane(dataSet.planes[sideViewPlaneId].data_extent.slice);
			dataSet.planes[sideViewPlaneId].changeToZoomLevel(dataSet.planes[mainViewPlaneId].getDataExtent().zoom_level);
			dataSet.planes[mainViewPlaneId].changeToZoomLevel(zoomLevelSideView);

			dataSet.planes[sideViewPlaneId].updateExtentInfo(
			dataSet.planes[sideViewPlaneId].getDataExtent().getExtentCoordinates());
		});
	}
};