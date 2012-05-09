TissueStack.Init = function () {
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

		// bind coordinate center functionality
		(function (plane, planeId) {
			$('#center_point_in_canvas_' + planeId).bind("click", function() {
				var xCoord = parseInt($('#canvas_' + planeId + '_x').val());
				var yCoord = parseInt($('#canvas_' + planeId + '_y').val());
				
				if (xCoord < 0 || xCoord > plane.getDataExtent().x 
						|| yCoord < 0 || yCoord > plane.getDataExtent().y) {
					alert("Illegal coords");
					return;
				}
				plane.redrawWithCenterAndCrossAtGivenPixelCoordinates({x: xCoord, y: yCoord});
			});
			
			
		})(plane, planeId);
		
		// display data extent info on page
		$('#canvas_' + planeId + '_extent').html("Data Extent: " + plane.getDataExtent().x + " x " + plane.getDataExtent().y + " [Zoom Level: " + plane.getDataExtent().zoom_level + "] ");
		
		// change zoom levels of x and z to 0
		if (planeId == 'x' || planeId == 'z') {
			plane.changeToZoomLevel(0);
		}
		
		// fill canvases
		plane.queue.drawLowResolutionPreview();
		plane.queue.drawRequestAfterLowResolutionPreview();
	}
	
	// bind event for queue interval change
	$('#drawing_interval_button').bind("click", function() {
		var newValue = parseInt($('#drawing_interval').val());
		for (var i=0; i < data.length; i++) {	
			TissueStack.planes[data[i].plane].queue.setDrawingInterval(newValue);
		}
	});
	
	// bind event listener for sync checkbox
	$('#sync_canvases').bind("change", function() {
		for (var i=0; i < data.length; i++) {	
			TissueStack.planes[data[i].plane].sync_canvases = $('#sync_canvases')[0].checked;
		}
	});

	// bind event listener for color map radio group
	$('input[name="color_map"]').bind("click", function(e) {
		for (var i=0; i < data.length; i++) {	
			if (e.target.value === TissueStack.planes[data[i].plane].color_map) {
				return;
			}
			TissueStack.planes[data[i].plane].color_map = e.target.value;
			TissueStack.planes[data[i].plane].drawMe();
			TissueStack.planes[data[i].plane].applyColorMapToCanvasContent();
		}
	});

	
	// bind event listener for maximizing side views
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
		
		plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#main_view_div").attr("class").split(" "), "^canvas_");
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
		for (var i=0; i < sideCanvasChildren.length; i++) {
			sideCanvasChildren[i].width = mainCanvasDims.x;
			sideCanvasChildren[i].height = mainCanvasDims.y;
		}
		TissueStack.planes[sideViewPlaneId].setDimensions(mainCanvasDims.x, mainCanvasDims.y);
		// store zoom level for side view
		var zoomLevelSideView = TissueStack.planes[sideViewPlaneId].getDataExtent().zoom_level;
		
		for (var i=0; i < mainCanvasChildren.length; i++) {
			mainCanvasChildren[i].width = sideCanvasDims.x;
			mainCanvasChildren[i].height = sideCanvasDims.y;
		}
		TissueStack.planes[mainViewPlaneId].setDimensions(sideCanvasDims.x, sideCanvasDims.y);
		
		mainCanvas.append(sideCanvasChildren);
		sideCanvas.append(mainCanvasChildren);

		// remember change in class
		$("#" + event.target.id).attr("class", "canvas_" + mainViewPlaneId);
		$("#main_view_div").attr("class", "canvas_" + sideViewPlaneId);

		
		// redraw and change the zoom level as well
		TissueStack.planes[mainViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(mainCanvasRelativeCross);
		TissueStack.planes[sideViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(sideCanvasRelativeCross);
		TissueStack.planes[sideViewPlaneId].changeToZoomLevel(TissueStack.planes[mainViewPlaneId].getDataExtent().zoom_level);
		TissueStack.planes[mainViewPlaneId].changeToZoomLevel(zoomLevelSideView);
		
		// last but not least, swap their associated info above
		var sideViewInfo = $("#"+ sideCanvasId + "_info");
		var mainViewInfo = $("#main_view_info");
		if (!sideViewInfo || !mainViewInfo) {
			return;
		}
		var sideViewInfoChildren = sideViewInfo.children();
		var mainViewInfoChildren = mainViewInfo.children();
		if (!sideViewInfoChildren || !mainViewInfoChildren || sideViewInfoChildren.length == 0 || mainViewInfoChildren.length == 0) {
			return;
		}
		
		sideViewInfoChildren.detach();
		mainViewInfoChildren.detach();
		sideViewInfo.append(mainViewInfoChildren);
		mainViewInfo.append(sideViewInfoChildren);
	});
};

$(document).ready(function() {
	TissueStack.Init();
});
