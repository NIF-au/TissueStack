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
					extent_y: 499
	            },
	            { // y plane
					id: "mouse_1",
					one_to_one_zoom_level: 3,
					plane: 'y',
					slices: 1310,
					extent_x: 679,
					extent_y: 499
	            },
	            { // z plane
					id: "mouse_1",
					one_to_one_zoom_level: 3,
					plane: 'z',
					slices: 498,
					extent_x: 679,
					extent_y: 1311
	            }
	];

	TissueStack.planes = {};
	
	// loop over data, create objects and listeners and then display them
	for (var i=0; i < data.length; i++) {
		var dataForPlane = data[i];
		var planeId = dataForPlane.plane;
		
		// create extent
		var extent = new TissueStack.Extent(dataForPlane.id, dataForPlane.one_to_one_zoom_level, planeId, dataForPlane.slices,
				dataForPlane.extent_x, dataForPlane.extent_y, zoom_levels);

		// create canvas
		var plane = new TissueStack.Canvas(extent, "canvas_" + planeId + "_plane");

		// store plane  
		TissueStack.planes[planeId] = plane;

		// bind coordinate center functionality
		(function (plane, planeId) {
			$('#center_point_in_canvas_' + planeId).bind("click", function() {
				var xCoord = parseInt($('#canvas_' + planeId + '_x').val());
				var yCoord = parseInt($('#canvas_' + planeId + '_y').val());
				
				if (!plane.redrawWithCenterAndCrossAtGivenPixelCoordinates({x: xCoord, y: yCoord})) {
					alert("Illegal coords");
				}
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
};

$(document).ready(function() {
	TissueStack.Init();
});
