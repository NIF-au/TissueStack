TissueStack.Canvas = function(data_extent, canvas_id) {
	this.setDataExtent(data_extent);
	this.setCanvasElement(canvas_id);
	// set dimensions
	var tmpCanvasElement = this.getCanvasElement()[0];
	this.setDimensions(tmpCanvasElement.width, tmpCanvasElement.height);
	this.centerUpperLeftCorner();
	this.drawCoordinateCross(this.getCenter());
	this.registerMouseEvents();
	this.queue = new TissueStack.Queue(this);
};

TissueStack.Canvas.prototype = {
	data_extent: null,
	canvas_id: "canvas_" + this.plane, 
	image_format: 'png',
	mouse_down : false,
	isDragging : false,
	dim_x : 0,
	dim_y : 0,
	mouse_x : 0,
	mouse_y : 0,
	upper_left_x : 0,
	upper_left_y : 0,
	cross_x : 0,
	cross_y : 0,
	queue : null,
	init : function (data_extent, canvas_id) {
		this.setDataExtent(data_extent);
		this.setCanvasElement(canvas_id);
		// set dimensions
		var tmpCanvasElement = this.getCanvasElement()[0];
		this.setDimensions(tmpCanvasElement.width, tmpCanvasElement.height);
		this.centerUpperLeftCorner();
		this.drawCoordinateCross(this.getCenter());
		this.registerMouseEvents();
		this.queue = new TissueStack.Queue(this);
	},
	setDataExtent : function (data_extent) {
		if (typeof(data_extent) != "object") {
			throw "we miss a data_extent";
		}
		this.data_extent = data_extent;
	},
	setCanvasElement : function(canvas_id) {
		if (canvas_id && (typeof(canvas_id) != "string" || canvas_id.length == 0)) {
			throw "canvas_id has to be a non-empty string";
		}
		this.canvas_id = canvas_id;
		if (!$("#" + this.canvas_id)) {
			throw "Canvas element with id " + this.canvas_id + " does not exist!";
		}
	},
	getCanvasElement : function() {
		return $("#" + this.canvas_id);
	},
	getCanvasContext : function() {
		return this.getCanvasElement()[0].getContext("2d");
	},
	getDataExtent : function() {
		return this.data_extent;
	},
	changeToZoomLevel : function(zoom_level) {
		if (typeof(zoom_level) != 'number') {
			return;
		}
		zoom_level = Math.floor(zoom_level);
		if (zoom_level < 0 || zoom_level >= this.getDataExtent().zoom_levels.length || zoom_level ==  this.getDataExtent().zoom_level) {
			return;
		}
		
		this.getDataExtent().changeToZoomLevel(zoom_level);

		var centerAfterZoom = this.getCenterForPoint({x: this.cross_x, y: this.cross_y});
		this.setUpperLeftCorner(Math.floor(centerAfterZoom.x), Math.floor(centerAfterZoom.y));
	},
	getDataCoordinates : function(relative_mouse_coords) {
		var relDataX = -1;
		var relDataY = -1;
		if (this.upper_left_x < 0 && relative_mouse_coords.x <= (this.upper_left_x + this.getDataExtent().x)) {
			relDataX = Math.abs(this.upper_left_x) + relative_mouse_coords.x;
		} else if (this.upper_left_x >= 0 && relative_mouse_coords.x >= this.upper_left_x && relative_mouse_coords.x <= this.upper_left_x + this.getDataExtent().x) {
			relDataX = relative_mouse_coords.x - this.upper_left_x;
		}
		if (this.upper_left_y > 0 && this.upper_left_y - this.getDataExtent().y < this.dim_y && this.dim_y - relative_mouse_coords.y <= this.upper_left_y && this.dim_y - relative_mouse_coords.y >= this.upper_left_y - this.getDataExtent().y) {
			relDataY = this.upper_left_y - (this.dim_y - relative_mouse_coords.y);
		}
		
		return {x: relDataX, y: relDataY};
	},
	setDimensions : function(x,y) {
		if (typeof(x) != "number" || Math.floor(x) < 0) {
			throw "x has to be a non-negative integer";
		}
		this.dim_x = x;
		if (typeof(y) != "number" || Math.floor(y) < 0) {
			throw "y has to be a non-negative integer";
		}
		this.dim_y = y;
	},
	getCenter : function () {
		return TissueStack.Utils.getCenter(this.dim_x,this.dim_y);
	},
	getCoordinateCrossCanvas : function() {
		return $("#" + this.canvas_id + "_cross_overlay");
	},
	getRelativeCrossCoordinates : function() {
		var relCrossX = -1;
		var relCrossY = -1;
		if (this.upper_left_x < 0 && this.cross_x <= (this.upper_left_x + this.getDataExtent().x)) {
			relCrossX = Math.abs(this.upper_left_x) + this.cross_x;
		} else if (this.upper_left_x >= 0 && this.cross_x >= this.upper_left_x && this.cross_x <= this.upper_left_x + this.getDataExtent().x) {
			relCrossX = this.cross_x - this.upper_left_x;
		}
		if (this.upper_left_y > 0 && this.upper_left_y - this.getDataExtent().y < this.dim_y && this.dim_y - this.cross_y <= this.upper_left_y && this.dim_y - this.cross_y >= this.upper_left_y - this.getDataExtent().y) {
			relCrossY = this.upper_left_y - (this.dim_y - this.cross_y);
		}
		
		return {x: relCrossX, y: relCrossY};
	},
	registerMouseEvents : function () {
		// look for the cross overlay which will be the top layer
		var canvas = this.getCoordinateCrossCanvas();
		if (!canvas || !canvas[0]) {
			canvas = this.getCanvasElement();
		}

		var _this = this;

		// bind mouse down and up events
		canvas.bind("mousedown", function(e) {
			if (TissueStack.Utils.isLeftMouseButtonPressed(e)) {
				var coords = TissueStack.Utils.getRelativeMouseCoords(e);

				_this.mouse_down = true;
				_this.mouse_x = coords.x;
				_this.mouse_y = coords.y;
			 } 
		});
		$(document).bind("mouseup", function(e) {
			_this.mouse_down = false;
		});
		
		// bind the mouse move event
		canvas.bind("mousemove", function(e) {
			var now =new Date().getTime(); 
			var coords = TissueStack.Utils.getRelativeMouseCoords(e);
								
			var log = $('#coords');
			log.html("Canvas Coordinates X: " + coords.x + ", Canvas Y: " + coords.y);
			log = $('#relative_coords');
			
			var relCoordinates = _this.getDataCoordinates(coords);
			log.html("Data Coordinates (in pixels) X: " + relCoordinates.x + ", Data Y: " + relCoordinates.y);
			
			if (_this.mouse_down) {
				_this.isDragging = true;
				var dX = coords.x - _this.mouse_x;
				var dY = coords.y - _this.mouse_y;

				_this.mouse_x = coords.x;
				_this.mouse_y = coords.y;
				
				_this.moveUpperLeftCorner(dX, -dY);
				
				var upper_left_corner = {x: _this.upper_left_x, y: _this.upper_left_y};
				
				// queue events 
				_this.queue.addToQueue(
						{	timestamp : now,
							plane: _this.getDataExtent().plane,
							zoom_level : _this.getDataExtent().zoom_level,
							slice : _this.getDataExtent().slice,
							coords: relCoordinates,
							max_coords_of_event_triggering_plane : {max_x: _this.getDataExtent().x, max_y: _this.getDataExtent().y},
							move : true,
							upperLeftCornerOrCrossCoords: upper_left_corner
						});
				
				// send message out to others that they need to redraw as well
				_this.getCanvasElement().trigger("sync", 
							[	now,
							 	'PAN',
							 	_this.getDataExtent().plane,
							 	_this.getDataExtent().zoom_level,
							 	_this.getDataExtent().slice,
							 	_this.getRelativeCrossCoordinates(),
							 	{max_x: _this.getDataExtent().x, max_y: _this.getDataExtent().y},
				                upper_left_corner
				            ]);
			} else {
				_this.isDragging = false;
			}
		});

		// optionally, if the overlay canvas is defined, we register the mouse handler for click, to draw the cross
		var coordinateCrossCanvas = this.getCoordinateCrossCanvas();
		if (coordinateCrossCanvas && coordinateCrossCanvas[0]) {
			canvas.bind("click", function(e) {
				var now = new Date().getTime(); 
				var coords = TissueStack.Utils.getRelativeMouseCoords(e);

				if (_this.isDragging) {
					return;
				}

				_this.drawCoordinateCross(coords);

				// send message out to others that they need to redraw as well
				canvas.trigger("sync", [now,
										'CLICK',
				                        _this.getDataExtent().plane,
				                        _this.getDataExtent().zoom_level,
				                        _this.getDataExtent().slice,
				                        _this.getDataCoordinates(coords),
				                        {max_x: _this.getDataExtent().x, max_y: _this.getDataExtent().y},
										{x: coords.x, y: coords.y}
				                       ]);

			});

			$(document).bind("sync", function(e, timestamp, action, plane, zoom_level, slice, coords, max_coords_of_event_triggering_plane, upperLeftCornerOrCrossCoords) {
				// ignore one's own events
				var thisHerePlane = _this.getDataExtent().plane;
				if (thisHerePlane === plane) {
					return;
				}
				
				// queue events 
				_this.queue.addToQueue(
						{	timestamp : timestamp,
							action : action,
							plane: plane,
							zoom_level : zoom_level,
							slice : slice,
							coords: coords,
							max_coords_of_event_triggering_plane : max_coords_of_event_triggering_plane,
							upperLeftCornerOrCrossCoords : upperLeftCornerOrCrossCoords
						});
			});
		}

		// bind the mouse wheel scroll event
		canvas.bind('mousewheel', function(event, delta) {
			var newZoomLevel = _this.getDataExtent().zoom_level + delta;
			var now = new Date().getTime();
			
			_this.queue.addToQueue(
					{	timestamp : now,
						action : "ZOOM",
						plane: _this.getDataExtent().plane,
						zoom_level : newZoomLevel,
						slice : _this.getDataExtent().slice
					});

			// send message out to others that they need to redraw as well
			canvas.trigger("zoom", 
						[	now,
						 	"ZOOM",
						 	_this.getDataExtent().plane,
						 	newZoomLevel,
						 	_this.getDataExtent().slice
			            ]);
		});
		
		$(document).bind("zoom", function(e, timestamp, action, plane, zoom_level, slice) {
			// ignore one's own events
			var thisHerePlane = _this.getDataExtent().plane;
			if (thisHerePlane === plane) {
				return;
			}

			_this.queue.addToQueue(
					{	timestamp : timestamp,
						action : action,
						plane: plane,
						zoom_level : zoom_level,
						slice : slice
					});
		});
	},
	drawCoordinateCross : function(coords) {
		var coordinateCrossCanvas = this.getCoordinateCrossCanvas();
		if (!coordinateCrossCanvas || !coordinateCrossCanvas[0]) {
			return;
		}
		
		// store cross coords
		this.cross_x = coords.x;
		this.cross_y = coords.y;
		
		var ctx = coordinateCrossCanvas[0].getContext("2d");
		// clear overlay
		ctx.save();
		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0,0, this.dim_x, this.dim_y);
		ctx.restore();
		
		// draw bulls eye
		ctx.beginPath();
		ctx.strokeStyle="rgba(200,0,0,0.3)";
		ctx.lineWidth=1.0;

		ctx.moveTo(coords.x + 0.5, 0);
		ctx.lineTo(coords.x + 0.5, (coords.y - 10));
		ctx.moveTo(coords.x + 0.5, (coords.y + 10));
		ctx.lineTo(coords.x + 0.5, this.dim_y);
		ctx.stroke();

		ctx.moveTo(0, coords.y + 0.5);
		ctx.lineTo(coords.x - 10, coords.y + 0.5);
		ctx.moveTo(coords.x + 10, coords.y + 0.5);
		ctx.lineTo(this.dim_x, coords.y + 0.5);
		ctx.stroke();
		ctx.closePath();
	},
	setUpperLeftCorner : function(x,y) {
		this.upper_left_x = x;
		this.upper_left_y = y;
	},
	moveUpperLeftCorner : function(deltaX,deltaY) {
		this.upper_left_x += deltaX;
		this.upper_left_y += deltaY;
	},
	centerUpperLeftCorner : function() {
		var center = this.getCenteredUpperLeftCorner();
		this.setUpperLeftCorner(Math.floor(center.x), Math.floor(center.y));
	},
	getCenteredUpperLeftCorner : function() {
		return {x: (this.dim_x - this.getDataExtent().x) / 2,
				y: this.dim_y - ((this.dim_y - this.getDataExtent().y) / 2)
		};
	}, 
	getCenterForPoint : function(coords) {
		return {x: (this.dim_x - this.getDataExtent().x) / (this.dim_x / coords.x),
				y: this.dim_y - ((this.dim_y - this.getDataExtent().y) / (this.dim_y / coords.y))
		};
	},
	redrawWithCenterAndCrossAtGivenPixelCoordinates: function(coords) {
		if (coords.x < 0 || coords.x > this.getDataExtent().x 
				|| coords.y < 0 || coords.y > this.getDataExtent().y) {
			return false;
		}
		// this stops any still running draw requests 
		var now = new Date().getTime(); 
		this.queue.latestDrawRequestTimestamp = now;

		this.eraseCanvasContent();
		
		// make sure crosshair is centered:
		this.drawCoordinateCross(this.getCenter());
		
		this.setUpperLeftCorner(
				Math.floor(this.dim_x / 2) - coords.x,
				Math.floor(this.dim_y / 2) + coords.y);
		this.queue.drawLowResolutionPreview(now);
		this.queue.drawRequestAfterLowResolutionPreview(null,now);
		
		return true;
	},
	eraseCanvasContent: function() {
    	var ctx = this.getCanvasContext();
    	
    	var myImageData = ctx.getImageData(0, 0, this.dim_x, this.dim_y);
    	for ( var x = 0; x < this.dim_x * this.dim_y * 4; x += 4) {
    		myImageData.data[x] = myImageData.data[x + 1] = myImageData.data[x + 2] = 255;
    		myImageData.data[x + 3] = 0;
    	}
    	ctx.putImageData(myImageData, 0, 0);
	},
	eraseCanvasPortion: function(x, y, w, h) {
    	var ctx = this.getCanvasContext();
    	
    	var myImageData = ctx.getImageData(x, y, w, h);
    	for ( var i = 0; i < w * h * 4; i += 4) {
    		myImageData.data[i] = myImageData.data[i + 1] = myImageData.data[i + 2] = 255;
    		myImageData.data[i + 3] = 0;
    	}
    	ctx.putImageData(myImageData, x, y);
	}, drawMe : function(timestamp) {
		// preliminary check if we are within the slice range
		var slice = this.getDataExtent().slice;
		if (slice < 0 || slice > this.getDataExtent().max_slices) {
			return;
		}
		
		var ctx = this.getCanvasContext();

		// nothing to do if we are totally outside
		if (this.upper_left_x < 0 && (this.upper_left_x + this.getDataExtent().x) <=0
				|| this.upper_left_x > 0 && this.upper_left_x > this.dim_x
				|| this.upper_left_y <=0 || (this.upper_left_y - this.getDataExtent().y) >= this.dim_y) {
			return;
		} 
		
		var startTileX = this.upper_left_x / this.getDataExtent().tile_size;
		var canvasX = 0;
		var deltaStartTileXAndUpperLeftCornerX = 0;
		if (startTileX < 0) {
			startTileX = Math.abs(Math.ceil(startTileX));
			deltaStartTileXAndUpperLeftCornerX =  this.getDataExtent().tile_size - Math.abs(this.upper_left_x + startTileX * this.getDataExtent().tile_size);
		} else {
			//startTileX = Math.floor(startTileX);
			startTileX = 0;
			canvasX = this.upper_left_x;
		}
		
		var startTileY = 0;
		var canvasY = 0;
		var deltaStartTileYAndUpperLeftCornerY = 0;
		if (this.upper_left_y <= this.dim_y) {
			//startTileY = Math.floor((this.dim_y - this.upper_left_y)  / this.getDataExtent().tile_size);
			canvasY = this.dim_y - this.upper_left_y;
		} else {
			startTileY = Math.floor((this.upper_left_y - this.dim_y)  / this.getDataExtent().tile_size);
			deltaStartTileYAndUpperLeftCornerY = this.getDataExtent().tile_size - (this.upper_left_y - startTileY * this.getDataExtent().tile_size - this.dim_y);
		}

		// for now set end at data extent, we cut off in the loop later once we hit the canvas bounds or data bounds which ever occurs first
		var endTileX = Math.floor(this.getDataExtent().x / this.getDataExtent().tile_size) + ((this.getDataExtent().x % this.getDataExtent().tile_size == 0) ? 0 : 1);
		var endTileY = Math.floor(this.getDataExtent().y / this.getDataExtent().tile_size) + ((this.getDataExtent().y % this.getDataExtent().tile_size == 0) ? 0 : 1);

		var copyOfCanvasY = canvasY;

		// loop over rows
		for (var tileX = startTileX  ; tileX < endTileX ; tileX++) {
			var tileOffsetX = startTileX * this.getDataExtent().tile_size;
			var imageOffsetX = 0;
			var width =  this.getDataExtent().tile_size;
			var rowIndex = tileX; 
			
			// reset to initial canvasX
			canvasY = copyOfCanvasY;
			
			// we are at the beginning, do we have a partial?
			if (canvasX == 0 && deltaStartTileXAndUpperLeftCornerX !=0) {
				width = deltaStartTileXAndUpperLeftCornerX;
				imageOffsetX =  this.getDataExtent().tile_size - width;
			}
			
			// we exceed canvas
			if (canvasX == this.dim_x) {
				tileX = endTileX; // this will make us stop in the next iteration 
				width =  this.dim_x - tileOffsetX;
			} else if (canvasX > this.dim_x) {
				break;
			}

			// walk through columns
			for (var tileY = startTileY ; tileY < endTileY ; tileY++) {
				var imageOffsetY = 0;
				var height =  this.getDataExtent().tile_size;
				var colIndex = tileY; 

				// we are at the beginning, do we have a partial?
				if (canvasY == 0 && deltaStartTileYAndUpperLeftCornerY !=0) {
					height = deltaStartTileYAndUpperLeftCornerY;
					imageOffsetY =  this.getDataExtent().tile_size - height;
				}

				if (canvasY  == this.dim_y) {
					tileY = endTileY; // this will make us stop in the next iteration 
					height = this.dim_y - canvasY;
				} else if (canvasY > this.dim_y) {
					break;
				}

				// create the image object that loads the tile we need
				var imageTile = new Image();
				imageTile.src = 
					TissueStack.tile_directory + this.getDataExtent().data_id + "/" + this.getDataExtent().zoom_level + "/" + this.getDataExtent().plane
					+ "/" + slice + "/" + rowIndex + '_' + colIndex + "." + this.image_format;

				(function(_this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height, deltaStartTileXAndUpperLeftCornerX, deltaStartTileYAndUpperLeftCornerY, tile_size) {
					imageTile.onload = function() {
						// check with actual image dimensions ...
						if (canvasX == 0 && width != tile_size && deltaStartTileXAndUpperLeftCornerX !=0) {
							imageOffsetX = (tile_size - deltaStartTileXAndUpperLeftCornerX);
							width = this.width - imageOffsetX;
						} else if (this.width < width) {
								width = this.width;
						}

						if (imageOffsetX < 0 || imageOffsetY < 0 || width < 0 || height < 0 || canvasX < 0 || canvasY < 0) {
							console.info("no!");
						}

						if (canvasY == 0 && height != tile_size && deltaStartTileYAndUpperLeftCornerY !=0) {
								imageOffsetY = (tile_size - deltaStartTileYAndUpperLeftCornerY);
								height = this.height - imageOffsetY;
						} else	if (this.height < height) {
								height = this.height;
						}

						// damn you async loads
						if (timestamp && timestamp < _this.queue.latestDrawRequestTimestamp) {
							return;
						}
						
						ctx.drawImage(this,
								imageOffsetX, imageOffsetY, width, height, // tile dimensions
								canvasX, canvasY, width, height); // canvas dimensions
					};
				})(this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height, deltaStartTileXAndUpperLeftCornerX, deltaStartTileYAndUpperLeftCornerY, this.getDataExtent().tile_size);
				
				// increment canvasY
				canvasY += height;
			}
			
			// increment canvasX
			canvasX += width;
		};
	}
};
