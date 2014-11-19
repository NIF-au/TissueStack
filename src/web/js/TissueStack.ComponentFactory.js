TissueStack.ComponentFactory = {
    extractHostNameFromUrl : function(url) {
		if (typeof(url) !== "string") {
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
    checkDivExistence : function (div) {
        if (typeof (div) !== 'string' || $.trim(div) === '') return false;
	
        div = $("#" + div);
        if (div.length == 0) return false;
        
        return true;
    },
    checkDataSetObjectIntegrity : function(dataSet) {
        if (typeof(dataSet) != 'object') return false;

        // data member
        if (typeof(dataSet.data) != 'object' || typeof(dataSet.data.length) != 'number') return false;

        if (dataSet.data.length == 0) return false;
        
        return true;
    },
    checkDataSetOrdinal : function (ordinal, checkForExistence) {
        if (typeof (ordinal) !== 'number' || ordinal < 0) return false;
        
        if (typeof (checkForExistence) !== 'boolean' || !checkForExistence) return true;
        
        return !TissueStack.ComponentFactory.checkDivExistence("dataset_" + ordinal);
    },
    checkStandardInput : function(div, dataSet, dataSetOrdinal, checkForExistence) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::createDataSet => Given div id is invalid!");
            return false;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::createDataSet => Given dataSet object is invalid!");
            return false;
        }
            

        if (!TissueStack.ComponentFactory.checkDataSetOrdinal(dataSetOrdinal, checkForExistence)) {
            alert("ComponentFactory::createDataSet => Given dataSet ordinal is invalid!");
            return false;
        }

        return true;
    },
    initDataSetWidget : function(div, dataSet, includeCrossHair, addScaleBar, useImageService) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::resizeDataSetWidget => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::resizeDataSetWidget => given data set is invalid!");
            return;
        }
        
        if (typeof(includeCrossHair) !== 'boolean')
            includeCrossHair = false;

        if (typeof(useImageService) !== 'boolean')
            useImageService = false;

        if (typeof(addScaleBar) !== 'boolean')
            addScaleBar = false;

        // we use that for the image service to be able to abort pending requests
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
					!useImageService,
					dataForPlane.oneToOneZoomLevel,
					planeId,
					dataForPlane.maxSlices,
					dataForPlane.maxX,
					dataForPlane.maxY,
					zoomLevels,
					transformationMatrix,
					dataForPlane.resolutionMm);
			
			// create canvas
			var plane = new TissueStack.Canvas(
					extent,
					"canvas_" + planeId + "_plane",
					div,
					includeCrossHair);

			// for scalebar to know its parent
			if (i == 0) plane.is_main_view = true;
			if (plane.is_main_view && addScaleBar) plane.updateScaleBar();
			
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
	},
    resizeDataSetWidget : function(parent, div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(parent)) {
            alert("ComponentFactory::resizeDataSetWidget => parent div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::resizeDataSetWidget => given div does not exist!");
            return;
        }

        // adjust sizes based on surrounding div
        var width = $('#' + parent).width();
		var height = $('#' + parent).height();
		
		$('#' + div).css({"width" : width, "height" : height});
				
		// set main canvas dimensions
		$('#' + div + '_main_view_canvas').css({"width" : width, "height" : height});
		$('#' + div + '_main_view_canvas canvas').attr("width", width);
		$('#' + div + '_main_view_canvas canvas').attr("height", height);
		
		// set main canvas dimensions
		var sideCanvasDims = {width: Math.floor(width * 0.3), height: Math.floor(height * 0.2)};
		$('#' + div + ' .left_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('#' + div + ' .right_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('#' + div + ' .left_side_view canvas').attr("width", sideCanvasDims.width);
		$('#' + div + ' .left_side_view canvas').attr("height", sideCanvasDims.height);
		$('#' + div + ' .right_side_view canvas').attr("width", sideCanvasDims.width);
		$('#' + div + ' .right_side_view canvas').attr("height", sideCanvasDims.height);
    },
    createDataSetWidget : function(div, dataSet, dataSetOrdinal, server, includeCrossHair, addScaleBar, useImageService) {
        if (!TissueStack.ComponentFactory.checkStandardInput(div, dataSet, dataSetOrdinal, true))
            return;

        if (typeof(server) === 'string' && server !== 'localhost')
            server = TissueStack.ComponentFactory.extractHostNameFromUrl(server);
        
        var maximizePngPath = 'images/maximize.png';
        if (server != null)
            maximizePngPath = "http://" + server + "/" + maximizePngPath;
        
        var dataSetPrefix = "dataset_" + dataSetOrdinal;
        
        var html = '<div id="' + dataSetPrefix + '" class="dataset">';
		
        if (typeof(includeCrossHair) !== 'boolean')
            includeCrossHair = false;

        if (typeof(useImageService) !== 'boolean')
            useImageService = false;

        if (typeof(addScaleBar) !== 'boolean')
            addScaleBar = false;

		// loop over all planes in the data
		for (var i=0; i < dataSet.data.length; i++) {
			var planeId = dataSet.data[i].name;
			
			switch(i) {
				case 0:
					html +=
							'<div id="' + dataSetPrefix + '_main_view_canvas" class="canvasview canvas_' + planeId + '">'
						+ 	'<canvas id="' + dataSetPrefix + '_canvas_' + planeId + '_plane" class="plane"></canvas>'
						+ (includeCrossHair ? 
								'<canvas id="' + dataSetPrefix + '_canvas_'  + planeId + '_plane_cross_overlay" class="cross_overlay"></canvas>'
								: '')
                        + '</div>';
					break;
				case 1:
					html +=
							'<div id="' + dataSetPrefix + '_left_side_view_canvas" class="left_side_view ui-bar-a">'
						+	'<img id="' + dataSetPrefix + '_left_side_view_maximize" class="canvas_' + planeId
						+ 	' maximize_view_icon" src="' + maximizePngPath + '" alt="Maximize View" />'
						+	'<canvas id="' + dataSetPrefix + '_canvas_' + planeId + '_plane" class="side_canvas"></canvas>'
						+ 	(includeCrossHair ? 
								'<canvas id="' + dataSetPrefix + '_canvas_' + planeId +
								'_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>'
								: '')
                        + '</div>';
					break;
				case 2:
					html +=
							'<div id="' + dataSetPrefix + '_right_side_view_canvas" class="right_side_view ui-bar-a">'
						+	'<img id="' + dataSetPrefix + '_right_side_view_maximize" class="canvas_' + planeId
						+ 	' maximize_view_icon" src="' + maximizePngPath + '" alt="Maximize View" />'
						+	'<canvas id="' + dataSetPrefix + '_canvas_' + planeId + '_plane" class="side_canvas"></canvas>'
						+ 	(includeCrossHair ? 
								'<canvas id="' + dataSetPrefix + '_canvas_' + planeId +
								'_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>'
								: '')
                        + '</div>';                    
					break;
			}
        }
			
		html += "</div>";
        
        $("#" + div).append(html);
        
        if (addScaleBar)
            TissueStack.ComponentFactory.addScaleToDataSetWidget(dataSetPrefix);
        
        TissueStack.ComponentFactory.resizeDataSetWidget(div, dataSetPrefix);
        TissueStack.ComponentFactory.initDataSetWidget(dataSetPrefix, dataSet, includeCrossHair, addScaleBar, useImageService);
        TissueStack.ComponentFactory.registerMaximizeEventsForDataSetWidget(dataSetPrefix, dataSet);
        
        return dataSetPrefix;
    },
    addScaleToDataSetWidget : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::addScaleToDataSet => Failed to add scale bar!");
            return;
        }
        
        var html = '<div class="scalecontrol_main"><div id="' + div + '_scale_middle" class="scalecontrol_middle">'
            + '<div class="scalecontrol_image" style="left: 0px; top: -424px; width: 89px;"></div></div>'
            + '<div id="' + div + '_scale_left" class="scalecontrol_left">'
			+ '<div class="scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
			+ '<div id="' + div + '_scale_center_left" class="scalecontrol_center_left">'
	    	+ '<div class="scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + div + '_scale_center_right" class="scalecontrol_center_right">'
	    	+ '<div class="scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + div + '_scale_up" class="scalecontrol_up">'
    		+ '<div class="scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + div + '_scale_text_up" class="scalecontrol_text_up"></div></div>';
   
        $("#" + div).prepend(html);
	},
	registerMaximizeEventsForDataSetWidget : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::resizeDataSetWidget => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::resizeDataSetWidget => given data set is invalid!");
            return;
        }
        
		$('#' + div + '_left_side_view_maximize, #' + div + '_right_side_view_maximize').bind("click", function(event) {
			// what side view and canvas called for maximization
			if (!event.target.id || !$("#" + event.target.id).attr("class")) {
				return;
			}

			var plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#" + event.target.id).attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}
			var startPos = "canvas_".length;
			var sideViewPlaneId = plane.substring(startPos, startPos + 1);
			
			plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($('#' + div + '_main_view_canvas').attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}
			var mainViewPlaneId = plane.substring(startPos, startPos + 1);
			
			// with the id we get the can get the main canvas and the side canvas and swap them, including their dimensions and zoom levels
			var mainCanvas = $('#' + div + '_main_view_canvas');
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
			$('#' + div + '_main_view_canvas').addClass("canvas_" + sideViewPlaneId);
			$('#' + div + '_main_view_canvas').removeClass("canvas_" + mainViewPlaneId);
			
			// swap main view
			dataSet.planes[mainViewPlaneId].is_main_view = false;
			dataSet.planes[sideViewPlaneId].is_main_view = true;
			
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