TissueStack.Init = function () {
	// create an extent for each of the three planes
	// TODO: this info will come from the back-end most likely. for now hard-code!
	var zoom_levels = [0.25, // level 0
	                   0.5,  // level 1
	                   0.75, // level 2
	                   1, 	 // level 3 == 1:1
	                   1.25, // level 4
	                   1.5,  // level 5
	                   1.75  // level 6
	                  ];
	
	var brain_extent_x_plane = new TissueStack.Extent();
	brain_extent_x_plane.init("mouse_1", 3, 'x', 678, 1311, 499, zoom_levels);

	var brain_extent_y_plane = new TissueStack.Extent();
	brain_extent_y_plane.init("mouse_1", 3, 'y', 1310, 679, 499, zoom_levels);
	
	var brain_extent_z_plane = new TissueStack.Extent();
	brain_extent_z_plane.init("mouse_1", 3, 'z', 498, 679, 1311, zoom_levels);
	
	// create three instances of the brain canvas (1 for each plane)
	TissueStack.brain_canvas_x_plane = new TissueStack.Canvas();
	TissueStack.brain_canvas_x_plane.init(brain_extent_x_plane, "canvas_x_plane");

	TissueStack.brain_canvas_y_plane = new TissueStack.Canvas();
	TissueStack.brain_canvas_y_plane.init(brain_extent_y_plane, "canvas_y_plane");

	TissueStack.brain_canvas_z_plane = new TissueStack.Canvas();
	TissueStack.brain_canvas_z_plane.init(brain_extent_z_plane, "canvas_z_plane");

	// show total data extent and canvas dimensions
	var log = $('#total_data_extent');
	log.html("Total Data Extent (z): " + TissueStack.brain_canvas_z_plane.getDataExtent().x  + " x " + TissueStack.brain_canvas_z_plane.getDataExtent().y);
	log = $('#canvas_dimensions');
	log.html("Canvas Dimensions: " + TissueStack.brain_canvas_z_plane.dim_x  + " x " + TissueStack.brain_canvas_z_plane.dim_y);
	
	TissueStack.brain_canvas_x_plane.changeToZoomLevel(0);
	TissueStack.brain_canvas_y_plane.changeToZoomLevel(0);
	TissueStack.brain_canvas_z_plane.changeToZoomLevel(0);
	
	TissueStack.brain_canvas_x_plane.queue.drawLowResolutionPreview();
	TissueStack.brain_canvas_y_plane.queue.drawLowResolutionPreview();
	TissueStack.brain_canvas_z_plane.queue.drawLowResolutionPreview();
	
	// draw 
	TissueStack.brain_canvas_x_plane.queue.drawRequestAfterLowResolutionPreview();
	TissueStack.brain_canvas_y_plane.queue.drawRequestAfterLowResolutionPreview();
	TissueStack.brain_canvas_z_plane.queue.drawRequestAfterLowResolutionPreview();
	
	$('#drawing_interval').val(TissueStack.brain_canvas_x_plane.queue.drawingIntervalInMillis);
	// bind event for queue interval change
	$('#drawing_interval_button').bind("click", function() {
		var newValue = parseInt($('#drawing_interval').val());
		TissueStack.brain_canvas_x_plane.queue.setDrawingInterval(newValue);
		TissueStack.brain_canvas_y_plane.queue.setDrawingInterval(newValue);
		TissueStack.brain_canvas_z_plane.queue.setDrawingInterval(newValue);
	});
	$('#zoom_level_selection').bind("change", function() {
		var newValue = parseInt($('#zoom_level_selection').val());
		TissueStack.brain_canvas_x_plane.changeToZoomLevel(newValue);
		TissueStack.brain_canvas_y_plane.changeToZoomLevel(newValue);
		TissueStack.brain_canvas_z_plane.changeToZoomLevel(newValue);	
	});
};

$(document).ready(function() {
	TissueStack.Init();
});