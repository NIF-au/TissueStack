TissueStack.Init = function () {
	TissueStack.dataSetStore = new TissueStack.DataSetStore();
	new TissueStack.DataSetNavigation();
	
	TissueStack.Utils.adjustScreenContentToActualScreenSize();
	
	TissueStack.Utils.indexColorMaps();
	
	if (TissueStack.phone) {
		new TissueStack.PhoneMenu();
	}

	// for the time being to be compliant and not break things entirely, we init with only the first of the backend data sets
	if (TissueStack.dataSetStore.getSize() == 0) {
		alert("This is a broken work in progress. Do not attempt to fix it. It will work again soon enough !!!");
		return;
	}
	
	// loop over the data sets in the store
	for (var dataStoreKey in TissueStack.dataSetStore) {
		var dataStorePlane = TissueStack.dataSetStore[dataStoreKey];
		
		// loop over all planes in the data, create canvas and extent objects, then display them
		for (var i=0; i < dataStorePlane.data.length; i++) {
			var dataForPlane = dataStorePlane.data[i];
			var planeId = dataForPlane.name;
			
			// create extent
			var extent = 
				new TissueStack.Extent(
					dataStorePlane.id,
					dataForPlane.oneToOneZoomLevel,
					planeId,
					dataForPlane.maxSclices,
					dataForPlane.maxX,
					dataForPlane.maxY,
					dataForPlane.zoomLevels,
					dataForPlane.transformationMatrix);
			
			// create canvas
			var plane = new TissueStack.Canvas(extent, "canvas_" + planeId + "_plane");

			// store plane  
			dataStorePlane.planes[planeId] = plane;

			// get the real world coordinates 
			dataStorePlane.realWorldCoords[planeId] = plane.getDataExtent().getExtentCoordinates();
			
			// display data extent info on page
			plane.updateExtentInfo(dataStorePlane.realWorldCoords[planeId]);
			
			// for desktop version show 2 small canvases
			if (TissueStack.desktop && planeId != 'y') {
				plane.changeToZoomLevel(0);
				
			}
			
			// fill canvases
			plane.queue.drawLowResolutionPreview();
			plane.queue.drawRequestAfterLowResolutionPreview();
		}

		// we abort after the first data set. the rest of the ui code can not yet handle 2 data sets displayed at once !!!
		break;
	}
};

// TODO: link the listeners to the first data set in the store !!!

TissueStack.BindUniqueEvents = function () {
	// DRAWING INTERVAL CHANGE HANDLER 
	$('#drawing_interval_button').bind("click", function() {
		var newValue = parseInt($('#drawing_interval').val());
		for (var id in TissueStack.planes) {	
			TissueStack.planes[id].queue.setDrawingInterval(newValue);
		}
	});
	
	// COLOR MAP CHANGE HANDLER
	$('input[name="color_map"]').bind("click", function(e) {
		for (var id in TissueStack.planes) {	
			TissueStack.planes[id].color_map = e.target.value;
			TissueStack.planes[id].drawMe();
			TissueStack.planes[id].applyColorMapToCanvasContent();
		}
	});

	
	// MAXIMIZING SIDE VIEWS
	if (TissueStack.desktop) {
		$('#left_side_view_maximize, #right_side_view_maximize').bind("click", function(event) {
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
			
			plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#main_view_canvas").attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}
			var mainViewPlaneId = plane.substring(startPos, startPos + 1);
			
			// with the id we get the can get the main canvas and the side canvas and swap them, including their dimensions and zoom levels
			var mainCanvas = $("#main_view_canvas");
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
			var sideCanvasRelativeCross = TissueStack.planes[sideViewPlaneId].getRelativeCrossCoordinates(); 
			var mainCanvasRelativeCross = TissueStack.planes[mainViewPlaneId].getRelativeCrossCoordinates();
			
			var sideCanvasDims = {x: sideCanvasChildren[0].width, y: sideCanvasChildren[0].height};
			var mainCanvasDims = {x: mainCanvasChildren[0].width, y: mainCanvasChildren[0].height};
			var tmpAttr = [];
			
			for (var i=0; i < sideCanvasChildren.length; i++) {
				tmpAttr[i] = sideCanvasChildren[i].getAttribute("class");
				sideCanvasChildren[i].setAttribute("class", mainCanvasChildren[i].getAttribute("class"));
				sideCanvasChildren[i].width = mainCanvasDims.x;
				sideCanvasChildren[i].height = mainCanvasDims.y;
			}
			TissueStack.planes[sideViewPlaneId].setDimensions(mainCanvasDims.x, mainCanvasDims.y);
			// store zoom level for side view
			var zoomLevelSideView = TissueStack.planes[sideViewPlaneId].getDataExtent().zoom_level;
			
			for (var i=0; i < mainCanvasChildren.length; i++) {
				mainCanvasChildren[i].setAttribute("class", tmpAttr[i]);
				mainCanvasChildren[i].width = sideCanvasDims.x;
				mainCanvasChildren[i].height = sideCanvasDims.y;
			}
			TissueStack.planes[mainViewPlaneId].setDimensions(sideCanvasDims.x, sideCanvasDims.y);
			
			mainCanvas.append(sideCanvasChildren);
			sideCanvas.append(mainCanvasChildren);
	
			// remember change in class
			$("#" + event.target.id).addClass("canvas_" + mainViewPlaneId);
			$("#" + event.target.id).removeClass("canvas_" + sideViewPlaneId);
			$("#main_view_canvas").addClass("canvas_" + sideViewPlaneId);
			$("#main_view_canvas").removeClass("canvas_" + mainViewPlaneId);
			$("#canvas_main_slider").addClass("canvas_" + sideViewPlaneId);
			$("#canvas_main_slider").removeClass("canvas_" + mainViewPlaneId);
			// swap slice dimension values
			$("#canvas_main_slider").attr("value", TissueStack.planes[sideViewPlaneId].data_extent.slice);
			$("#canvas_main_slider").attr("max", TissueStack.planes[sideViewPlaneId].data_extent.max_slices);
			
			// redraw and change the zoom level as well
			TissueStack.planes[sideViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(sideCanvasRelativeCross);
			TissueStack.planes[mainViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(mainCanvasRelativeCross);
			TissueStack.planes[sideViewPlaneId].changeToZoomLevel(TissueStack.planes[mainViewPlaneId].getDataExtent().zoom_level);
			TissueStack.planes[mainViewPlaneId].changeToZoomLevel(zoomLevelSideView);
			TissueStack.planes[sideViewPlaneId].updateExtentInfo(
					TissueStack.planes[sideViewPlaneId].getDataExtent().getExtentCoordinates());
		});
	}

	// COORDINATE CENTER FUNCTIONALITY FOR DESKTOP
	if (TissueStack.desktop) {
		$('#center_point_in_canvas').bind("click", function() {
			
			var plane =
				TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#main_view_canvas").attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}
			var startPos = "canvas_".length;
			var planeId = plane.substring(startPos, startPos + 1);
			
			var xCoord = parseFloat($('#canvas_point_x').val());
			var yCoord = parseFloat($('#canvas_point_y').val());
			var zCoord = parseFloat($('#canvas_point_z').val());
			
			if (isNaN(xCoord) || isNaN(yCoord) || isNaN(zCoord)
					|| xCoord.length == 0 || yCoord.length == 0 || zCoord.length == 0
					|| xCoord < TissueStack.realWorldCoords[planeId].min_x || xCoord > TissueStack.realWorldCoords[planeId].max_x 
					|| yCoord < TissueStack.realWorldCoords[planeId].min_y || yCoord > TissueStack.realWorldCoords[planeId].max_y
					|| zCoord < TissueStack.realWorldCoords[planeId].min_z || zCoord > TissueStack.realWorldCoords[planeId].max_z) {
				alert("Illegal coords");
				return;
			}
			
			// if we had a transformation matrix, we know we have been handed in real word coords and therefore need to convert back to pixel
			var givenCoords = {x: xCoord, y: yCoord, z: zCoord};
			plane = TissueStack.planes[planeId];
			if (plane.getDataExtent().worldCoordinatesTransformationMatrix) {
				givenCoords = plane.getDataExtent().getPixelForWorldCoordinates(givenCoords);
			}
			plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(givenCoords);
			plane.events.changeSliceForPlane(plane.getDataExtent().slice);
			var slider = $("#canvas_main_slider");
			if (slider) {
				slider.val(givenCoords.z);
				slider.blur();
			}
		});
	}	
	
	// Z PLANE AKA SLICE SLIDER 
	var extractCanvasId = function(sliderId) {
		if (!sliderId) {
			return;
		}
		
		var planeId = null;
		if (TissueStack.desktop) {
			var plane =
				TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#" + sliderId).attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}
			var startPos = "canvas_".length;
			planeId = plane.substring(startPos, startPos + 1);
		} else {
			planeId = sliderId.substring("canvas_".length, "canvas_".length + 1);
		}
		
		return planeId;
	};
	var triggerQueuedRedraw = function(id, slice) {
		if (!id) {
			return;
		}
		
		if (slice < 0 || slice > TissueStack.planes[id].data_extent.max_slices) {
			return;
		}
		
		TissueStack.planes[id].events.updateCoordinateDisplay();
		TissueStack.planes[id].events.changeSliceForPlane(slice);			
	};
	
	// z dimension slider: set proper length and min/max for dimension
	// sadly a separate routine is necessary for the active page slider.
	// for reasons unknown the active page slider does not refresh until after a page change has been performed 
	if (TissueStack.desktop) {
		$('.ui-slider-vertical').css({"height": TissueStack.canvasDimensions.height - 50});
	} 
	$('.canvasslider').each(
		function() {
			var id = extractCanvasId(this.id);

			$(this).attr("min", 0);
			$(this).attr("max", TissueStack.planes[id].data_extent.max_slices);
			$(this).attr("value", TissueStack.planes[id].data_extent.slice);
		}
	);
	$('.canvasslider').bind ("change", function (event, ui)  {
		var id = extractCanvasId(this.id);
		triggerQueuedRedraw(id, this.value);
	});
	
	$(".canvasslider").live ("slidercreate", function () {
		var res = $('#' + this.id).data('events');
		// unbind previous change
		$('#' + this.id).unbind("change");
		if (!res.change || res.change.length == 0) {
			$('#' + this.id).bind("change", function (event, ui)  {
				var id = extractCanvasId(this.id);
				triggerQueuedRedraw(id, this.value);
			});
		}
	});
};


$(document).ready(function() {
	TissueStack.Init();
	TissueStack.BindUniqueEvents();
});
