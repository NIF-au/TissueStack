utils = {
		getRelativeMouseCoords : function(event) {
			var totalOffsetX = 0;
			var totalOffsetY = 0;
			var x = 0;
			var y = 0;
			var currentElement = event.currentTarget;

			do {
				totalOffsetX += currentElement.offsetLeft;
				totalOffsetY += currentElement.offsetTop;
			} while (currentElement = currentElement.offsetParent)

			x = event.pageX - totalOffsetX;
			y = event.pageY - totalOffsetY;

			return {
				x : x,
				y : y
			};
		},
		isLeftMouseButtonPressed : function(event) {
			// IE compatibility
			 if (!event.which && event.button) {
				     if (event.button & 1) {
				    	 event.which = 1;      // Left
				     } else if (event.button & 4) {
				    	 event.which = 2; // Middle
				     } else if (e.button & 2) {
				    	 event.which = 3; // Right
				     }
			}
			if (event.which == 1) {
				return true;
			}
			return false;
		},
		getCenter : function(x,y) {
			return {x :  Math.floor(x / 2),  y : Math.floor(y / 2)};
	}
};

brain = {
	tile_directory : "tiles/",
	extent : function () {
		return {
			tile_size : 256,	
			x : 0,
			y : 0,
			data_id: "",
			zoom_level : 0,
			plane: 'z',
			slice: 0,
			init : function (data_id, zoom_level, plane, slice, x, y) {
				this.setDataId(data_id);
				this.setZoomLevel(zoom_level);
				this.setPlane(plane);
				this.setSlice(slice);
				this.setDimensions(x, y);
			}, setDataId : function(data_id) {
				if (typeof(data_id) != "string" || data_id.trim().length == 0) {
					throw "data_id has to be a non-empty string";
				}
				this.data_id = data_id;
			}, setZoomLevel : function(zoom_level) {
				if (typeof(zoom_level) != "number" || Math.floor(zoom_level) < 0) {
					throw "zoom_level has to be a non-negative integer";
				}
				this.zoom_level = zoom_level;
			}, setPlane : function(plane) {
				if (typeof(plane) != "string" || !(plane == 'x' || plane == 'y' || plane == 'z')) {
					throw "plane has to be one of the following: 'z', 'y' or 'z'";
				}
				this.plane = plane;
			}, setSlice : function(slice) {
				if (typeof(slice) != "number" || Math.floor(slice) < 0) {
					throw "slice has to be a non-negative integer";
				}
				this.slice = slice;
			}, setDimensions : function(x,y) {
				if (typeof(x) != "number" || Math.floor(x) < 0) {
					throw "x has to be a non-negative integer";
				}
				this.x = x;
				if (typeof(y) != "number" || Math.floor(y) < 0) {
					throw "y has to be a non-negative integer";
				}
				this.y = y;
			},
			getCenter : function () {
				return utils.getCenter(this.x,this.y);
			}
		};
	},
	canvas : function () {
		return {
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
			most_recent_draw_request : 0,
			init : function (data_extent, canvas_id) {
				this.setDataExtent(data_extent);
				this.setCanvasElement(canvas_id);
				// set dimensions
				var tmpCanvasElement = this.getCanvasElement()[0];
				this.setDimensions(tmpCanvasElement.width, tmpCanvasElement.height);
				this.registerMouseEvents();
			},
			setDataExtent : function (data_extent) {
				if (typeof(data_extent) != "object") {
					throw "we miss a data_extent";
				}
				this.data_extent = data_extent;
			},
			setCanvasElement : function(canvas_id) {
				if (canvas_id && (typeof(canvas_id) != "string" || canvas_id.trim().length == 0)) {
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
			getDataCoordinates : function(relative_mouse_coords) {
				var relX = (this.upper_left_x + relative_mouse_coords.x);
				var relY = (this.upper_left_y + relative_mouse_coords.y);

				return {x: (relX < 0 ? 0 : relX), y: (relY < 0 ? 0 : relY)};
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
				return utils.getCenter(this.dim_x,this.dim_y);
			},
			getCoordinateCrossCanvas : function() {
				return $("#" + this.canvas_id + "_cross_overlay");
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
					if (utils.isLeftMouseButtonPressed(e)) {
						var coords = utils.getRelativeMouseCoords(e);

						_this.mouse_down = true;
						_this.mouse_x = coords.x;
						_this.mouse_y = coords.y;
					 } 
				});
				$(document).bind("mouseup", function(e) {
					_this.mouse_down = false;
					_this.mouse_x = 0;
					_this.mouse_y = 0;
				});

				// bind the mouse move event
				canvas.bind("mousemove", function(e) {
					var coords = utils.getRelativeMouseCoords(e);
					
					var log = $('#coords');
					log.html("Canvas Coordinates X: " + coords.x + ", Canvas Y: " + coords.y);
					log = $('#relative_coords');
					
					var relCoordinates = _this.getDataCoordinates(coords);
					log.html("Data Coordinates (in pixels) X: " + relCoordinates.x + ", Data Y: " + relCoordinates.y);

					if (_this.mouse_down) {
						_this.isDragging = true;
						var dX = _this.mouse_x - coords.x;
						var dY = _this.mouse_y - coords.y;

						_this.mouse_x = coords.x;
						_this.mouse_y = coords.y;

						_this.moveUpperLeftCorner(dX, dY);
						
						// create the delay drawing if more requests come in in short intervals (to use for setTimeout)
						var delayedDrawMe = function() {
							var now =new Date().getTime(); 
							if (now > _this.most_recent_draw_request && now-_this.most_recent_draw_request < 5) {
								// we don't draw if we have another draw request following within a short time span
								// unless the overall delay exceeds a certain threshold (see if above)
								return;
							}
							// update timestamp
							_this.most_recent_draw_request = now;

							// tidy up where we left debris
							if (_this.dim_x > _this.getDataExtent().x && dX != 0) {
								// first x debris
								if (dX < 0) {	// up front 
									_this.eraseCanvasPortion(0, 0, Math.abs(_this.upper_left_x), _this.dim_y);
								} else { // behind us
									_this.eraseCanvasPortion(
											Math.abs(_this.upper_left_x) + _this.getDataExtent().x, 0,
											_this.dim_x - _this.getDataExtent().x, _this.dim_y);
								}
							}
							
							if (_this.dim_y > _this.getDataExtent().y && dY != 0) {
								// now y debris
								if (dY < 0) {	// up front 
									// erase x debris
									_this.eraseCanvasPortion(0, 0, _this.dim_x, Math.abs(_this.upper_left_y));
								} else {
									_this.eraseCanvasPortion(
											0, Math.abs(_this.upper_left_y) + _this.getDataExtent().y,
											_this.dim_x, _this.dim_y - _this.getDataExtent().y);
								}
							}

							// redraw
							_this.drawMe();
							
							// send message out to others that they need to redraw as well
							_this.getCanvasElement().trigger("sync", [_this.getDataExtent().plane,
							                        _this.getDataCoordinates(coords),
							                        {max_x: _this.getDataExtent().x, max_y: _this.getDataExtent().y}
							                       ]);
						};
						setTimeout(delayedDrawMe, 10);

					} else {
						_this.isDragging = false;
					}
				});

				// optionally, if the overlay canvas is defined, we register the mouse handler for click, to draw the cross
				var coordinateCrossCanvas = this.getCoordinateCrossCanvas();
				if (coordinateCrossCanvas && coordinateCrossCanvas[0]) {
					canvas.bind("click", function(e) {
						var coords = utils.getRelativeMouseCoords(e);

						// send message out to others that they need to redraw as well
						canvas.trigger("sync", [_this.getDataExtent().plane,
						                        _this.getDataCoordinates(coords),
						                        {max_x: _this.getDataExtent().x, max_y: _this.getDataExtent().y}
						                       ]);

						if (_this.isDragging) {
							return;
						}

						_this.drawCoordinateCross(coords);
					});

					$(document).bind("sync", function(e, plane, coords, max_coords_of_event_triggering_plane) {
						// ignore one's own events
						var thisHerePlane = _this.getDataExtent().plane;
						if (thisHerePlane === plane) {
							return;
						}
						
						if (thisHerePlane === 'x' && plane === 'z') {
							_this.getDataExtent().slice = coords.x;
							//_this.drawCoordinateCross(coords.x, _this.cross_y);
						} else if (thisHerePlane === 'y' && plane === 'z') {
							_this.getDataExtent().slice = max_coords_of_event_triggering_plane.max_y - coords.y;
							//_this.drawCoordinateCross(max_coords_of_event_triggering_plane.max_y - coords.y, _this.cross_y);
						} else if (thisHerePlane === 'x' && plane === 'y') {
							_this.getDataExtent().slice = coords.x;
							//_this.drawCoordinateCross(_this.cross_x, coords.x);
						} else if (thisHerePlane === 'z' && plane === 'y') {
							_this.getDataExtent().slice = max_coords_of_event_triggering_plane.max_y - coords.y;
							//_this.drawCoordinateCross(max_coords_of_event_triggering_plane.max_y - coords.y, _this.cross_y);
						} else if (thisHerePlane === 'y' && plane === 'x') {
							_this.getDataExtent().slice = coords.x;
							//_this.drawCoordinateCross(_this.cross_x, coords.x);
						} else if (thisHerePlane === 'z' && plane === 'x') {
							_this.getDataExtent().slice = max_coords_of_event_triggering_plane.max_y - coords.y;
							//_this.drawCoordinateCross(_this.cross_x, max_coords_of_event_triggering_plane.max_y - coords.y);
						}
						
						var delayedDrawMe = function() {
							var now =new Date().getTime(); 
							if (now > _this.most_recent_draw_request && now-_this.most_recent_draw_request < 5) {
								// we don't draw if we have another draw request following within a short time span
								// unless the overall delay exceeds a certain threshold (see if above)
								return;
							}
							// update timestamp
							_this.most_recent_draw_request = now;

							// tidy up where we left debris
							if (_this.dim_x > _this.getDataExtent().x && dX != 0) {
								// first x debris
								if (dX < 0) {	// up front 
									_this.eraseCanvasPortion(0, 0, Math.abs(_this.upper_left_x), _this.dim_y);
								} else { // behind us
									_this.eraseCanvasPortion(
											Math.abs(_this.upper_left_x) + _this.getDataExtent().x, 0,
											_this.dim_x - _this.getDataExtent().x, _this.dim_y);
								}
							}
							
							if (_this.dim_y > _this.getDataExtent().y && dY != 0) {
								// now y debris
								if (dY < 0) {	// up front 
									// erase x debris
									_this.eraseCanvasPortion(0, 0, _this.dim_x, Math.abs(_this.upper_left_y));
								} else {
									_this.eraseCanvasPortion(
											0, Math.abs(_this.upper_left_y) + _this.getDataExtent().y,
											_this.dim_x, _this.dim_y - _this.getDataExtent().y);
								}
							}

							// redraw
							_this.drawMe();
						};
						setTimeout(delayedDrawMe, 10);
						
					});
					
				}
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
				ctx.strokeStyle="#FF0000";
				ctx.moveTo(coords.x, 0);
				ctx.lineTo(coords.x, coords.y - 10);
				ctx.moveTo(coords.x, coords.y + 10);
				ctx.lineTo(coords.x, this.dim_y);
				ctx.stroke();

				ctx.moveTo(0, coords.y);
				ctx.lineTo(coords.x - 10, coords.y);
				ctx.moveTo(coords.x + 10, coords.y);
				ctx.lineTo(this.dim_x, coords.y);
				ctx.stroke();
				ctx.closePath();
			},
			setUpperLeftCorner : function(x,y) {
				this.upper_left_x = x;
				this.upper_left_y = y;
				this.correctUpperLeftCornerToFitExtent();
			},
			moveUpperLeftCorner : function(deltaX,deltaY) {
				this.upper_left_x += deltaX;
				this.upper_left_y += deltaY;
				var log = $('#upper_left_corner');
				log.html("Upper X: " + this.upper_left_x + ", Upper Y: " + this.upper_left_y);
				this.correctUpperLeftCornerToFitExtent();
			},
			centerUpperLeftCorner : function() {
				var brainExtentCenter = this.getDataExtent().getCenter();
				var canvasExtentCenter = this.getCenter();
				this.setUpperLeftCorner(brainExtentCenter.x - canvasExtentCenter.x, brainExtentCenter.y - canvasExtentCenter.y);
			},
			eraseCanvasContent: function(extent) {
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
			},
			correctUpperLeftCornerToFitExtent : function() {
				// canvas is bigger than data extent => allow negative upper left corner
				// we can however not move the extent bounds out of the canvas entirely
				if (this.dim_x > this.getDataExtent().x && this.upper_left_x < this.getDataExtent().x - this.dim_x) {
					this.upper_left_x = this.getDataExtent().x - this.dim_x;
				}	else  if (this.dim_x > this.getDataExtent().x && this.upper_left_x > 0) {
					this.upper_left_x = 0;
				} else if (this.dim_x < this.getDataExtent().x && this.upper_left_x < 0) {
					this.upper_left_x = 0;
				} else if (this.dim_x < this.getDataExtent().x && this.upper_left_x + this.dim_x > this.getDataExtent().x) {
					this.upper_left_x = this.getDataExtent().x -this.dim_x;
				}
				
				if (this.dim_y > this.getDataExtent().y && this.upper_left_y < this.getDataExtent().y - this.dim_y) {
					this.upper_left_y = this.getDataExtent().y - this.dim_y;
				}	else  if (this.dim_y > this.getDataExtent().y && this.upper_left_y > 0) {
					this.upper_left_y = 0;
				} else if (this.dim_y < this.getDataExtent().y && this.upper_left_y < 0) {
					this.upper_left_y = 0;
				} else if  (this.dim_y < this.getDataExtent().y && this.upper_left_y + this.dim_y > this.getDataExtent().y) {
					this.upper_left_y = this.getDataExtent().y - this.dim_y;
				}
				
				// show data extent displayed
				var log = $('#data_extent_displayed');
				log.html("Data Extent displayed (z) X : " + (this.upper_left_x < 0 ? 0 : this.upper_left_x) + " => " + (this.getDataExtent().x > this.dim_x ? this.upper_left_x + this.dim_x : this.getDataExtent().x) + 
						", Y: " + (this.upper_left_y < 0 ? 0 : this.upper_left_y) + " => " + (this.getDataExtent().y > this.dim_y ? this.upper_left_y + this.dim_y : this.getDataExtent().y));
			},
			drawMe : function() {
				var ctx = this.getCanvasContext();
	
				// start tile range
				var startTileX = Math.floor(this.upper_left_x / this.getDataExtent().tile_size);
				var startTileY = Math.floor(this.upper_left_y / this.getDataExtent().tile_size);
				var deltaStartTileXAndUpperLeftCornerX = this.upper_left_x - (startTileX * this.getDataExtent().tile_size);  
				var deltaStartTileYAndUpperLeftCornerY = this.upper_left_y - (startTileY * this.getDataExtent().tile_size); 
	
				var endTileX = (Math.floor((this.upper_left_x + this.dim_x) / this.getDataExtent().tile_size) + (((this.upper_left_x + this.dim_x) % this.getDataExtent().tile_size) != 0 ? 1 : 0)) * this.getDataExtent().tile_size;
				
				if (this.dim_x > this.getDataExtent().x) {
					(Math.floor((this.upper_left_x + this.getDataExtent().x) / this.getDataExtent().tile_size) + (((this.upper_left_x + this.getDataExtent().x) % this.getDataExtent().tile_size) != 0 ? 1 : 0)) * this.getDataExtent().tile_size;
				}
				var endTileY = 
					(Math.floor((this.upper_left_y + this.dim_y) / this.getDataExtent().tile_size) + (((this.upper_left_y + this.dim_y) % this.getDataExtent().tile_size) != 0 ? 1 : 0)) * this.getDataExtent().tile_size;
				
				if (this.dim_y > this.getDataExtent().y) {
					(Math.floor((this.upper_left_y + this.getDataExtent().y) / this.getDataExtent().tile_size) + (((this.upper_left_y + this.getDataExtent().y) % this.getDataExtent().tile_size) != 0 ? 1 : 0)) * this.getDataExtent().tile_size;
				}
				
				// remember canvas x - position we'd like to write to
				var canvasX =  0;
				var canvasY =  0;
	
				// loop over rows
				for (var tileOffsetX = startTileX * this.getDataExtent().tile_size  ; tileOffsetX < endTileX ; tileOffsetX += this.getDataExtent().tile_size) {
					// stop if we exceed extent
					if (this.dim_x > this.getDataExtent().x && tileOffsetX >= this.getDataExtent().x) {
						break;
					} else if (this.dim_x > this.getDataExtent().x && tileOffsetX < 0) {
						canvasX = Math.abs(this.upper_left_x);
						continue;
					}
					
					var canvasOffsetX = tileOffsetX + deltaStartTileXAndUpperLeftCornerX;
	
					// find cached tile by using rowIndex_colIndex naming convention
					var rowIndex = Math.floor(tileOffsetX / this.getDataExtent().tile_size);
					
					// define the width and height for the portion of the tile taken and the canvas portion to be filled 
					var width =  this.getDataExtent().tile_size;
	
					// remember the image offset
					var imageOffsetX = 0;
	
					// we have hit the left canvas bound if canvas offsetX equals upper left x
					if (canvasOffsetX == this.upper_left_x) {
						// set canvas position to 0
						canvasX = 0;
						width = this.getDataExtent().tile_size - deltaStartTileXAndUpperLeftCornerX;
						if (width < this.getDataExtent().tile_size) {
							imageOffsetX = this.getDataExtent().tile_size - width; 
						}
					} 
					
					// see if we have hit the right and bottom bounds 
					if (tileOffsetX + this.getDataExtent().tile_size == endTileX) {
						//width = endTileX - canvasOffsetX;
						width = this.dim_x -canvasX;
					}
	
					// walk through columns
					for (var tileOffsetY = startTileY *  this.getDataExtent().tile_size; tileOffsetY < endTileY ; tileOffsetY += this.getDataExtent().tile_size) {
						// stop if we exceed extent
						if (this.dim_y > this.getDataExtent().y && tileOffsetY >= this.getDataExtent().y) {
							break;
						} else if (this.dim_y > this.getDataExtent().y && tileOffsetY < 0) {
							canvasY = Math.abs(this.upper_left_y);
							continue;
						}
	
						var canvasOffsetY = tileOffsetY + deltaStartTileYAndUpperLeftCornerY;
						
						// find cached tile by using rowIndex_colIndex naming convention
						var colIndex = Math.floor(tileOffsetY / this.getDataExtent().tile_size);
						
						// define the width and height for the portion of the tile taken and the canvas portion to be filled 
						var height =  this.getDataExtent().tile_size;
	
						// remember the image offset
						var imageOffsetY = 0;
	
						// we have hit the left canvas bound if canvas offsetX equals upper left x
						if (canvasOffsetY == this.upper_left_y) {
							// set canvas position to 0
							canvasY = 0;
							height = this.getDataExtent().tile_size - deltaStartTileYAndUpperLeftCornerY;
							if (height < this.getDataExtent().tile_size) {
								imageOffsetY = this.getDataExtent().tile_size - height; 
							};
						} 
	
						// see if we have hit the right and bottom bounds 
						if (tileOffsetY + this.getDataExtent().tile_size == endTileY) {
							//height = endTileY - canvasOffsetY;
							height = this.dim_y -canvasY;
						}
						
						// create the image object that loads the tile we need
						var imageTile = new Image();
						imageTile.src = 
							brain.tile_directory + this.getDataExtent().data_id + "/" + this.getDataExtent().zoom_level + "/" + this.getDataExtent().plane
							+ "/" + this.getDataExtent().slice + "/" + rowIndex + '_' + colIndex + "." + this.image_format;
	
						(function(imageOffsetX, imageOffsetY, canvasX, canvasY, width, height, dim_x, dim_y) {
							imageTile.onload = function() {
								// check with actual image dimensions ...
								if (this.width < width) {
									width = this.width;
								}
								if (this.height < height) {
									height = this.height;
								}
	
								ctx.drawImage(this,
										imageOffsetX, imageOffsetY, width, height, // tile dimensions
										canvasX, canvasY, width, height); // canvas dimensions
							};
						})(imageOffsetX, imageOffsetY, canvasX, canvasY, width, height, this.dim_x, this.dim_y);
						
						// increment canvasY
						canvasY += height;
					}
					
					// increment canvasX
					canvasX += width;
				};
			}
		};

	}
};


function initMe() {
	// create a brain extent for each of the three planes
	// TODO: this info will come from the back-end most likely. for now hard-code!
	var brain_extent_x_plane = new brain.extent();
	brain_extent_x_plane.init("example_data", 0, 'x', Math.floor(679/2), 1311, 499);

	var brain_extent_y_plane = new brain.extent();
	brain_extent_y_plane.init("example_data", 0, 'y', Math.floor(1311/2), 679, 499);

	var brain_extent_z_plane = new brain.extent();
	brain_extent_z_plane.init("example_data", 0, 'z', Math.floor(499/2), 679, 1311);
	
	// create three instances of the brain canvas (1 for each plane)
	var brain_canvas_x_plane = new brain.canvas();
	brain_canvas_x_plane.init(brain_extent_x_plane, "canvas_x_plane");

	var brain_canvas_y_plane = new brain.canvas();
	brain_canvas_y_plane.init(brain_extent_y_plane, "canvas_y_plane");

	var brain_canvas_z_plane = new brain.canvas();
	brain_canvas_z_plane.init(brain_extent_z_plane, "canvas_z_plane");

	// show total data extent and canvas dimensions
	var log = $('#total_data_extent');
	log.html("Total Data Extent (z): " + brain_canvas_z_plane.getDataExtent().x  + " x " + brain_canvas_z_plane.getDataExtent().y);
	log = $('#canvas_dimensions');
	log.html("Canvas Dimensions: " + brain_canvas_z_plane.dim_x  + " x " + brain_canvas_z_plane.dim_y);
	
	// center and draw
	brain_canvas_x_plane.centerUpperLeftCorner();
	brain_canvas_x_plane.drawCoordinateCross(brain_canvas_x_plane.getCenter());
	brain_canvas_x_plane.drawMe();

	brain_canvas_y_plane.centerUpperLeftCorner();
	brain_canvas_y_plane.drawCoordinateCross(brain_canvas_y_plane.getCenter());
	brain_canvas_y_plane.drawMe();

	brain_canvas_z_plane.centerUpperLeftCorner();
	brain_canvas_z_plane.drawCoordinateCross(brain_canvas_y_plane.getCenter());
	brain_canvas_z_plane.drawMe();
}

$(document).ready(function() {
	initMe();
});