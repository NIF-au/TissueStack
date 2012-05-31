TissueStack.Init = function () {
	TissueStack.Utils.adjustScreenContentToActualScreenSize();
	TissueStack.Utils.indexColorMaps();
	
	if (TissueStack.phone) {
		new TissueStack.PhoneMenu();
	}
	
	var zoom_levels = [0.25, // level 0
	                   0.5,  // level 1
	                   0.75, // level 2
	                   1, 	 // level 3 == 1:1
	                   1.25, // level 4
	                   1.5,  // level 5
	                   1.75  // level 6
	                  ];
	
	var data = [
	            { // x plane
					id: "mouse_1",
					one_to_one_zoom_level: 3,
					plane: 'x',
					slices: 678,
					extent_x: 1311,
					extent_y: 499,
					worldCoordinatesTransformationMatrix : 
						[
						 	[0.5, 0   , 0   , -327.15 ],
						 	[0   , 0.5, 0   , -124.2   ],
						 	[0   , 0   , 0.5, -169.2   ],
						 	[0   , 0   , 0   ,  1          ]
						]
	            },
	            { // y plane
					id: "mouse_1",
					one_to_one_zoom_level: 3,
					plane: 'y',
					slices: 1310,
					extent_x: 679,
					extent_y: 499,
					worldCoordinatesTransformationMatrix : 
						[
						 	[0.5, 0   , 0   , -169.2   ],
						 	[0   , 0.5, 0   , -124.2   ],
						 	[0   , 0   , 0.5, -327.15 ],
						 	[0   , 0   , 0   ,  1          ]
						]
	            },
	            { // z plane
					id: "mouse_1",
					one_to_one_zoom_level: 3,
					plane: 'z',
					slices: 498,
					extent_x: 679,
					extent_y: 1311,
					worldCoordinatesTransformationMatrix : 
						[
						 	[0.5, 0   , 0   , -169.2   ],
						 	[0   , 0.5, 0   , -327.15 ],
						 	[0   , 0   , 0.5, -124.2  ],
						 	[0   , 0   , 0   ,  1           ]
						]
	            }
	];

	TissueStack.planes = {};
	TissueStack.realWorldCoords = {};
	
	// loop over data, create objects and listeners and then display them
	for (var i=0; i < data.length; i++) {
		var dataForPlane = data[i];
		var planeId = dataForPlane.plane;
		
		// create extent
		var extent = new TissueStack.Extent(dataForPlane.id, dataForPlane.one_to_one_zoom_level, planeId, dataForPlane.slices,
				dataForPlane.extent_x, dataForPlane.extent_y, zoom_levels, dataForPlane.worldCoordinatesTransformationMatrix);

		
		// create canvas
		var plane = new TissueStack.Canvas(extent, "canvas_" + planeId + "_plane");

		// store plane  
		TissueStack.planes[planeId] = plane;

		// get the real world coordinates 
		TissueStack.realWorldCoords[planeId] = plane.getDataExtent().getExtentCoordinates();
		
		// display data extent info on page
		plane.updateExtentInfo(TissueStack.realWorldCoords[planeId]);
		
		// for desktop version show 2 small canvases
		if (TissueStack.desktop && planeId != 'y') {
			plane.changeToZoomLevel(0);
			
		}
		
		// fill canvases
		plane.queue.drawLowResolutionPreview();
		plane.queue.drawRequestAfterLowResolutionPreview();
	}
};

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

TissueStack.CreateTreeEvent = function () {
	//CREATE DATA SET TREE 
	var treeData = [
	    {title: "CROSS", key: "cross_tree_id", select: true, tooltip: "cross overlay" },
	    {title: "CANVAS TILES", key:"canvas_tree_id", select: true },
	    {title: "REGIONS", isFolder: true, key: "id3", expand: true,
	      children: [
	        {title: "REGIONS X", key: "regions_tree_id", 
	          children: [
	            {title: "Sub-item 3.1.1", key: "regions_tree_x1.1" },
	            {title: "Sub-item 3.1.2", key: "regions_tree_x1.2" }
	          ]
	        },
	        {title: "REGIONS Y",
	          children: [
	            {title: "Sub-item 3.2.1", key: "regions_tree_y1.1" },
	            {title: "Sub-item 3.2.2", key: "regions_tree_y1.2" }
	          ]
	        },
	        {title: "REGIONS Z",
	          children: [
	            {title: "Sub-item 3.2.1", key: "regions_tree_y1.1" },
	            {title: "Sub-item 3.2.2", key: "regions_tree_y1.2" }
	          ]
	        } 
	      ]
	    },
	    {title: "REMOTE SERVER", key: "remote_server_id", expand: true,
	      children: [
	        {title: "FISH TILE",
	          children: [
	            {title: "FISH TILE 1", key: "remote_server_fish1" },
	            {title: "FISH TILE 2", key: "remote_server_fish2" }
	          ]
	        },
	        {title: "MOUSE TILE", expand: true,
	          children: [
	            {title: "MOUSE TILE 1", key: "remote_server_mouse1", select: true },
	            {title: "MOUSE TILE 2", key: "remote_server_mouse2" }
	          ]
	        },
	        {title: "Sub-item 4.3", hideCheckbox: true },
	        {title: "Sub-item 4.4", hideCheckbox: true }
	      ]
	    }
	  ];
	 // TRIGGER DATA SET FUNCTION 
	 $("#treedataset").dynatree({
	       checkbox: true,
	       selectMode: 2,
	       children: treeData,

	       onActivate: function(node) {

		   },
	       onDeactivate: function(node) {

           },
	       
	       onSelect: function(select, node) {
	         var selNodes = node.tree.getSelectedNodes();
	         var selKeys = $.map(selNodes, function(node){
	              return "[" + node.data.key + "]: '" + node.data.title + "'";
	         });
	         
	         //CONTROL CROSS SHOW/HIDE
	         var selected = node.isSelected();
	         if(node.data.key == "cross_tree_id" && selected == false){
	         	$("#canvas_y_plane_cross_overlay").fadeOut(50);
	         }else if(node.data.key == "cross_tree_id" && selected == true){
	         	$("#canvas_y_plane_cross_overlay").fadeIn(50);
	         }
	         //END
	         	         
	       },
	       onClick: function(node, event) {
	         if( node.getEventTargetType(event) == "title" )
	           node.toggleSelect();
	       },
	       onKeydown: function(node, event) {
	         if( event.which == 32 ) {
	           node.toggleSelect();
	           return false;
	         }
	       },
	       onDblClick: function(node, event) {
	         node.toggleSelect();
	       },
	  });
	//FUTURE USE USING JSON TO LOAD DATA SET TREE	  
	//	  $("#treedataset").dynatree({
	//	        title: "JSON LOADING FOR TISSUESTACK TREE EVENT",
	//	        fx: { height: "toggle", duration: 200 },
	//	        autoFocus: false, // Set focus to first child, when expanding or lazy-loading.
	//            initAjax: {
	//                url: "/TissueStack/minc/test",
	//                data: { mode: "test" } // data from data set
	//                },
	//	        onActivate: function(node) {
	//	        },
	//	  
	//	        onLazyRead: function(node){
	//                node.appendAjax({
	//                  url: "/TissueStack/minc/test",
	//                  data: {key: node.data.key,
	//                   mode: "test"
	//                 }
	//             });
	//	        }
	//	    });
	//END
};

$(document).ready(function() {
	TissueStack.Init();
	TissueStack.BindUniqueEvents();
	TissueStack.CreateTreeEvent();	
});
