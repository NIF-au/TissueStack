TissueStack.Init = function (afterLoadingRoutine) {
	// hide second jquery coordinate search button  
	if (TissueStack.desktop) {
		$('#dataset_2_center_point_in_canvas').closest('.ui-btn').hide();
	}

	// prepare color maps
	TissueStack.Utils.indexColorMaps();

	// add phone menu for page navigation
	if (TissueStack.phone) {
		new TissueStack.PhoneMenu();
	}
	
	// create data store and load it with backend data
	TissueStack.dataSetStore = new TissueStack.DataSetStore(afterLoadingRoutine);
};

TissueStack.InitUserInterface = function (datasets) {
	// if we received no data sets from the backend, our job is done for now
	if (TissueStack.dataSetStore.getSize() == 0) {
		return;
	}
	
	if (typeof(datasets) != "object" || typeof(datasets.length) == 'undefined') {
		alert("Handed in data set is not an array of datasets!");
		return;
	}
	if (datasets.length == 0) {
		alert("No dataset handed in!");
		return;
	}
	
	var maxDataSets = (TissueStack.phone || TissueStack.tablet) ? 1 : 2;
	if (maxDataSets > datasets.length) {
		maxDataSets = datasets.length;
	}

	TissueStack.Utils.adjustScreenContentToActualScreenSize(maxDataSets);

	for (var x=0;x<maxDataSets;x++) {
		var dataSet = datasets[x];
		
		if (!dataSet.data || dataSet.data.length == 0) {
			alert("Data set '" + dataSet.id + "' does not have any planes associated with it!");
			continue; 
		}
		
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
					dataForPlane.oneToOneZoomLevel,
					planeId,
					dataForPlane.maxSclices,
					dataForPlane.maxX,
					dataForPlane.maxY,
					zoomLevels,
					transformationMatrix);
			
			// create canvas
			var canvasElementSelector = (TissueStack.desktop || TissueStack.tablet) ? ("dataset_" + (x+1)) : ""; 
			var plane = new TissueStack.Canvas(extent, "canvas_" + planeId + "_plane", canvasElementSelector);

			// store plane  
			dataSet.planes[planeId] = plane;

			// get the real world coordinates 
			dataSet.realWorldCoords[planeId] = plane.getDataExtent().getExtentCoordinates();
			
			// display data extent info on page
			plane.updateExtentInfo(dataSet.realWorldCoords[planeId]);
			
			// for desktop version show 2 small canvases
			if ((TissueStack.desktop || TissueStack.tablet) && planeId != 'y') {
				plane.changeToZoomLevel(0);
				
			}
			
			// fill canvases
			plane.queue.drawLowResolutionPreview();
			plane.queue.drawRequestAfterLowResolutionPreview();
		}
	} 
	
};


// TODO: the following lines needs to become more flexible to have new data sets displayed which includes dynamically un/binding events
TissueStack.BindUniqueEvents = function (datasets) {
	if (TissueStack.dataSetStore.getSize() ==0) {
		return;
	}

	if (typeof(datasets) != "object" || typeof(datasets.length) == 'undefined') {
		alert("Handed in data set is not an array of datasets!");
		return;
	}
	if (datasets.length == 0) {
		alert("No dataset handed in!");
		return;
	}

	var maxDataSets = (TissueStack.phone || TissueStack.tablet) ? 1 : 2;
	if (maxDataSets > datasets.length) {
		maxDataSets = datasets.length;
	}

	// first handle events that are linked to potentially more than 1 data set
	// DRAWING INTERVAL CHANGE HANDLER 
	$('#drawing_interval_button').bind("click", function() {
		var newValue = parseInt($('#drawing_interval').val());
		
		for (var x=0;x<maxDataSets;x++) {
			var dataSet = datasets[x];
			
			for (var id in dataSet.planes) {	
				dataSet.planes[id].queue.setDrawingInterval(newValue);
			}
		}
	});
	
	// COLOR MAP CHANGE HANDLER
	$('input[name="color_map"]').bind("click", function(e) {
		for (var x=0;x<maxDataSets;x++) {
			var dataSet = datasets[x];
			
			for (var id in dataSet.planes) {	
				dataSet.planes[id].color_map = e.target.value;
				dataSet.planes[id].drawMe();
				dataSet.planes[id].applyColorMapToCanvasContent();
			}
		}
	});

	// now let's bind events that are intimately linked to their own data set
	for (var y=0;y<maxDataSets;y++) {
		var dataSet = datasets[y];
	
		// MAXIMIZING SIDE VIEWS
		if ((TissueStack.desktop || TissueStack.tablet)) {
			$('#dataset_' + (y+1) + '_left_side_view_maximize, #dataset_' + (y+1) + '_right_side_view_maximize').bind("click", [{actualDataSet: dataSet,x: y}], function(event) {
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
				
				x = event.data[0].x;
				
				plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#dataset_" + (x+1) +"_main_view_canvas").attr("class").split(" "), "^canvas_");
				if (!plane) {
					return;
				}
				var mainViewPlaneId = plane.substring(startPos, startPos + 1);
				
				// with the id we get the can get the main canvas and the side canvas and swap them, including their dimensions and zoom levels
				var mainCanvas = $("#dataset_" + (x+1) + "_main_view_canvas");
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
				var sideCanvasRelativeCross = event.data[0].actualDataSet.planes[sideViewPlaneId].getRelativeCrossCoordinates(); 
				var mainCanvasRelativeCross = event.data[0].actualDataSet.planes[mainViewPlaneId].getRelativeCrossCoordinates();
				
				var sideCanvasDims = {x: sideCanvasChildren[0].width, y: sideCanvasChildren[0].height};
				var mainCanvasDims = {x: mainCanvasChildren[0].width, y: mainCanvasChildren[0].height};
				
				var tmpAttr = [];
				for (var i=0; i < sideCanvasChildren.length; i++) {
					tmpAttr[i] = sideCanvasChildren[i].getAttribute("class");
					sideCanvasChildren[i].setAttribute("class", mainCanvasChildren[i].getAttribute("class"));
					sideCanvasChildren[i].width = mainCanvasDims.x;
					sideCanvasChildren[i].height = mainCanvasDims.y;
				}
				event.data[0].actualDataSet.planes[sideViewPlaneId].setDimensions(mainCanvasDims.x, mainCanvasDims.y);
				// store zoom level for side view
				var zoomLevelSideView = event.data[0].actualDataSet.planes[sideViewPlaneId].getDataExtent().zoom_level;

				for (var i=0; i < mainCanvasChildren.length; i++) {
					mainCanvasChildren[i].setAttribute("class", tmpAttr[i]);
					mainCanvasChildren[i].width = sideCanvasDims.x;
					mainCanvasChildren[i].height = sideCanvasDims.y;
				}
				event.data[0].actualDataSet.planes[mainViewPlaneId].setDimensions(sideCanvasDims.x, sideCanvasDims.y);
								
				mainCanvas.append(sideCanvasChildren);
				sideCanvas.append(mainCanvasChildren);
				
				// remember change in class
				$("#" + sideCanvasId + "_maximize").addClass("canvas_" + mainViewPlaneId);
				$("#" + sideCanvasId  + "_maximize").removeClass("canvas_" + sideViewPlaneId);
				$("#dataset_" + (x+1) +"_main_view_canvas").addClass("canvas_" + sideViewPlaneId);
				$("#dataset_" + (x+1) +"_main_view_canvas").removeClass("canvas_" + mainViewPlaneId);
				$("#dataset_" + (x+1) + "_canvas_main_slider").addClass("canvas_" + sideViewPlaneId);
				$("#dataset_" + (x+1) + "_canvas_main_slider").removeClass("canvas_" + mainViewPlaneId);
				// swap slice dimension values
				$("#dataset_" + (x+1) + "_canvas_main_slider").attr("value", event.data[0].actualDataSet.planes[sideViewPlaneId].data_extent.slice);
				$("#dataset_" + (x+1) + "_canvas_main_slider").attr("max", event.data[0].actualDataSet.planes[sideViewPlaneId].data_extent.max_slices);
				
				// redraw and change the zoom level as well
				event.data[0].actualDataSet.planes[sideViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(sideCanvasRelativeCross);
				event.data[0].actualDataSet.planes[mainViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(mainCanvasRelativeCross);
				event.data[0].actualDataSet.planes[sideViewPlaneId].changeToZoomLevel(event.data[0].actualDataSet.planes[mainViewPlaneId].getDataExtent().zoom_level);
				event.data[0].actualDataSet.planes[mainViewPlaneId].changeToZoomLevel(zoomLevelSideView);
				event.data[0].actualDataSet.planes[sideViewPlaneId].updateExtentInfo(
				event.data[0].actualDataSet.planes[sideViewPlaneId].getDataExtent().getExtentCoordinates());
			});
		}

		// COORDINATE CENTER FUNCTIONALITY FOR DESKTOP
		if ((TissueStack.desktop || TissueStack.tablet)) {
			$('#dataset_' + (y+1) + '_center_point_in_canvas').bind("click", [{actualDataSet: dataSet,x: y}], function(event) {
				
				var plane =
					TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray(
							$("#" + (TissueStack.desktop || TissueStack.tablet ? "dataset_" + (event.data[0].x + 1) + "_": "") + "main_view_canvas").attr("class").split(" "), "^canvas_");
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
						|| xCoord < event.data[0].actualDataSet.realWorldCoords[planeId].min_x || xCoord > event.data[0].actualDataSet.realWorldCoords[planeId].max_x 
						|| yCoord < event.data[0].actualDataSet.realWorldCoords[planeId].min_y || yCoord > event.data[0].actualDataSet.realWorldCoords[planeId].max_y
						|| zCoord < event.data[0].actualDataSet.realWorldCoords[planeId].min_z || zCoord > event.data[0].actualDataSet.realWorldCoords[planeId].max_z) {
					alert("Illegal coords");
					return;
				}
				
				// if we had a transformation matrix, we know we have been handed in real word coords and therefore need to convert back to pixel
				var givenCoords = {x: xCoord, y: yCoord, z: zCoord};
				plane = event.data[0].actualDataSet.planes[planeId];
				if (plane.getDataExtent().worldCoordinatesTransformationMatrix) {
					givenCoords = plane.getDataExtent().getPixelForWorldCoordinates(givenCoords);
				}
	
				plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(givenCoords);
				
				var slider = $("#" + (plane.dataset_id == "" ? "" : plane.dataset_id + "_") + "canvas_main_slider");
				if (slider) {
					slider.val(givenCoords.z);
					slider.blur();
				}
			});
		}	
	
		// Z PLANE AKA SLICE SLIDER 
		var extractCanvasId = function(sliderId, actualDataSet) {
			if (!sliderId) {
				return;
			}
			
			var planeId = null;
			if ((TissueStack.desktop || TissueStack.tablet)) {
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

			var dataset_prefixEnd = sliderId.lastIndexOf("_canvas_main_slider");
			if (dataset_prefixEnd > 0 && sliderId.substring(0,dataset_prefixEnd) != actualDataSet.planes[planeId].dataset_id) {
				return null;
			}
			
			return planeId;
		};
		var triggerQueuedRedraw = function(id, slice, actualDataSet) {
			if (!id) {
				return;
			}
			
			if (slice < 0 || slice > actualDataSet.planes[id].data_extent.max_slices) {
				return;
			}
			
			actualDataSet.planes[id].events.updateCoordinateDisplay();
			actualDataSet.planes[id].events.changeSliceForPlane(slice);			
		};
		
		(function(actualDataSet) {
			// z dimension slider: set proper length and min/max for dimension
			// sadly a separate routine is necessary for the active page slider.
			// for reasons unknown the active page slider does not refresh until after a page change has been performed 
			if ((TissueStack.desktop || TissueStack.tablet)) {
				$('.ui-slider-vertical').css({"height": TissueStack.canvasDimensions.height - 50});
			} 
			$('.canvasslider').each(
				function() {
					var id = extractCanvasId(this.id, actualDataSet);
					
					if (!id) {
						return;
					}
					
					$(this).attr("min", 0);
					$(this).attr("max", actualDataSet.planes[id].data_extent.max_slices);
					$(this).attr("value", actualDataSet.planes[id].data_extent.slice);
				}
			);
			$('.canvasslider').bind ("change", function (event, ui)  {
				var id = extractCanvasId(this.id, actualDataSet);
				if (!id) {
					return;
				}

				triggerQueuedRedraw(id, this.value, actualDataSet);
			});
			
			$(".canvasslider").live ("slidercreate", function () {
				var res = $('#' + this.id).data('events');
				// unbind previous change
				$('#' + this.id).unbind("change");
				if (!res.change || res.change.length == 0) {
					$('#' + this.id).bind("change", function (event, ui)  {
						var id = extractCanvasId(this.id);
						if (!id) {
							return;
						}
						triggerQueuedRedraw(id, this.value, actualDataSet);
					});
				}
			});
		})(dataSet);
	}
};

TissueStack.UnBindUniqueEvents = function () {
	// Note: the 2 underneath are global and probably don't need to be unbound
	// DRAWING INTERVAL CHANGE HANDLER 
	$('#drawing_interval_button').unbind("click");
	// COLOR MAP CHANGE HANDLER
	$('input[name="color_map"]').unbind("click");
	
	// MAXIMIZING SIDE VIEWS
	if ((TissueStack.desktop || TissueStack.tablet)) {
		$('#left_side_view_maximize, #right_side_view_maximize').unbind("click");
		//NOTE: we might have to restore the default plane names
	}
	
	// COORDINATE CENTER FUNCTIONALITY FOR DESKTOP
	if ((TissueStack.desktop || TissueStack.tablet)) {
		$('#center_point_in_canvas').unbind("click");
	}	
	
	$('.canvasslider').unbind ("change");
};

$(document).ready(function() {
	var afterLoadingRoutine = function() {
		// on the first load we always display the first one only
		var dataSets = [TissueStack.dataSetStore.getDataSetByIndex(0)];
		
		TissueStack.InitUserInterface(dataSets);
		TissueStack.BindUniqueEvents(dataSets);
		new TissueStack.DataSetNavigation();
	};
	// call init
	TissueStack.Init(afterLoadingRoutine);
});
