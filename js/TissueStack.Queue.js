TissueStack.Queue = function (canvas) {
	return {
		canvas : canvas,
		queue_handle : null,
		drawingIntervalInMillis : 15,
		accumulatedRequests : null,
		requests : [],
		presentlyQueuedZoomLevelAndSlice: null,
		lowResolutionPreviewDrawn : false,
		latestDrawRequestTimestamp : 0,
		setDrawingInterval : function(value) {
			if (typeof(value) !== 'number' || value < 0) {
				throw "Interval has to be greater or equal to 0";
			}
			this.stopQueue();
			this.drawingIntervalInMillis = value;
			this.startQueue();
		},startQueue : function() {
			this.queue_handle = setInterval(function(_this) {
				// sanity check, if we still have some requests queued
				var latestRequest =_this.requests.pop();
				if (!latestRequest) {
					return;
				}
				
				// double check if we are obsolete already
				if (_this.presentlyQueuedZoomLevelAndSlice !== ('' + latestRequest.zoom_level + '_' + latestRequest.slice)) {
					_this.clearRequestQueue();
					return;
				}

				var accumulatedRequestsDeepCopy = $.extend(true, {}, _this.accumulatedRequests);
				_this.clearRequestQueue();
				_this.latestDrawRequestTimestamp = accumulatedRequestsDeepCopy.timestamp;

				_this.drawLowResolutionPreview();
				var lowResBackdrop = setInterval(function(_this) {
					if (_this.lowResolutionPreviewDrawn) {
						_this.drawRequest(accumulatedRequestsDeepCopy);
						clearInterval(lowResBackdrop);
					}
				}, 10, _this);

			}, this.drawingIntervalInMillis , this);
		},
		stopQueue : function() {
			if (!this.queue_handle) {
				return;
			}
			clearInterval(this.queue_handle);
		},
		addToQueue : function(draw_request) {
			// we have no existing queue for the zoom level and slice =>
			// this means: A) we have to create it AND B) we have to empty the queue to get rid of old requests
			if (this.presentlyQueuedZoomLevelAndSlice !== ('' + draw_request.zoom_level + '_' + draw_request.slice)) {
				this.presentlyQueuedZoomLevelAndSlice = '' + draw_request.zoom_level + '_' + draw_request.slice;
				this.accumulatedRequests = null;
				this.requests = [];
			}
			
			// clicks are processed instantly
			if (!draw_request.move) {
				this.clearRequestQueue();
				this.latestDrawRequestTimestamp = draw_request.timestamp;
				
				this.drawLowResolutionPreview();
				var lowResBackdrop = setInterval(function(_this) {
					if (_this.lowResolutionPreviewDrawn) {
						_this.drawRequest(draw_request);
						clearInterval(lowResBackdrop);
					}
				}, 10, this);				
				//this.drawRequest(draw_request);
				return;
			}

			// queue pans and accumulate them
			if (draw_request.move) {
				this.accumulateRequests(draw_request);
			}
			this.requests.push(draw_request);
		},
		accumulateRequests : function(draw_request, accumulatedRequests) {
			if (typeof(accumulatedRequests === 'undefined') || !this.accumulatedRequests) {
				this.accumulatedRequests = accumulatedRequests;
			}
			
			if (typeof(this.accumulatedRequests === 'undefined') || !this.accumulatedRequests) {
				this.accumulatedRequests = draw_request;
			} else {
				// we really only accumulate the deltas, the rest we take from the request
				this.accumulatedRequests.timestamp = draw_request.timestamp;
				this.accumulatedRequests.plane = draw_request.plane;
				this.accumulatedRequests.zoom_level = draw_request.zoom_level;
				this.accumulatedRequests.slice = draw_request.slice;
				this.accumulatedRequests.coords = draw_request.coords;
				this.accumulatedRequests.max_coords_of_event_triggering_plane = draw_request.max_coords_of_event_triggering_plane;
				this.accumulatedRequests.move = draw_request.move;
				
				this.accumulatedRequests.deltas.x += draw_request.deltas.x;
				this.accumulatedRequests.deltas.y += draw_request.deltas.y;
			}
			
			return this.accumulatedRequests;
 		},
 		clearRequestQueue : function() {
			this.accumulatedRequests = [];
			this.requests = [];
 		}, drawLowResolutionPreview : function() {
 			this.lowResolutionPreviewDrawn = false;
 			
			var ctx = this.canvas.getCanvasContext();

			var imageTile = new Image();
			imageTile.src = 
				TissueStack.tile_directory + this.canvas.getDataExtent().data_id + "/" + this.canvas.getDataExtent().zoom_level + "/" 
				+ this.canvas.getDataExtent().plane
				+ "/" + this.canvas.getDataExtent().slice + ".low.res." + this.canvas.image_format;

			(function(_this, imageOffsetX, imageOffsetY) {
				imageTile.onload = function() {
					ctx.drawImage(this,
							imageOffsetX, imageOffsetY,
							this.width - imageOffsetX, this.height - imageOffsetY,
							0, 0, this.width - imageOffsetX, this.height - imageOffsetY);
					_this.lowResolutionPreviewDrawn = true;
				};
			})(this, this.canvas.upper_left_x, this.canvas.upper_left_y);
		}, drawRequest : function(draw_request) {
			var thisHerePlane = this.canvas.getDataExtent().plane;
			
			if (draw_request.plane === thisHerePlane) { // this is the own canvas move
				this.canvas.moveUpperLeftCorner(draw_request.deltas.x, draw_request.deltas.y);
				// these are the moves caused by other canvases
			} else if (thisHerePlane === 'x' && draw_request.plane === 'z') {
				this.canvas.getDataExtent().slice = draw_request.coords.x;
				if (draw_request.move && draw_request.deltas.y != 0) {
					this.canvas.moveUpperLeftCorner(-draw_request.deltas.y , 0);
				} else if (!draw_request.move) {
					this.canvas.drawCoordinateCross(
							{x: this.canvas.cross_x + draw_request.deltas.y, y:  this.canvas.cross_y});
				}
			} else if (thisHerePlane === 'y' && draw_request.plane === 'z') {
				this.canvas.getDataExtent().slice = draw_request.max_coords_of_event_triggering_plane.max_y - draw_request.coords.y;
				if (draw_request.move && draw_request.deltas.x != 0) {
					this.canvas.moveUpperLeftCorner(draw_request.deltas.x , 0);
				} else if (!draw_request.move) {
					this.canvas.drawCoordinateCross({x: this.canvas.cross_x - draw_request.deltas.x, y: this.canvas.cross_y});
				}
			} else if (thisHerePlane === 'x' && draw_request.plane === 'y') {
				this.canvas.getDataExtent().slice = draw_request.coords.x;
				if (draw_request.move && draw_request.deltas.y != 0) {
					this.canvas.moveUpperLeftCorner(0 , draw_request.deltas.y);
				} else if (!draw_request.move) {
					this.canvas.drawCoordinateCross({x: this.canvas.cross_x, y:   this.canvas.cross_y - draw_request.deltas.y});
				}
			} else if (thisHerePlane === 'z' && draw_request.plane === 'y') {
				this.canvas.getDataExtent().slice = draw_request.max_coords_of_event_triggering_plane.max_y - draw_request.coords.y;
				if (draw_request.move && draw_request.deltas.x != 0) {
					this.canvas.moveUpperLeftCorner(draw_request.deltas.x , 0);
				} else if (!draw_request.move) {
					this.canvas.drawCoordinateCross({x:  this.canvas.cross_x - draw_request.deltas.x, y:  this.canvas.cross_y});
				}
			} else if (thisHerePlane === 'y' && draw_request.plane === 'x') {
				this.canvas.getDataExtent().slice = draw_request.coords.x;
				if (draw_request.move && draw_request.deltas.y != 0) {
					this.canvas.moveUpperLeftCorner(0 , draw_request.deltas.y);
				} else if (!draw_request.move) {
					this.canvas.drawCoordinateCross({x:   this.canvas.cross_x , y: this.canvas.cross_y - draw_request.deltas.y});
				}
			} else if (thisHerePlane === 'z' && draw_request.plane === 'x') {
				this.canvas.getDataExtent().slice = draw_request.max_coords_of_event_triggering_plane.max_y - draw_request.coords.y;
				if (draw_request.move && draw_request.deltas.x != 0) {
					this.canvas.moveUpperLeftCorner(0 , -draw_request.deltas.x);
				} else if (!draw_request.move) {
					this.canvas.drawCoordinateCross({x: this.canvas.cross_x, y: this.canvas.cross_y + draw_request.deltas.x});
				}
			}

			// redraw
			this.canvas.drawMe(draw_request.timestamp);

			// tidy up where we left debris
			if (this.canvas.dim_x > this.canvas.getDataExtent().x && draw_request.deltas.x < 0) { // in front of us
				this.canvas.eraseCanvasPortion(0, 0, Math.abs(this.canvas.upper_left_x), this.canvas.dim_y);
			} else if (this.canvas.dim_x > this.canvas.getDataExtent().x && draw_request.deltas.x > 0) { // behind us
				this.canvas.eraseCanvasPortion(
					Math.abs(this.canvas.upper_left_x) + this.canvas.getDataExtent().x, 0,
					this.canvas.dim_x - this.canvas.getDataExtent().x, this.canvas.dim_y);
			}
			
			if (this.canvas.dim_y > this.canvas.getDataExtent().y && draw_request.deltas.y < 0) { // in front of us
				this.canvas.eraseCanvasPortion(0, 0, this.canvas.dim_x, Math.abs(this.canvas.upper_left_y));
			} else if (this.canvas.dim_y > this.canvas.getDataExtent().y && draw_request.deltas.y > 0) { // behind us
				this.canvas.eraseCanvasPortion(
					0, Math.abs(this.canvas.upper_left_y) + this.canvas.getDataExtent().y,
					this.canvas.dim_x, this.canvas.dim_y - this.canvas.getDataExtent().y);
			}
		}
	};
};
