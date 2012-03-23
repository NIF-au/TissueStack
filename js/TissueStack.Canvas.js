TissueStack.Canvas = function () {
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
				return TissueStack.Utils.getCenter(this.dim_x,this.dim_y);
			},
			getCoordinateCrossCanvas : function() {
				return $("#" + this.canvas_id + "_cross_overlay");
			},
			getRelativeCrossCoordinates : function() {
				return {x: (this.upper_left_x + this.cross_x), y: (this.upper_left_y + this.cross_y)};
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
					_this.mouse_x = 0;
					_this.mouse_y = 0;
				});

				// create the delay drawing if more requests come in in short intervals (to use for setTimeout)
				var delayedDrawMe = function(notifyOthers, dX, dY) {
					var now =new Date().getTime(); 
					if (now > _this.most_recent_draw_request && now-_this.most_recent_draw_request < 0) {
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
					
					if (notifyOthers) {
						// send message out to others that they need to redraw as well
						_this.getCanvasElement().trigger("sync", [_this.getDataExtent().plane,
						                        _this.getRelativeCrossCoordinates(),
						                        {max_x: _this.getDataExtent().x, max_y: _this.getDataExtent().y},
						                        true,
						                        {x: dX, y: dY}]);
					}
				};
				
				// bind the mouse move event
				canvas.bind("mousemove", function(e) {
					var coords = TissueStack.Utils.getRelativeMouseCoords(e);
					
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
						
						setTimeout(delayedDrawMe, 0, true, dX, dY);

					} else {
						_this.isDragging = false;
					}
				});

				// optionally, if the overlay canvas is defined, we register the mouse handler for click, to draw the cross
				var coordinateCrossCanvas = this.getCoordinateCrossCanvas();
				if (coordinateCrossCanvas && coordinateCrossCanvas[0]) {
					canvas.bind("click", function(e) {
						var coords = TissueStack.Utils.getRelativeMouseCoords(e);

						if (_this.isDragging) {
							return;
						}


						var dX = _this.cross_x - coords.x;
						var dY = _this.cross_y - coords.y;
						
						// send message out to others that they need to redraw as well
						canvas.trigger("sync", [_this.getDataExtent().plane,
						                        _this.getDataCoordinates(coords),
						                        {max_x: _this.getDataExtent().x, max_y: _this.getDataExtent().y},
												false,
												{x: dX, y: dY}
						                       ]);

						_this.drawCoordinateCross(coords);
					});

					$(document).bind("sync", function(e, plane, coords, max_coords_of_event_triggering_plane,  move, deltas) {
						// ignore one's own events
						var thisHerePlane = _this.getDataExtent().plane;
						if (thisHerePlane === plane) {
							return;
						}
						
						if (thisHerePlane === 'x' && plane === 'z') {
							_this.getDataExtent().slice = coords.x;
							if (move && deltas.y != 0) {
								_this.moveUpperLeftCorner(-deltas.y , 0);
							} else if (!move) {
								_this.drawCoordinateCross({x: _this.cross_x + deltas.y, y:  _this.cross_y});
							}
						} else if (thisHerePlane === 'y' && plane === 'z') {
							_this.getDataExtent().slice = max_coords_of_event_triggering_plane.max_y - coords.y;
							if (move && deltas.x != 0) {
								_this.moveUpperLeftCorner(deltas.x , 0);
							} else if (!move) {
								_this.drawCoordinateCross({x: _this.cross_x - deltas.x, y: _this.cross_y});
							}
						} else if (thisHerePlane === 'x' && plane === 'y') {
							_this.getDataExtent().slice = coords.x;
							if (move && deltas.y != 0) {
								_this.moveUpperLeftCorner(0 , deltas.y);
							} else if (!move) {
								_this.drawCoordinateCross({x: _this.cross_x, y:   _this.cross_y - deltas.y});
							}
						} else if (thisHerePlane === 'z' && plane === 'y') {
							_this.getDataExtent().slice = max_coords_of_event_triggering_plane.max_y - coords.y;
							if (move && deltas.x != 0) {
								_this.moveUpperLeftCorner(deltas.x , 0);
							} else if (!move) {
								_this.drawCoordinateCross({x:  _this.cross_x - deltas.x, y:  _this.cross_y});
							}
						} else if (thisHerePlane === 'y' && plane === 'x') {
							_this.getDataExtent().slice = coords.x;
							if (move && deltas.y != 0) {
								_this.moveUpperLeftCorner(0 , deltas.y);
							} else if (!move) {
										_this.drawCoordinateCross({x:   _this.cross_x , y: _this.cross_y - deltas.y});
									}
						} else if (thisHerePlane === 'z' && plane === 'x') {
							_this.getDataExtent().slice = max_coords_of_event_triggering_plane.max_y - coords.y;
							if (move && deltas.x != 0) {
								_this.moveUpperLeftCorner(0 , -deltas.x);
							} else if (!move) {
								_this.drawCoordinateCross({x: _this.cross_x, y: _this.cross_y + deltas.x});
							}
						}
						
						// redraw 
						setTimeout(delayedDrawMe, 0, false);
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
							TissueStack.tile_directory + this.getDataExtent().data_id + "/" + this.getDataExtent().zoom_level + "/" + this.getDataExtent().plane
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
	};
