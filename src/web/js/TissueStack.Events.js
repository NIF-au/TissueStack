TissueStack.Events = function(canvas) {
	this.canvas = canvas;
	this.registerCommonEvents();
	this.registerDesktopEvents();
	this.registerMobileEvents();
};

TissueStack.Events.prototype = {
	getCanvasElement : function() {
		// get coordinate cross or canvas element
		var canvasElement = this.canvas.getCoordinateCrossCanvas();
		if (!canvasElement || !canvasElement[0]) {
			canvasElement = this.canvas.getCanvasElement();
		}
		
		return canvasElement;
	},	
	registerCommonEvents: function() {
		var _this = this;
		
		// TOUCH END and MOUSE UP
		$(document).bind("touchend mouseup", function(e) {
			// call pan move
			_this.panEnd();
		});
		
		// CLICK
		this.getCanvasElement().bind("click", function(e) {
		
			if (e.originalEvent.touches) {
				var touches = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
				e.pageX = touches.pageX;
				e.pageY = touches.pageY;
			}
		
			// call click
			_this.click(e );
		});

		// SYNC
		$(document).bind("sync", function(e, timestamp, action, plane, zoom_level, slice, coords, max_coords_of_event_triggering_plane, upperLeftCorner, crossCoords, canvasDims) {
			// call sync
			_this.sync(e, timestamp, action, plane, zoom_level, slice, coords, max_coords_of_event_triggering_plane, upperLeftCorner, crossCoords, canvasDims);
		});

		// SYNC for ZOOM
		$(document).bind("zoom", function(e, timestamp, action, plane, zoom_level, slice) {
			// call sync zoom
			_this.sync_zoom(e, timestamp, action, plane, zoom_level, slice);
		});
				
	}, registerMobileEvents: function() {
		var _this = this;
		
		// TOUCH START
		this.getCanvasElement().bind("touchstart", function(e) {
		
		    if (e.originalEvent.touches.length > 1) {
				return;
			};
		
			var touches = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
			e.pageX = touches.pageX;
			e.pageY = touches.pageY;

			// call pan start
			_this.panStart(e);
		});			

		// TOUCH MOVE
		this.getCanvasElement().bind("touchmove", function(e) {
		
			if (e.originalEvent.touches.length > 1) {
				return;
			};
			
			var touches = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
			e.pageX = touches.pageX;
			e.pageY = touches.pageY;
			
			// call panAndMove
			_this.panAndMove(e);
		});

		var delta = 0;
		
		// GESTURE START
		this.getCanvasElement().bind('gesturestart', function(e) {
			delta = e.originalEvent.scale;
		});
		
		// GESTURE END
		this.getCanvasElement().bind('gestureend', function(e) {
			delta = e.originalEvent.scale - delta;
			
			// call zoom
			_this.zoom(e, delta);
		});
		
		//DOUBLE TAP TO ENLARGE IMAGES
		this.getCanvasElement().bind('doubletap', function(e) {
			if(delta < 7){
				delta = e.originalEvent.scale + delta;
				_this.zoom(e, delta);
			}
		});
		
	}, registerDesktopEvents: function() {
		var _this = this;
		
		//MOUSE DOWN
		this.getCanvasElement().bind("mousedown", function(e) {
			if (TissueStack.Utils.isLeftMouseButtonPressed(e)) {

				// call pan start
				_this.panStart(e);
			} 
		});			

		// MOUSE MOVE
		this.getCanvasElement().bind("mousemove", function(e) {
			// call panAndMove
			_this.panAndMove(e);
		});

		// MOUSE WHEEL
		this.getCanvasElement().bind('mousewheel', function(e, delta) {
			// annoying opera
			if ($.browser.opera) {
				delta = delta > 0 ? 1.5 : 0.5; 
			}
			// call zoom
			_this.zoom(e, delta);
		});
		
		// this is sadly necessary to keep the window from scrolling when only the canvas should be scrolled
		TissueStack.Utils.preventBrowserWindowScrollingWhenInCanvas();
		this.getCanvasElement().hover(function() {
			if(TissueStack.Utils.forceWindowScrollY == -1) {
				  TissueStack.Utils.forceWindowScrollY = $(window).scrollTop();
			}
		}, function() {
			TissueStack.Utils.forceWindowScrollY = -1;
		});		
	},panStart : function(e) {
		var coords = TissueStack.Utils.getRelativeMouseCoords(e);
		
		this.canvas.mouse_down = true;
		this.canvas.mouse_x = coords.x;
		this.canvas.mouse_y = coords.y;
	}, panEnd : function() {
		this.canvas.mouse_down = false;
	}, panAndMove : function(e) {
		var now =new Date().getTime(); 

		var coords = TissueStack.Utils.getRelativeMouseCoords(e);
		var relCoordinates = this.canvas.getDataCoordinates(coords);
		var worldCoordinates = this.canvas.getDataExtent().getWorldCoordinatesForPixel(relCoordinates);
		
		// update coordinate info displayed
		this.canvas.updateCoordinateInfo(coords, relCoordinates, worldCoordinates);
		
		if (this.canvas.mouse_down) {
			this.canvas.isDragging = true;
			var dX = coords.x - this.canvas.mouse_x;
			var dY = coords.y - this.canvas.mouse_y;

			this.canvas.mouse_x = coords.x;
			this.canvas.mouse_y = coords.y;
			
			this.canvas.moveUpperLeftCorner(dX, -dY);
			
			var upper_left_corner = {x: this.canvas.upper_left_x, y: this.canvas.upper_left_y};
			var cross_coords = {x: this.canvas.cross_x, y: this.canvas.cross_y};
			var canvas_dims = {x: this.canvas.dim_x, y: this.canvas.dim_y};
			
			// queue events 
			this.canvas.queue.addToQueue(
					{	timestamp : now,
						action: 'PAN',
						plane: this.canvas.getDataExtent().plane,
						zoom_level : this.canvas.getDataExtent().zoom_level,
						slice : this.canvas.getDataExtent().slice,
						coords: relCoordinates,
						max_coords_of_event_triggering_plane : {max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y},
						upperLeftCorner : upper_left_corner,
						crossCoords : cross_coords,
						canvasDims : canvas_dims
					});
			
			if (this.canvas.sync_canvases) {				
				// send message out to others that they need to redraw as well
				this.canvas.getCanvasElement().trigger("sync", 
							[	now,
							 	'PAN',
							 	this.canvas.getDataExtent().plane,
							 	this.canvas.getDataExtent().zoom_level,
							 	this.canvas.getDataExtent().slice,
							 	this.canvas.getRelativeCrossCoordinates(),
							 	{max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y},
							 	upper_left_corner,
							 	cross_coords,
							 	canvas_dims
				            ]);
			}
		} else {
			this.canvas.isDragging = false;
		}
	}, click : function(e) {
		var now = new Date().getTime(); 
		var coords = TissueStack.Utils.getRelativeMouseCoords(e);

		if (this.canvas.isDragging) {
			return;
		}

		var upper_left_corner = {x: this.canvas.upper_left_x, y: this.canvas.upper_left_y};
		var cross_coords = {x: coords.x, y: coords.y};
		var canvas_dims = {x: this.canvas.dim_x, y: this.canvas.dim_y};

		this.canvas.drawCoordinateCross(cross_coords);

		if (this.canvas.sync_canvases) {				
			// send message out to others that they need to redraw as well
			this.canvas.getCanvasElement().trigger("sync", [now,
									'CLICK',
									this.canvas.getDataExtent().plane,
									this.canvas.getDataExtent().zoom_level,
									this.canvas.getDataExtent().slice,
									this.canvas.getRelativeCrossCoordinates(),
			                        {max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y},
			                        upper_left_corner,
			                        cross_coords,
			                        canvas_dims
			                       ]);
		}
	}, sync : function(e, timestamp, action, plane, zoom_level, slice, coords, max_coords_of_event_triggering_plane, upperLeftCorner, crossCoords, canvasDims) {
		// ignore one's own events
		var thisHerePlane = this.canvas.getDataExtent().plane;
		if (thisHerePlane === plane) {
			return;
		}
		
		// queue events 
		this.canvas.queue.addToQueue(
				{	timestamp : timestamp,
					action : action,
					plane: plane,
					zoom_level : zoom_level,
					slice : slice,
					coords: coords,
					max_coords_of_event_triggering_plane : max_coords_of_event_triggering_plane,
					upperLeftCorner: upperLeftCorner,
					crossCoords : crossCoords,
					canvasDims : canvasDims
				});
	}, zoom : function(e, delta) {
		// make sure zoom delta is whole number
		if (delta < 1) {
			delta = -1;
		} else if (delta > 1) {
			delta = 1;
		} 
							
		var newZoomLevel = this.canvas.getDataExtent().zoom_level + delta;
		if (newZoomLevel == this.canvas.data_extent.zoom_level ||  newZoomLevel < 0 || newZoomLevel >= this.canvas.data_extent.zoom_levels.length) {
			return;
			}
		
		var now = new Date().getTime();
		
		this.canvas.queue.addToQueue(
				{	timestamp : now,
					action : "ZOOM",
					plane: this.canvas.getDataExtent().plane,
					zoom_level : newZoomLevel,
					slice : this.canvas.getDataExtent().slice
					
				});
		event.stopPropagation();
					
		/* let's not sync zooms for now
		if (this.canvas.sync_canvases) {				
			// send message out to others that they need to redraw as well
			canvas.trigger("zoom", 
						[	now,
						 	"ZOOM",
						 	this.canvas.getDataExtent().plane,
						 	newZoomLevel,
						 	this.canvas.getDataExtent().slice
			            ]);
		}*/
	}, sync_zoom : function(e, timestamp, action, plane, zoom_level, slice) {
		// ignore one's own events
		var thisHerePlane = this.canvas.getDataExtent().plane;
		if (thisHerePlane === plane) {
			return;
		}

		this.canvas.queue.addToQueue(
				{	timestamp : timestamp,
					action : action,
					plane: plane,
					zoom_level : zoom_level,
					slice : slice
				});
		
	}
};