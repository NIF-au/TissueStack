TissueStack.Queue = function (canvas) {
	this.canvas = canvas;
};

TissueStack.Queue.prototype = {
	canvas : null,
	queue_handle : null,
	drawingIntervalInMillis : 150,
	requests : [],
	presentlyQueuedZoomLevelAndSlice: null,
	lowResolutionPreviewDrawn : false,
	latestDrawRequestTimestamp : 0,
	setDrawingInterval : function(value) {
		if (typeof(value) !== 'number' || value < 0) {
			value = 150; // set to default
		}
		this.stopQueue();
		this.drawingIntervalInMillis = value;
		this.startQueue();
	},startQueue : function() {
		if (this.queue_handle) { // already active
			return;
		}
		
		var _this = this;
		
		this.queue_handle = setInterval(function() {
			// sanity check, if we still have some requests queued
			if (_this.requests.length == 0) {
				_this.stopQueue();
				return;
			};
			
			// work with deep copy, is safer and also leave last request in there instead of popping it
			var latestRequest = $.extend(true, {}, _this.requests[_this.requests.length-1]);
			_this.clearRequestQueue();
			if (!latestRequest) {
				_this.stopQueue();
				return;
			}
			
			// double check if we are obsolete already
			if (_this.presentlyQueuedZoomLevelAndSlice !== (latestRequest.dataset_id + "_" + latestRequest.data_id + "_" + latestRequest.zoom_level + '_' + latestRequest.slice)) {
				_this.clearRequestQueue();
				_this.stopQueue();
				return;
			}

			_this.latestDrawRequestTimestamp = latestRequest.timestamp;
			//console.info('Action: ' + latestRequest.action + ' @ '  + latestRequest.timestamp + ' [' + latestRequest.data_id + ']');
			
			if (_this.prepareDrawRequest(latestRequest)) {
				setTimeout(function() {
					_this.drawLowResolutionPreview(_this.latestDrawRequestTimestamp);
					_this.drawRequestAfterLowResolutionPreview(latestRequest);}
				, 200);
			}
		}, this.drawingIntervalInMillis);
	},
	stopQueue : function() {
		if (!this.queue_handle) {
			return;
		}
		clearInterval();
		this.queue_handle = null;
	},
	addToQueue : function(draw_request) {
		// we have no existing queue for the zoom level and slice =>
		// this means: A) we have to create it AND B) we have to empty the queue to get rid of old requests
		if (this.presentlyQueuedZoomLevelAndSlice !== (draw_request.dataset_id + "_" + draw_request.data_id + "_" + draw_request.zoom_level + '_' + draw_request.slice)) {
			this.presentlyQueuedZoomLevelAndSlice = draw_request.dataset_id + "_" + draw_request.data_id + "_" + draw_request.zoom_level + '_' + draw_request.slice;
			this.clearRequestQueue();
			this.stopQueue();
		}
		
		// clicks and zooms are processed instantly
		if (draw_request.action == "CLICK" || draw_request.action == "ZOOM" || draw_request.action == "POINT") {
			var deepCopyOfRequest = $.extend(true, {}, draw_request);
			this.latestDrawRequestTimestamp = deepCopyOfRequest.timestamp;
			
			//console.info('Action: ' + deepCopyOfRequest.action + ' @ '  + deepCopyOfRequest.timestamp + ' [' + draw_request.data_id + ']');
			
			// work with a deep copy
			if (this.prepareDrawRequest(deepCopyOfRequest)) {
				if (deepCopyOfRequest.action == 'ZOOM') {
					this.canvas.eraseCanvasContent();

					var _this = this;
					setTimeout(function() {
						_this.drawLowResolutionPreview(deepCopyOfRequest.timestamp);
						_this.drawRequestAfterLowResolutionPreview(deepCopyOfRequest, deepCopyOfRequest.timestamp);}
					, 200);
				} else {
					this.drawLowResolutionPreview(deepCopyOfRequest.timestamp);
					this.drawRequestAfterLowResolutionPreview(deepCopyOfRequest, deepCopyOfRequest.timestamp);
				}
			}

			return;
		}

		// queue pans
		this.requests.push(draw_request);
		
		// process through queue
		this.startQueue();
	},
	drawRequestAfterLowResolutionPreview : function(draw_request, timestamp) {
		var _this = this;
		var lowResBackdrop = setInterval(function() {
			var t = draw_request ? draw_request.timestamp : timestamp;
			if (_this.latestDrawRequestTimestamp > 0 && t > _this.latestDrawRequestTimestamp) {
				clearInterval(lowResBackdrop);
				return;
			}
			if (_this.lowResolutionPreviewDrawn) {
				if (draw_request) {
					_this.drawRequest(draw_request);
				} else {
					_this.canvas.drawMe(timestamp);
				}
				clearInterval(lowResBackdrop);
			}
		}, 100);		
	},
	clearRequestQueue : function() {
		this.requests = [];
	}, drawLowResolutionPreview : function(timestamp) {
		if (this.latestDrawRequestTimestamp < 0 || timestamp < this.latestDrawRequestTimestamp) {
			//console.info('Drawing preview for ' + this.canvas.getDataExtent().data_id + '[' + this.canvas.getDataExtent().getOriginalPlane() +  ']: ' + timestamp);

			this.lowResolutionPreviewDrawn = true;
			return;
		}

		// this is to prevent preview fetching for the cases when the user is navigating in a view that exceeds the data extent
		// so that they can set the crosshair outside of the extent
		var slice = this.canvas.getDataExtent().slice;
		if (slice < 0 || slice > this.canvas.getDataExtent().max_slices) {
			this.lowResolutionPreviewDrawn = true;
			return;
		}

		this.lowResolutionPreviewDrawn = false;

		var ctx = this.canvas.getCanvasContext();
		
		// nothing to do if we are totally outside
		if (this.canvas.upper_left_x < 0 && (this.canvas.upper_left_x + this.canvas.getDataExtent().x) <=0
				|| this.canvas.upper_left_x > 0 && this.canvas.upper_left_x > this.canvas.dim_x
				|| this.canvas.upper_left_y <=0 || (this.canvas.upper_left_y - this.canvas.getDataExtent().y) >= this.canvas.dim_y) {
			this.lowResolutionPreviewDrawn = true;
			return;
		} 
		
		var dataSet = TissueStack.dataSetStore.getDataSetById(this.canvas.data_extent.data_id);
		if (!dataSet) {
			alert("Couldn't find data set with id: " + this.canvas.data_extent.data_id);
			return;
		}
		
		var canvasX = 0;
		var imageOffsetX = 0;
		var width = this.canvas.getDataExtent().x;
		if (this.canvas.upper_left_x < 0) {
			width += this.canvas.upper_left_x;
			imageOffsetX = this.canvas.getDataExtent().x - width;
		} else {
			canvasX = this.canvas.upper_left_x;
		}
		
		if (canvasX + width > this.canvas.dim_x) {
			width = this.canvas.dim_x - canvasX;
		}

		var canvasY = 0;
		var imageOffsetY = 0;
		var height = this.canvas.getDataExtent().y;
		if (this.canvas.upper_left_y <= this.canvas.dim_y) {
			canvasY = this.canvas.dim_y - this.canvas.upper_left_y;
		} else {
			imageOffsetY = this.canvas.upper_left_y - this.canvas.dim_y;
			height = this.canvas.getDataExtent().y - imageOffsetY;
		}
		
		if (height > this.canvas.dim_y) {
			height = this.canvas.dim_y;
		}
		
		var imageTile = new Image();
		imageTile.crossOrigin = '';
		var src =
			TissueStack.Utils.assembleTissueStackImageRequest(
					"http",
					dataSet.host,
					this.canvas.getDataExtent().getIsTiled(),
					dataSet.filename,
					dataSet.local_id,
					true,
					this.canvas.getDataExtent().getIsTiled() ?
							this.canvas.getDataExtent().zoom_level : 
								this.canvas.getDataExtent().getZoomLevelFactorForZoomLevel(this.canvas.getDataExtent().zoom_level),
					this.canvas.getDataExtent().getOriginalPlane(),
					slice,
					this.canvas.color_map,
					this.canvas.image_format
		);
		// append session id & timestamp for image service
		if (!this.canvas.getDataExtent().getIsTiled()) {
			src += ("&id=" + this.canvas.sessionId);
			src += ("&timestamp=" + timestamp);
		}
		imageTile.src = src; 
		

		(function(_this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height) {
			imageTile.onload = function() {
			
				if (_this.latestDrawRequestTimestamp < 0 || timestamp < _this.latestDrawRequestTimestamp) {
					_this.lowResolutionPreviewDrawn = true;
					//console.info('Aborting preview for ' + _this.canvas.getDataExtent().data_id + '[' +_this.canvas.getDataExtent().getOriginalPlane() +  ']: ' + timestamp);
					return;
				}

				if (this.width < width) {
					width = this.width;
				}

				if (this.height < height) {
					height = this.height;
				}

				//console.info('Drawing preview for ' +  _this.canvas.getDataExtent().data_id + '[' +_this.canvas.getDataExtent().getOriginalPlane() +  ']: ' + timestamp);
				_this.lowResolutionPreviewDrawn = true;
				ctx.drawImage(this, imageOffsetX, imageOffsetY, width, height, canvasX, canvasY, width, height);
				
				if (_this.canvas.getDataExtent().getIsTiled()) _this.canvas.applyColorMapToCanvasContent();
			};
		})(this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height);
	}, prepareDrawRequest : function(draw_request) {
		var thisHerePlane = this.canvas.getDataExtent().plane;
		var thisHereDataSet = this.canvas.dataset_id;
		
		// if more than 1 data set is displayed, we stop propagation to the other here !
		if (thisHereDataSet != draw_request.dataset_id) {
			return false;
		}

		// ZOOM CHANGE ACTION
		if (draw_request.action == 'ZOOM') { // zoom request
			this.canvas.changeToZoomLevel(draw_request.zoom_level);
			return true;
		}

		// for all the following actions do only the other planes,
		// not the plane that triggered the event, otherwise it will get "served" twice !! 
		if (draw_request.plane === thisHerePlane) { 
			return true;
		}
		
		// SLICE CHANGE ACTION
		if (draw_request.action == 'SLICE') {
			var sliceX = draw_request.slice;
			var sliceY = this.canvas.data_extent.one_to_one_y - draw_request.slice;
			
			sliceX = (this.canvas.getDataExtent().zoom_level == 1) ?
					Math.floor(sliceX * (this.canvas.getDataExtent().x / this.canvas.data_extent.one_to_one_x)) :
					Math.ceil(sliceX * (this.canvas.getDataExtent().x / this.canvas.data_extent.one_to_one_x));
			sliceY = (this.canvas.getDataExtent().zoom_level == 1) ?
					Math.floor(sliceY * (this.canvas.getDataExtent().y / this.canvas.data_extent.one_to_one_y)) :
					Math.ceil(sliceY * (this.canvas.getDataExtent().y / this.canvas.data_extent.one_to_one_y));

			var relativeCrossCoords = this.canvas.getRelativeCrossCoordinates();
			// correct for negative 1 which indicates end 
			if (relativeCrossCoords.x < 0) {
				relativeCrossCoords.x = sliceX + 1;
			}
			var deltaX = relativeCrossCoords.x - sliceX;
			var deltaY = relativeCrossCoords.y - sliceY;
			
			if (thisHerePlane === 'x' && draw_request.plane === 'z') {
				this.canvas.moveUpperLeftCorner(0, -deltaY);
			} else if (thisHerePlane === 'y' && draw_request.plane === 'z') {
				this.canvas.moveUpperLeftCorner(0, -deltaY);
			} else if (thisHerePlane === 'x' && draw_request.plane === 'y') {
				this.canvas.moveUpperLeftCorner(deltaX, 0);
			} else if (thisHerePlane === 'z' && draw_request.plane === 'y') {
				this.canvas.moveUpperLeftCorner(0, -deltaY);
			} else if (thisHerePlane === 'y' && draw_request.plane === 'x') {
				this.canvas.moveUpperLeftCorner(deltaX, 0);
			} else if (thisHerePlane === 'z' && draw_request.plane === 'x') {
				this.canvas.moveUpperLeftCorner(deltaX, 0);
			}
			return true;
		}
		
		// POINT FOCUS ACTION
		if (draw_request.action == 'POINT') {
			this.canvas.drawCoordinateCross(this.canvas.getCenter());
		}

		// CLICK ACTION 
		if (thisHerePlane === 'x' && draw_request.plane === 'z' && draw_request.action == 'CLICK') {
			this.canvas.drawCoordinateCross(
					{x: draw_request.canvasDims.y - (draw_request.crossCoords.y + ((draw_request.canvasDims.y - draw_request.crossCoords.y)- this.canvas.cross_x)),
					 y:  this.canvas.cross_y});
		} else if (thisHerePlane === 'y' && draw_request.plane === 'z' && draw_request.action == 'CLICK') {
			this.canvas.drawCoordinateCross(
					{x: draw_request.crossCoords.x + (this.canvas.cross_x - draw_request.crossCoords.x),
					 y: this.canvas.cross_y});
		} else if (thisHerePlane === 'x' && draw_request.plane === 'y' && draw_request.action == 'CLICK') {
			this.canvas.drawCoordinateCross(
					{x: this.canvas.cross_x,
					 y: (draw_request.canvasDims.y - draw_request.crossCoords.y) + ((this.canvas.dim_y - this.canvas.cross_y) - (draw_request.canvasDims.y - draw_request.crossCoords.y))});
		} else if (thisHerePlane === 'z' && draw_request.plane === 'y' && draw_request.action == 'CLICK') {
			this.canvas.drawCoordinateCross(
					{x: draw_request.crossCoords.x + (this.canvas.cross_x - draw_request.crossCoords.x),
					 y:  this.canvas.cross_y});
		} else if (thisHerePlane === 'y' && draw_request.plane === 'x' && draw_request.action == 'CLICK') {
			this.canvas.drawCoordinateCross(
					{x:  this.canvas.cross_x,
						y: (draw_request.canvasDims.y - draw_request.crossCoords.y) + ((this.canvas.dim_y - this.canvas.cross_y) - (draw_request.canvasDims.y - draw_request.crossCoords.y))});
		} else if (thisHerePlane === 'z' && draw_request.plane === 'x' && draw_request.action == 'CLICK') {
			this.canvas.drawCoordinateCross(
					{x: this.canvas.cross_x,
					 y: this.canvas.dim_y - (draw_request.crossCoords.x + ((this.canvas.dim_y - this.canvas.cross_y) - draw_request.crossCoords.x))});
		}
		
		// COORDINATE CHANGES DUE TO VARYING ZOOM LEVELS BETWEEN THE CANVASES 
		var originalZoomLevelDims = this.canvas.getDataExtent().getZoomLevelDimensions(draw_request.zoom_level);
		var crossXOutsideOfExtentX = (draw_request.coords.x < 0) ? -1 : 0;
		if (draw_request.coords.x > (draw_request.max_coords_of_event_triggering_plane.max_x - 1)) {
			crossXOutsideOfExtentX = 1;
		}
		var crossYOutsideOfExtentY = (draw_request.coords.y < 0) ? -1 : 0;
		if (draw_request.coords.y > (draw_request.max_coords_of_event_triggering_plane.max_y - 1)) {
			crossYOutsideOfExtentY = 1;
		}
		
		if (draw_request.zoom_level != this.canvas.getDataExtent().zoom_level) {
			if (draw_request.coords.x < 0) {
				draw_request.coords.x = Math.abs(draw_request.coords.x - draw_request.max_coords_of_event_triggering_plane.max_x);
				draw_request.upperLeftCorner.x = draw_request.crossCoords.x - draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);				
			} else if (draw_request.coords.x > (draw_request.max_coords_of_event_triggering_plane.max_x - 1)) {
				draw_request.coords.x = draw_request.coords.x - (draw_request.max_coords_of_event_triggering_plane.max_x - 1);
				draw_request.upperLeftCorner.x = draw_request.crossCoords.x + draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);
			} else {
				draw_request.upperLeftCorner.x = draw_request.crossCoords.x - draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);				
			}

			if (draw_request.coords.y < 0) {
				draw_request.coords.y = Math.abs(draw_request.coords.y);
				draw_request.upperLeftCorner.y = (draw_request.canvasDims.y - draw_request.crossCoords.y) - draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);				
			} else if (draw_request.coords.y > (draw_request.max_coords_of_event_triggering_plane.max_y - 1)) {
				draw_request.upperLeftCorner.y = (draw_request.canvasDims.y - draw_request.crossCoords.y) + draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);
			} else {
				draw_request.upperLeftCorner.y = (draw_request.canvasDims.y - draw_request.crossCoords.y) + draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);				
			}

			draw_request.upperLeftCorner.x = (draw_request.zoom_level == 1) ? Math.floor(draw_request.upperLeftCorner.x) : Math.ceil(draw_request.upperLeftCorner.x);
			draw_request.upperLeftCorner.y = (draw_request.zoom_level == 1) ? Math.floor(draw_request.upperLeftCorner.y) : Math.ceil(draw_request.upperLeftCorner.y);

			draw_request.coords.x = (draw_request.zoom_level == 1) ?
					Math.floor(draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x)) :
					Math.ceil(draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x));
			draw_request.coords.y = (draw_request.zoom_level == 1) ?
					Math.floor(draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y)) :
					Math.ceil(draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y));

			draw_request.max_coords_of_event_triggering_plane.max_x = (draw_request.zoom_level == 1) ?
					Math.floor(draw_request.max_coords_of_event_triggering_plane.max_x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x)) :
					Math.ceil(draw_request.max_coords_of_event_triggering_plane.max_x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x));
			draw_request.max_coords_of_event_triggering_plane.max_y = (draw_request.zoom_level == 1) ?
					Math.floor(draw_request.max_coords_of_event_triggering_plane.max_y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y)) :
					Math.ceil(draw_request.max_coords_of_event_triggering_plane.max_y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y));
		} 
		
		// PAN AND CLICK ACTION
		if (thisHerePlane === 'x' && draw_request.plane === 'z') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(
					(crossXOutsideOfExtentX < 0) ? -99 : ((crossXOutsideOfExtentX > 0) ? (draw_request.max_coords_of_event_triggering_plane.max_x + 99) : draw_request.coords.x));
			this.canvas.setUpperLeftCorner(
					(draw_request.upperLeftCorner.y - (draw_request.max_coords_of_event_triggering_plane.max_y - 1))
					+ (this.canvas.cross_x - (draw_request.canvasDims.y - draw_request.crossCoords.y)),
					this.canvas.upper_left_y);
		} else if (thisHerePlane === 'y' && draw_request.plane === 'z') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(
					(crossYOutsideOfExtentY < 0) ?
							-99 :
							((crossYOutsideOfExtentY > 0) ? (draw_request.max_coords_of_event_triggering_plane.max_y + 99) :
								((draw_request.max_coords_of_event_triggering_plane.max_y - 1) - draw_request.coords.y)));
			this.canvas.setUpperLeftCorner(
					draw_request.upperLeftCorner.x + (this.canvas.cross_x - draw_request.crossCoords.x),
					this.canvas.upper_left_y);
		} else if (thisHerePlane === 'x' && draw_request.plane === 'y') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(
					(crossXOutsideOfExtentX < 0) ? -99 : ((crossXOutsideOfExtentX > 0) ? (draw_request.max_coords_of_event_triggering_plane.max_x + 99) : draw_request.coords.x));
			this.canvas.setUpperLeftCorner(
					this.canvas.upper_left_x,
					draw_request.upperLeftCorner.y + ((this.canvas.dim_y - this.canvas.cross_y) - (draw_request.canvasDims.y - draw_request.crossCoords.y)));
		} else if (thisHerePlane === 'z' && draw_request.plane === 'y') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(
					(crossYOutsideOfExtentY < 0) ?
							-99 :
							((crossYOutsideOfExtentY > 0) ? (draw_request.max_coords_of_event_triggering_plane.max_y + 99) :
								((draw_request.max_coords_of_event_triggering_plane.max_y - 1) - draw_request.coords.y)));
			this.canvas.setUpperLeftCorner(
					draw_request.upperLeftCorner.x + (this.canvas.cross_x - draw_request.crossCoords.x),
					this.canvas.upper_left_y);
		} else if (thisHerePlane === 'y' && draw_request.plane === 'x') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(
					(crossXOutsideOfExtentX < 0) ? -99 : ((crossXOutsideOfExtentX > 0) ? (draw_request.max_coords_of_event_triggering_plane.max_x + 99) : draw_request.coords.x));
			this.canvas.setUpperLeftCorner(
					this.canvas.upper_left_x ,
					draw_request.upperLeftCorner.y + ((this.canvas.dim_y - this.canvas.cross_y) - (draw_request.canvasDims.y - draw_request.crossCoords.y)));
		} else if (thisHerePlane === 'z' && draw_request.plane === 'x') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(((draw_request.max_coords_of_event_triggering_plane.max_y - 1) - draw_request.coords.y));
			this.canvas.setUpperLeftCorner(
					this.canvas.upper_left_x ,
					((draw_request.max_coords_of_event_triggering_plane.max_x - 1) + draw_request.upperLeftCorner.x + ((this.canvas.dim_y - this.canvas.cross_y) - draw_request.crossCoords.x)));
		}
		
		return true;
	}, drawRequest : function(draw_request) {
		// redraw 
		this.canvas.drawMe(draw_request.timestamp);

		if (this.canvas.getDataExtent().slice < 0 || this.canvas.getDataExtent().slice > this.canvas.getDataExtent().max_slices 
				|| this.canvas.upper_left_x > this.canvas.dim_x || this.canvas.upper_left_x + this.canvas.data_extent.x < 0
				|| this.canvas.upper_left_y < 0 || this.canvas.upper_left_y - this.canvas.data_extent.y > this.canvas.dim_y) {
			this.canvas.eraseCanvasContent(); // in these cases we erase the entire content
			return;
		}
		
		// tidy up where we left debris
		if (this.canvas.upper_left_x > 0) { // in front of us
			this.canvas.eraseCanvasPortion(0, 0, this.canvas.upper_left_x, this.canvas.dim_y);
		}
		if (this.canvas.upper_left_x <= 0 || (this.canvas.upper_left_x >= 0 && (this.canvas.upper_left_x + this.canvas.getDataExtent().x) < this.canvas.dim_x)){ // behind us
			this.canvas.eraseCanvasPortion(
					this.canvas.upper_left_x + this.canvas.getDataExtent().x, 0,
					this.canvas.dim_x - (this.canvas.upper_left_x + this.canvas.getDataExtent().x), this.canvas.dim_y);
		}
		
		if (this.canvas.upper_left_y < 0 || (this.canvas.upper_left_y < this.canvas.dim_y && this.canvas.upper_left_y >= 0)) { // in front of us
			this.canvas.eraseCanvasPortion(0, 0, this.canvas.dim_x, (this.canvas.upper_left_y <= 0) ? this.canvas.dim_y : (this.canvas.dim_y - this.canvas.upper_left_y));
		}
		if ((this.canvas.upper_left_y - this.canvas.getDataExtent().y) >= this.canvas.dim_y || (this.canvas.upper_left_y - this.canvas.getDataExtent().y) > 0) { // behind us
			this.canvas.eraseCanvasPortion(
				0, (this.canvas.upper_left_y >= this.canvas.dim_y && this.canvas.upper_left_y - this.canvas.getDataExtent().y >= this.canvas.dim_y) ? 0 : (this.canvas.dim_y - (this.canvas.upper_left_y - this.canvas.getDataExtent().y)),
				this.canvas.dim_x, 
				(this.canvas.upper_left_y >= this.canvas.dim_y && this.canvas.upper_left_y - this.canvas.getDataExtent().y >= this.canvas.dim_y) ?
						this.canvas.dim_y : (this.canvas.dim_y - (this.canvas.upper_left_y - this.canvas.getDataExtent().y)));
		}
	}
};
