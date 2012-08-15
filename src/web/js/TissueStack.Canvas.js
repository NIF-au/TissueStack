TissueStack.Canvas = function(data_extent, canvas_id, dataset_id, include_cross_hair) {
	// assemble data set id
	this.dataset_id = typeof(dataset_id) != "string" ? "" : dataset_id;
	this.setDataExtent(data_extent);
	this.setCanvasElement(this.dataset_id == "" ? canvas_id : (this.dataset_id + "_" + canvas_id));
	// set dimensions
	var tmpCanvasElement = this.getCanvasElement()[0];
	this.setDimensions(tmpCanvasElement.width, tmpCanvasElement.height);
	this.centerUpperLeftCorner();
	this.drawCoordinateCross(this.getCenter());
	this.setIncludeCrossHair(include_cross_hair);
	this.events = new TissueStack.Events(this, this.include_cross_hair); 
	this.queue = new TissueStack.Queue(this);
	// make parent and ourselves visible
	this.getCanvasElement().parent().removeClass("hidden");
	
};

TissueStack.Canvas.prototype = {
	is_main_view: false,
	data_extent: null,
	dataset_id: "",
	canvas_id: this.dataset_id + "canvas_" + this.plane,
	include_cross_hair: true,
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
	color_map : "grey",
	updateScaleBar : function() {
		// update scale bar if main view
		if (this.is_main_view) this.getDataExtent().adjustScaleBar(100);
	}, setIncludeCrossHair : function(include_cross_hair) {
		// include cross hair canvas or not
		if (typeof(include_cross_hair) != 'boolean' || include_cross_hair == true) {
			this.include_cross_hair = true;
		} else {
			this.include_cross_hair = false;
		}
	},
	setDataExtent : function (data_extent) {
		if (typeof(data_extent) != "object") {
			throw new Error("we miss a data_extent");
		}
		this.data_extent = data_extent;
		// store reference back
		this.data_extent.canvas = this;
	},
	setCanvasElement : function(canvas_id) {
		if (canvas_id && (typeof(canvas_id) != "string" || canvas_id.length == 0)) {
			throw new Error("canvas_id has to be a non-empty string");
		}
		this.canvas_id = canvas_id;
		if (!$("#" + this.canvas_id)) {
			throw new Error("Canvas element with id " + this.canvas_id + " does not exist!");
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

		var centerAfterZoom = this.getNewUpperLeftCornerForPointZoom({x: this.cross_x, y: this.cross_y}, zoom_level);

		this.getDataExtent().changeToZoomLevel(zoom_level);

		this.setUpperLeftCorner(centerAfterZoom.x, centerAfterZoom.y);
		
		// update displayed info
		this.updateExtentInfo(this.getDataExtent().getExtentCoordinates());
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
		if (typeof(x) == "string") {
			x = parseInt(x);
		}
		if (typeof(x) != "number" || Math.floor(x) < 0) {
			throw new Error("x has to be a non-negative integer");
		}
		this.dim_x = x;
		if (typeof(y) == "string") {
			y = parseInt(y);
		}
		if (typeof(y) != "number" || Math.floor(y) < 0) {
			throw new Error("y has to be a non-negative integer");
		}
		this.dim_y = y;
	},
	resizeCanvas : function() {
		var tmpCanvasElement = this.getCanvasElement()[0];
		this.setDimensions(tmpCanvasElement.width, tmpCanvasElement.height);
		this.centerUpperLeftCorner();
		this.drawCoordinateCross(this.getCenter());
		this.drawMe(new Date().getTime());
	},
	getCenter : function () {
		return TissueStack.Utils.getCenter(this.dim_x,this.dim_y);
	},
	getCoordinateCrossCanvas : function() {
		return $("#" + this.canvas_id + "_cross_overlay");
	},
	getRelativeCrossCoordinates : function() {
		var relCrossX = (this.cross_x > (this.upper_left_x + (this.getDataExtent().x - 1))) ? -(this.cross_x - (this.upper_left_x + (this.getDataExtent().x - 1))) : (this.getDataExtent().x -1) +  (this.upper_left_x - this.cross_x);
		var relCrossY = ((this.dim_y - this.cross_y) > this.upper_left_y) ? (this.upper_left_y - (this.dim_y - this.cross_y)) : ((this.getDataExtent().y - 1) + (this.upper_left_y - (this.getDataExtent().y - 1) - (this.dim_y - this.cross_y)));
		if (this.upper_left_x < 0 && this.cross_x <= (this.upper_left_x + (this.getDataExtent().x - 1))) {
			relCrossX = Math.abs(this.upper_left_x) + this.cross_x;
		} else if (this.upper_left_x >= 0 && this.cross_x >= this.upper_left_x && this.cross_x <= this.upper_left_x + (this.getDataExtent().x -1)) {
			relCrossX = this.cross_x - this.upper_left_x;
		}
		if (this.upper_left_y > 0 && this.upper_left_y - (this.getDataExtent().y-1) < this.dim_y && this.dim_y - this.cross_y <= this.upper_left_y && this.dim_y - this.cross_y >= this.upper_left_y - (this.getDataExtent().y -1)) {
			relCrossY = this.upper_left_y - (this.dim_y - this.cross_y);
		}
		
		return {x: relCrossX, y: relCrossY};
	},drawCoordinateCross : function(coords) {
		// store cross coords
		this.cross_x = coords.x;
		this.cross_y = coords.y;

		var coordinateCrossCanvas = this.getCoordinateCrossCanvas();
		if (!coordinateCrossCanvas || !coordinateCrossCanvas[0]) {
			return;
		}
		
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
	getNewUpperLeftCornerForPointZoom : function(coords, zoom_level) {
		var newZoomLevelDims = this.getDataExtent().getZoomLevelDimensions(zoom_level);
		
		var deltaXBetweenCrossAndUpperLeftCorner = (this.upper_left_x < 0) ? (coords.x - this.upper_left_x) : Math.abs(coords.x - this.upper_left_x); 
		var deltaYBetweenCrossAndUpperLeftCorner = (this.upper_left_y < 0) ? ((this.dim_y - coords.y) - this.upper_left_y) : Math.abs((this.dim_y - coords.y) - this.upper_left_y);
		
		var zoomLevelCorrectionX = deltaXBetweenCrossAndUpperLeftCorner * (newZoomLevelDims.x / this.data_extent.x);
		var zoomLevelCorrectionY = deltaYBetweenCrossAndUpperLeftCorner * (newZoomLevelDims.y / this.data_extent.y);
		
		var newX = Math.floor((this.upper_left_x <= coords.x) ? (coords.x - zoomLevelCorrectionX) : (coords.x + zoomLevelCorrectionX));
		var newY = Math.floor((this.upper_left_y <= (this.dim_y - coords.y)) ? ((this.dim_y - coords.y) - zoomLevelCorrectionY) : ((this.dim_y - coords.y) + zoomLevelCorrectionY));

		return {x: newX, y: newY};
	},
	redrawWithCenterAndCrossAtGivenPixelCoordinates: function(coords, timestamp) {
		// this stops any still running draw requests 
		var now = typeof(timestamp) == 'number' ? timestamp : new Date().getTime(); 
		this.eraseCanvasContent();
		
		// make sure crosshair is centered:
		this.drawCoordinateCross(this.getCenter());
		
		this.setUpperLeftCorner(
				Math.floor(this.dim_x / 2) - coords.x,
				Math.floor(this.dim_y / 2) + coords.y);
		
		if (coords.z) {
			this.data_extent.slice = coords.z;
		}

		// look for the cross overlay which will be the top layer
		var canvas = this.getCoordinateCrossCanvas();
		if (!canvas || !canvas[0]) {
			canvas = this.getCanvasElement();
		}
		
		// send message out to others that they need to redraw as well
		canvas.trigger("sync", [this.data_extent.data_id,
		                        this.dataset_id,
		                        now,
								'POINT',
		                        this.getDataExtent().plane,
		                        this.getDataExtent().zoom_level,
		                        this.getDataExtent().slice,
		                        this.getRelativeCrossCoordinates(),
		                        {max_x: this.getDataExtent().x, max_y: this.getDataExtent().y},
								{x: this.upper_left_x, y: this.upper_left_y},
								{x: this.cross_x, y: this.cross_y},
								{x: this.dim_x, y: this.dim_y}
		                       ]);
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
		if (x<0 || y<0 || x>this.dim_x || y>this.dim_y || w <=0 || h<=0 || w>this.dim_x || h>this.dim_y) {
			return;
		}
		
    	var ctx = this.getCanvasContext();
    	var myImageData = ctx.getImageData(x, y, w, h);
    	for ( var i = 0; i < w * h * 4; i += 4) {
    		myImageData.data[i] = myImageData.data[i + 1] = myImageData.data[i + 2] = 255;
    		myImageData.data[i + 3] = 0;
    	}
    	ctx.putImageData(myImageData, x, y);
	}, applyColorMapToCanvasContent: function() {
		if (!this.color_map || this.color_map == "grey") {
			return;
		}

    	if (this.upper_left_x > this.dim_x || this.upper_left_x + this.data_extent.x < 0 || this.upper_left_y < 0 || this.upper_left_y - this.data_extent.y > this.dim_y) {
    		return;
    	}
    		
    	var ctx = this.getCanvasContext();
    	var xStart = this.upper_left_x < 0 ? 0 : this.upper_left_x;
    	var yStart = this.upper_left_y > this.dim_y ? 0 : this.dim_y - this.upper_left_y;
    	var width = xStart + this.data_extent.x;
    	var height = (this.dim_y - yStart - this.data_extent.y) > 0 ? this.data_extent.y : this.dim_y - yStart;
    	if (width > this.dim_x) {
    		width = this.dim_x - xStart;
    	}
    	
    	var myImageData = ctx.getImageData(xStart, yStart, width, height);
    	
    	for ( var x = 0; x < myImageData.data.length; x += 4) {
    		var val = myImageData.data[x];

			// set new red value
			myImageData.data[x] = TissueStack.indexed_color_maps[this.color_map][val][0];
			// set new green value
			myImageData.data[x + 1] = TissueStack.indexed_color_maps[this.color_map][val][1];			
			// set new blue value
			myImageData.data[x + 2] = TissueStack.indexed_color_maps[this.color_map][val][2];
    	}
    	
    	// put altered data back into canvas
    	ctx.putImageData(myImageData, xStart, yStart);  	
	}, drawMe : function(timestamp) {
		// damn you async loads
		if (this.queue.latestDrawRequestTimestamp < 0 ||
				(timestamp && timestamp < this.queue.latestDrawRequestTimestamp)) {
			//console.info('Beginning abort for ' + this.getDataExtent().data_id + '[' + this.getDataExtent().getOriginalPlane() + ']: ' + timestamp);
			return;
		}
		
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

		var dataSet = TissueStack.dataSetStore.getDataSetById(this.data_extent.data_id);
		if (!dataSet) {
			alert("Couldn't find data set with id: " + this.data_extent.data_id);
			return;
		}
		
		var counter = 0;
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
			// prelim check
			if (this.data_extent.slice < 0 || this.data_extent.slice >= this.data_extent.x) {
				//break;
			}
			
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
				imageTile.crossOrigin = '';
				
				var src = 
					TissueStack.Utils.assembleTissueStackImageRequest(
							"http",
							dataSet.host,
							this.getDataExtent().getIsTiled(),
							dataSet.filename,
							dataSet.local_id,
							false,
							this.getDataExtent().getIsTiled() ?
									this.getDataExtent().zoom_level : 
										this.getDataExtent().getZoomLevelFactorForZoomLevel(this.getDataExtent().zoom_level),
							this.getDataExtent().getOriginalPlane(),
							slice,
							this.color_map,
							this.image_format,
							this.getDataExtent().tile_size,
							rowIndex,
							colIndex
					);
				// append session id & timestamp for image service
				if (!this.getDataExtent().getIsTiled()) {
					src += ("&id=" + this.sessionId);
					src += ("&timestamp=" + timestamp);
				}

				// damn you async loads
				if (this.queue.latestDrawRequestTimestamp < 0 ||
						(timestamp && timestamp < this.queue.latestDrawRequestTimestamp)) {
					//console.info('Abort for ' + this.getDataExtent().data_id + '[' + this.getDataExtent().getOriginalPlane() + ']: R: ' + rowIndex + ' C: ' + colIndex  + ' t: ' + timestamp + ' qt: ' + this.queue.latestDrawRequestTimestamp);	
					return;
				}

				counter++;
				imageTile.src = src; 
				
				(function(_this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height, deltaStartTileXAndUpperLeftCornerX, deltaStartTileYAndUpperLeftCornerY, tile_size, row, col) {
					imageTile.onload = function() {
						// check with actual image dimensions ...
						if (canvasX == 0 && width != tile_size && deltaStartTileXAndUpperLeftCornerX !=0) {
							imageOffsetX = (tile_size - deltaStartTileXAndUpperLeftCornerX);
							width = this.width - imageOffsetX;
						} else if (this.width < width) {
								width = this.width;
						}

						if (canvasY == 0 && height != tile_size && deltaStartTileYAndUpperLeftCornerY !=0) {
							imageOffsetY = (tile_size - deltaStartTileYAndUpperLeftCornerY);
							height = this.height - imageOffsetY;
						} else	if (this.height < height) {
								height = this.height;
						}

						counter--;

						// damn you async loads
						if (_this.queue.latestDrawRequestTimestamp < 0 ||
								(timestamp && timestamp < _this.queue.latestDrawRequestTimestamp)) {
							//console.info('Abort for ' + _this.getDataExtent().data_id + '[' + _this.getDataExtent().getOriginalPlane() + ']: R: ' + row + ' C: ' + col + ' t: ' + timestamp + ' qt: ' + _this.queue.latestDrawRequestTimestamp);
							return;
						}
						
						//console.info('Drawing [' + _this.getDataExtent().data_id + ']: ' + timestamp + ' (' + _this.getDataExtent().getOriginalPlane()  + ') R => ' + row + ' C => ' + col + ' Left: ' + counter);
						ctx.drawImage(this,
								imageOffsetX, imageOffsetY, width, height, // tile dimensions
								canvasX, canvasY, width, height); // canvas dimensions
						
						if (counter == 0 && _this.getDataExtent().getIsTiled()) _this.applyColorMapToCanvasContent();
					};
				})(this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height, deltaStartTileXAndUpperLeftCornerX, deltaStartTileYAndUpperLeftCornerY, this.getDataExtent().tile_size, rowIndex, colIndex);
				
				// increment canvasY
				canvasY += height;
			}
			
			// increment canvasX
			canvasX += width;
		};
	},
	updateExtentInfo : function(realWorldCoords) {
		var log = (TissueStack.desktop || TissueStack.tablet) ? $('#canvas_extent') : $('#canvas_' + this.getDataExtent().plane + '_extent');
		if(TissueStack.phone){
			log.html("Zoom Level: " + this.getDataExtent().zoom_level);

		} else {
			var text = "Zoom Level: " + this.getDataExtent().zoom_level
					+ "<br/><hr />X: " + Math.round(realWorldCoords.min_x *1000) / 1000 + " to " + Math.round(realWorldCoords.max_x *1000) / 1000 + "<br/>Y: "
					+ Math.round(realWorldCoords.min_y *1000) / 1000 + " to " + Math.round(realWorldCoords.max_y *1000) / 1000 + "<br/>";
			if (this.data_extent.max_slices > 1) {
				text += ("Z: "+ Math.round(realWorldCoords.min_z *1000) / 1000 + " to " + Math.round(realWorldCoords.max_z *1000) / 1000 + "<br />");
			}
			log.html(text + "<hr />");
		}
	},
	updateCoordinateInfo : function(mouseCoords, pixelCoords, worldCoords) {
		var log;
		
		pixelCoords = this.getXYCoordinatesWithRespectToZoomLevel(pixelCoords);
		// outside of extent check
		if (!pixelCoords || pixelCoords.x < 0 || pixelCoords.x > this.data_extent.x -1 ||  pixelCoords.y < 0 || pixelCoords.y > this.data_extent.y -1) {
			log = $("#canvas_point_x").val("");
			log = $("#canvas_point_y").val("");
			log = $("#canvas_point_z").val("");
			
			return;
		}
			
		// for desktop and tablet
		if(TissueStack.desktop || TissueStack.tablet ){
			var x = worldCoords ? worldCoords.x : pixelCoords.x;
			var y = worldCoords ? worldCoords.y : pixelCoords.y;
			var z = worldCoords ? worldCoords.z : pixelCoords.z;

			log = $("#canvas_point_x").val(Math.round(x *1000) / 1000);
			log = $("#canvas_point_y").val(Math.round(y *1000) / 1000);
			if (this.data_extent.max_slices > 1) {
				log = $("#canvas_point_z").val(Math.round(z *1000) / 1000);
			} else {
				log = $("#canvas_point_z").val("");
			}
			
			var dataSet = TissueStack.dataSetStore.getDataSetById(this.getDataExtent().data_id);
			
			this.updateExtentInfo(dataSet.realWorldCoords[this.data_extent.plane]);
			
			return;
		}
		
		// for everything else...
		if (mouseCoords) {
			log = $('.coords');
			log.html("Canvas > X: " + mouseCoords.x + ", Y: " + mouseCoords.y);
		}
		log = $('.pixel_coords');
		log.html("Pixels > X: " + pixelCoords.x + ", Y: " + pixelCoords.y);
		if (worldCoords) {
			log = $('.world_coords');
			log.html("World > X: " +  Math.round(worldCoords.x * 1000) / 1000 + ", Y: " +  Math.round(worldCoords.y * 1000) / 1000);
		}
	}, getXYCoordinatesWithRespectToZoomLevel : function(coords) {
		if (this.upper_left_y < this.dim_y - this.cross_y || this.upper_left_y - (this.data_extent.y - 1) > this.dim_y - this.cross_y) {
			return;
		}

		if (this.cross_x < this.upper_left_x 
				|| this.cross_x > (this.upper_left_x + (this.getDataExtent().x - 1))) {
			return;
		}

		return this.data_extent.getXYCoordinatesWithRespectToZoomLevel(coords);
	}
};
