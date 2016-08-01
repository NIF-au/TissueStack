/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
TissueStack.Queue = function (canvas) {
	this.canvas = canvas;
    this.tilesRendered = 0;
    this.totalNumberOfTiles = 0;
    this.cache = {};
    this.is_partial_render = false;
    this.prefetchList = {};
    this.prefetcherHandle = null;
};

TissueStack.Queue.prototype = {
	canvas : null,
	queue_handle : null,
	drawingIntervalInMillis : 100,
	requests : [],
	presentlyQueuedZoomLevelAndSlice: null,
	lowResolutionPreviewDrawn : false,
	latestDrawRequestTimestamp : 0,
	last_sync_timestamp: -1,
    tilesRendered : 0,
    totalNumberOfTiles : 0,
    cache : null,
    is_partial_render : false,
    prefetchList : null,
    prefetcherHandle : null,
	setDrawingInterval : function(value) {
		if (typeof(value) !== 'number' || value < 0) {
			value = 100; // set to default
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

            if (_this.prepareDrawRequest(latestRequest))
                _this.canvas.drawMe(latestRequest.timestamp);
		}, this.drawingIntervalInMillis);
	},
	stopQueue : function() {
		if (!this.queue_handle) {
			return;
		}
		clearInterval(this.queue_handle);
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

        // process through queue
		this.requests.push(draw_request);
		this.startQueue();
	},
	clearRequestQueue : function() {
		this.requests = [];
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
			// crosshair focus
			var crossHairPosition = {x: this.canvas.cross_x, y: this.canvas.cross_y};

            if (this.canvas.data_extent.one_to_one_x != this.canvas.data_extent.origX &&
                draw_request.max_coords_of_event_triggering_plane.step != this.canvas.getDataExtent().step)
                draw_request.slice *= (this.canvas.data_extent.one_to_one_x / this.canvas.data_extent.origX);
            if (this.canvas.data_extent.one_to_one_y != this.canvas.data_extent.origY &&
                draw_request.max_coords_of_event_triggering_plane.step != this.canvas.getDataExtent().step)
                draw_request.slice *= (this.canvas.data_extent.one_to_one_y / this.canvas.data_extent.origY);

			// get slice changes
			var sliceX = draw_request.slice;
			var sliceY = this.canvas.data_extent.one_to_one_y - draw_request.slice;

			// adjust for zoom level
			sliceX = sliceX * (this.canvas.getDataExtent().x / this.canvas.data_extent.one_to_one_x);
			sliceY = sliceY * (this.canvas.getDataExtent().y / this.canvas.data_extent.one_to_one_y);

			sliceX = crossHairPosition.x - sliceX;
			sliceY = (this.canvas.dim_y - crossHairPosition.y) + sliceY;

			if (thisHerePlane === 'x' && draw_request.plane === 'z') {
				this.canvas.setUpperLeftCorner(this.canvas.upper_left_x, sliceY);
			} else if (thisHerePlane === 'y' && draw_request.plane === 'z') {
				this.canvas.setUpperLeftCorner(this.canvas.upper_left_x, sliceY);
			} else if (thisHerePlane === 'x' && draw_request.plane === 'y') {
				this.canvas.setUpperLeftCorner(sliceX, this.canvas.upper_left_y);
			} else if (thisHerePlane === 'z' && draw_request.plane === 'y') {
				this.canvas.setUpperLeftCorner(this.canvas.upper_left_x, sliceY);
			} else if (thisHerePlane === 'y' && draw_request.plane === 'x') {
				this.canvas.setUpperLeftCorner(sliceX, this.canvas.upper_left_y);
			} else if (thisHerePlane === 'z' && draw_request.plane === 'x') {
				this.canvas.setUpperLeftCorner(sliceX, this.canvas.upper_left_y);
			}
			return true;
		}

		// POINT FOCUS ACTION
		if (draw_request.action == 'POINT') {
			this.canvas.drawCoordinateCross(this.canvas.getCenter());
		}

		// COORDINATE CHANGES DUE TO VARYING ZOOM LEVELS BETWEEN THE CANVASES
		var originalZoomLevelDims = this.canvas.getDataExtent().getZoomLevelDimensions(draw_request.zoom_level);
		var crossXOutsideOfExtentX = (draw_request.coords.x < 0) ? -1 : 0;
		if (draw_request.coords.x > (draw_request.max_coords_of_event_triggering_plane.max_x)) {
			crossXOutsideOfExtentX = 1;
		}
		var crossYOutsideOfExtentY = (draw_request.coords.y < 0) ? -1 : 0;
		if (draw_request.coords.y > (draw_request.max_coords_of_event_triggering_plane.max_y )) {
			crossYOutsideOfExtentY = 1;
		}

		if (draw_request.zoom_level != this.canvas.getDataExtent().zoom_level) {
			if (draw_request.coords.x < 0) {
				draw_request.coords.x = Math.abs(draw_request.coords.x - draw_request.max_coords_of_event_triggering_plane.max_x);
				draw_request.upperLeftCorner.x = draw_request.crossCoords.x - draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);
			} else if (draw_request.coords.x > (draw_request.max_coords_of_event_triggering_plane.max_x )) {
				draw_request.coords.x = draw_request.coords.x - (draw_request.max_coords_of_event_triggering_plane.max_x );
				draw_request.upperLeftCorner.x = draw_request.crossCoords.x + draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);
			} else {
				draw_request.upperLeftCorner.x = draw_request.crossCoords.x - draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);
			}

			if (draw_request.coords.y < 0) {
				draw_request.coords.y = Math.abs(draw_request.coords.y);
				draw_request.upperLeftCorner.y = (draw_request.canvasDims.y - draw_request.crossCoords.y) - draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);
			} else if (draw_request.coords.y > (draw_request.max_coords_of_event_triggering_plane.max_y )) {
				draw_request.upperLeftCorner.y = (draw_request.canvasDims.y - draw_request.crossCoords.y) + draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);
			} else {
				draw_request.upperLeftCorner.y = (draw_request.canvasDims.y - draw_request.crossCoords.y) + draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);
			}

			draw_request.coords.x = draw_request.coords.x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);
			draw_request.coords.y = draw_request.coords.y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);

			draw_request.max_coords_of_event_triggering_plane.max_x =
					draw_request.max_coords_of_event_triggering_plane.max_x * (this.canvas.getDataExtent().x / originalZoomLevelDims.x);
			draw_request.max_coords_of_event_triggering_plane.max_y =
					draw_request.max_coords_of_event_triggering_plane.max_y * (this.canvas.getDataExtent().y / originalZoomLevelDims.y);
		}

        if (draw_request.max_coords_of_event_triggering_plane.aniso_factor_x &&
        	draw_request.max_coords_of_event_triggering_plane.aniso_factor_x != 1 &&
            draw_request.max_coords_of_event_triggering_plane.step != this.canvas.getDataExtent().step) {
            draw_request.max_coords_of_event_triggering_plane.max_x /= draw_request.max_coords_of_event_triggering_plane.aniso_factor_x;
            draw_request.coords.x /= draw_request.max_coords_of_event_triggering_plane.aniso_factor_x;
        }
        if (draw_request.max_coords_of_event_triggering_plane.aniso_factor_y &&
        	draw_request.max_coords_of_event_triggering_plane.aniso_factor_y != 1 &&
            draw_request.max_coords_of_event_triggering_plane.step != this.canvas.getDataExtent().step) {
            draw_request.max_coords_of_event_triggering_plane.max_y /= draw_request.max_coords_of_event_triggering_plane.aniso_factor_y;
            draw_request.coords.y /= draw_request.max_coords_of_event_triggering_plane.aniso_factor_y;
        }

		// PAN AND CLICK ACTION
		if (thisHerePlane === 'x' && draw_request.plane === 'z') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(
					(crossXOutsideOfExtentX < 0) ? -99 : ((crossXOutsideOfExtentX > 0) ? (draw_request.max_coords_of_event_triggering_plane.max_x + 99) : draw_request.coords.x));
			this.canvas.setUpperLeftCorner(
					(draw_request.upperLeftCorner.y - (draw_request.max_coords_of_event_triggering_plane.max_y ))
					+ (this.canvas.cross_x - (draw_request.canvasDims.y - draw_request.crossCoords.y)),
					this.canvas.upper_left_y);
		} else if (thisHerePlane === 'y' && draw_request.plane === 'z') {
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(
					(crossYOutsideOfExtentY < 0) ?
							-99 :
							((crossYOutsideOfExtentY > 0) ? (draw_request.max_coords_of_event_triggering_plane.max_y + 99) :
								((draw_request.max_coords_of_event_triggering_plane.max_y ) - draw_request.coords.y)));
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
								((draw_request.max_coords_of_event_triggering_plane.max_y ) - draw_request.coords.y)));
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
			this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(((draw_request.max_coords_of_event_triggering_plane.max_y ) - draw_request.coords.y));
			this.canvas.setUpperLeftCorner(
					this.canvas.upper_left_x ,
					((draw_request.max_coords_of_event_triggering_plane.max_x ) + draw_request.upperLeftCorner.x + ((this.canvas.dim_y - this.canvas.cross_y) - draw_request.crossCoords.x)));
		}

		return true;
    }, prefetchTiles : function() {
        var extent = this.canvas.getDataExtent();
        if (!this.canvas.is_main_view || extent.getIsTiled()) return;

        var aboutToPrefetchId = this.getPrefetchIdentifier();
        // check if we are on the prefetch list...
        if (typeof this.prefetchList[aboutToPrefetchId] === 'object') {
            this.startPrefetcher();
            return;
        }

        this.prefetchList[aboutToPrefetchId] = [];

        var dataSet = TissueStack.dataSetStore.getDataSetById(extent.data_id);
        var endTileX =
            (extent.x % extent.tile_size) === 0 ?
                extent.x / extent.tile_size :
                parseInt(extent.x / extent.tile_size)+1;
        var endTileY =
            (extent.y % extent.tile_size) === 0 ?
                extent.y / extent.tile_size :
                parseInt(extent.y / extent.tile_size)+1;

        for (var tileX = 0  ; tileX < endTileX ; tileX++) {
            for (var tileY = 0 ; tileY < endTileY ; tileY++) {
                var req =
                    TissueStack.Utils.assembleTissueStackImageRequest(
                        dataSet, this.canvas, false, this.canvas.color_map,
                        tileX, tileY);
                this.prefetchList[aboutToPrefetchId].push(req);
            }
        }
        this.startPrefetcher();
	}, getPrefetchIdentifier : function() {
        return this.canvas.getDataExtent().slice + "/" +
            this.canvas.getDataExtent().zoom_level + "/" +
            this.canvas.color_map +
                (this.canvas.contrast ?
                    ("/" + this.canvas.contrast.canvas.contrast.getMinimum() +
                    "/" + this.canvas.contrast.canvas.contrast.getMaximum()) : "");
    }, tidyUp : function() {
		if (this.canvas.getDataExtent().slice < 0 || this.canvas.getDataExtent().slice > this.canvas.getDataExtent().max_slices
				|| this.canvas.upper_left_x > this.canvas.dim_x || this.canvas.upper_left_x + this.canvas.data_extent.x < 0
				|| this.canvas.upper_left_y < 0 || this.canvas.upper_left_y - this.canvas.data_extent.y > this.canvas.dim_y) {
			this.canvas.eraseCanvasContent(); // in these cases we erase the entire content
			return;
		}

		// tidy up where we left debris
		if (this.canvas.upper_left_x > 0) { // in front of us
            var tillX = Math.ceil(this.canvas.upper_left_x);
            if (tillX < 0) tillX = 0;
			this.canvas.eraseCanvasPortion(0, 0, tillX, this.canvas.dim_y);
		}
		if (this.canvas.upper_left_x <= 0 || (this.canvas.upper_left_x >= 0 && (this.canvas.upper_left_x + this.canvas.getDataExtent().x-1) < this.canvas.dim_x)){ // behind us
			this.canvas.eraseCanvasPortion(
					Math.floor(this.canvas.upper_left_x + this.canvas.getDataExtent().x-1), 0,
					this.canvas.dim_x - Math.floor(this.canvas.upper_left_x + this.canvas.getDataExtent().x-1), this.canvas.dim_y);
		}

		if (this.canvas.upper_left_y < 0 || (this.canvas.upper_left_y < this.canvas.dim_y && this.canvas.upper_left_y >= 0)) { // in front of us
			this.canvas.eraseCanvasPortion(0, 0, this.canvas.dim_x, (this.canvas.upper_left_y <= 0) ? this.canvas.dim_y : Math.ceil(this.canvas.dim_y - this.canvas.upper_left_y));
		}
		if ((this.canvas.upper_left_y - this.canvas.getDataExtent().y-1) >= this.canvas.dim_y || (this.canvas.upper_left_y - this.canvas.getDataExtent().y-1) > 0) { // behind us
			this.canvas.eraseCanvasPortion(
				0, (this.canvas.upper_left_y >= this.canvas.dim_y && this.canvas.upper_left_y - this.canvas.getDataExtent().y-1 >= this.canvas.dim_y) ? 0 : Math.floor(this.canvas.dim_y - (this.canvas.upper_left_y - this.canvas.getDataExtent().y)),
				this.canvas.dim_x,
				(this.canvas.upper_left_y >= this.canvas.dim_y && this.canvas.upper_left_y - this.canvas.getDataExtent().y-1 >= this.canvas.dim_y) ?
						this.canvas.dim_y : this.canvas.dim_y - Math.floor(this.canvas.dim_y - (this.canvas.upper_left_y - this.canvas.getDataExtent().y)));
		}
	}, displayLoadingProgress : function(reset, error) {
        if (TissueStack.phone) return;

        var dataset_id = this.canvas.dataset_id;
        var plane = this.canvas.data_extent.plane

        if (typeof(reset) == 'boolean' && reset) {
            $("#" + dataset_id + " .tile_count_div span." + plane).css('color', '');
            $("#" + dataset_id + " .tile_count_div progress").val(0);
            if (typeof(error) === 'boolean' && error)
                $("#" + dataset_id + " .tile_count_div span." + plane).html("ERR");
        else {
                $("#" + dataset_id + " .tile_count_div span." + plane).html("0%");
                $("#" + dataset_id + " .tile_count_div span." + plane).hide();
            }
            return;
        }

        if (this.tilesRendered >= this.totalNumberOfTiles) {
            $("#" + dataset_id + " .tile_count_div progress").val(100);
            $("#" + dataset_id + " .tile_count_div span." + plane).html("100%");
            $("#" + dataset_id + " .tile_count_div span." + plane).css('color', '');
            $("#" + dataset_id + " .tile_count_div span." + plane).show();
            return;
        }

        var perCent = Math.round((this.tilesRendered / this.totalNumberOfTiles) * 100);
        $("#" + dataset_id + " .tile_count_div progress").val(Math.round(perCent));
        $("#" + dataset_id + " .tile_count_div span." + plane).html(perCent + "%");
        $("#" + dataset_id + " .tile_count_div span." + plane).css('color', 'red');
        $("#" + dataset_id + " .tile_count_div span." + plane).show();
    }, setTotalNumberOfTiles : function(totalNumberOfTiles) {
        this.totalNumberOfTiles = totalNumberOfTiles;
        this.tilesRendered = 0;
        this.displayLoadingProgress();
    }, hasFinishedTiling : function() {
        return (this.tilesRendered >= this.totalNumberOfTiles);
    },
    incrementTileCount : function() {
        this.tilesRendered++;
        this.displayLoadingProgress();
    }, dispose : function() {
        this.latestDrawRequestTimestamp = -1;
        this.stopQueue();
        this.canvas = null;
        this.cache = null;
        this.stopPrefetcher();
        this.prefetchList = null;
    }, isImageCached : function(cache_key) {
        if (typeof cache_key !== 'string' || cache_key.length === 0)
            return false;
        return (this.cache && (this.cache[cache_key] instanceof Image));
    }, addImageToCache : function(cache_key, img) {
        if (typeof cache_key !== 'string' || cache_key.length === 0 ||
            !(img instanceof Image)) return;
        if (this.cache) this.cache[cache_key] = img;
    }, getCachedImage : function(cache_key) {
        if (typeof cache_key !== 'string' || cache_key.length === 0) return null;
        if (this.cache) return this.cache[cache_key];
        return null;
    }, stopPrefetcher : function() {
        if (this.prefetcherHandle) {
            clearInterval(this.prefetcherHandle);
            this.prefetcherHandle = null;
            this.prefetchStart = null;
        }
    }, startPrefetcher : function() {
        if (this.prefetcherHandle) return;

        this.prefetcherHandle = setInterval(
            function() {
                var now = new Date().getTime();
                if (this.prefetchStart === null)
                    this.prefetchStart = now;
                // we are rather conservative so as to not impact on performance
                // if we are presently not the main canvas queue we don't do
                // anything. neither do we pre-fetch if we are not in the zoom
                // level/color/contrast setting enqueued or, of course, once
                // we have finished prefetching altogether
                // as a safeguard in case of long term connectivity loss or
                // some crazy event that would keep the image list from decreasing
                // we have a duration check so that prefetcher task will run
                // no longer than 5 min
                var presentPrefetchId = this.getPrefetchIdentifier();
                if (!this.canvas.is_main_view ||
                    typeof this.prefetchList[presentPrefetchId] !== 'object' ||
                    this.prefetchList[presentPrefetchId].length === 0 ||
                    now > this.prefetchStart + 5*60*1000) {
                     this.stopPrefetcher();
                     return;
                 }

                var reqToProcess = this.prefetchList[presentPrefetchId];
                var newList = [];
                var todoCount = 0;
                for (var r=0;r<reqToProcess.length;r++) {
                    var req = reqToProcess[r];
                    // we do this in batches of 10
                    // too much
                    if (todoCount >= 10) {
                        newList.push(req);
                        continue;
                    }
                    if (!this.isImageCached(req.cache_key)) {
                        todoCount++;
                        newList.push(req);
                        var imageTile = new Image();

                        imageTile.crossOrigin = '';
                        imageTile.src = req.url;

                        (function(this_, cache_key) {
                            imageTile.onload = function() {
                                this_.addImageToCache(cache_key, this);
                            }
                        })(this, req.cache_key);
                    }
                }
                this.prefetchList[presentPrefetchId] = newList;
            }.bind(this), 2000);
    }
};
