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
TissueStack.Events = function(canvas, include_cross_hair) {
	this.canvas = canvas;
	this.setIncludeCrossHair(include_cross_hair);
	this.registerCommonEvents();
	if (TissueStack.desktop || TissueStack.debug) {
		this.registerDesktopEvents();
	}
	if (TissueStack.tablet || TissueStack.phone) {
		this.registerMobileEvents();
	}
};

TissueStack.Events.prototype = {
	include_cross_hair : true,	
	setIncludeCrossHair : function(include_cross_hair) {
		// include cross hair canvas or not
		if (typeof(include_cross_hair) != 'boolean' || include_cross_hair == true) {
			this.include_cross_hair = true;
		} else {
			this.include_cross_hair = false;
		}
	},
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
		this.getCanvasElement().bind("mouseup", function(e) {
			// call pan move
			_this.panEnd();
		});
		
		// CLICK
		if (this.include_cross_hair) {
			this.getCanvasElement().bind("click", function(e) {
			
				if (e.originalEvent.touches) {
					var touches = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
					e.pageX = touches.pageX;
					e.pageY = touches.pageY;
				}
			
				// call click
				_this.click(e );
			});
		}
		
		// SYNC
		$(document).bind("sync", function(e, data_id, dataset_id, timestamp, action, plane, zoom_level, slice, coords, max_coords_of_event_triggering_plane, upperLeftCorner, crossCoords, canvasDims) {
			// call sync
			_this.sync(e,  data_id, dataset_id, timestamp, action, plane, zoom_level, slice, coords, max_coords_of_event_triggering_plane, upperLeftCorner, crossCoords, canvasDims);
		});

		// SYNC for ZOOM
		$(document).bind("zoom", function(e,  data_id, dataset_id, timestamp, action, plane, zoom_level, slice) {
			// call sync zoom
			_this.sync_zoom(e,  data_id, dataset_id, timestamp, action, plane, zoom_level, slice);
		});
				
	}, registerMobileEvents: function() {
		var _this = this;
		var delta = 0;
		var tmpTouches;

		// TOUCH START
		this.getCanvasElement().bind("touchstart", function(e) {
            var touches = e.originalEvent.touches || e.originalEvent.changedTouches;
            e.pageX = touches[0].pageX;
            e.pageY = touches[0].pageY;
			
			// tablet zoom 
		    if (touches.length > 1) {
				delta = 
					Math.sqrt(
						Math.pow(touches[1].pageX - touches[0].pageX,2) +
						Math.pow(touches[1].pageY - touches[0].pageY,2));
		    	return;
			};

			// handle PAN
			_this.panStart(e);
		});			

		// TOUCH MOVE
		this.getCanvasElement().bind("touchmove", function(e) {
            var touches = e.originalEvent.touches || e.originalEvent.changedTouches;
            e.pageX = touches[0].pageX;
            e.pageY = touches[0].pageY;
			
			// zoom for tablets
			if (touches.length > 1) {
				tmpTouches = touches;
		    	return;
			};
			
			// call panAndMove
			_this.panAndMove(e);
		});

		// TOUCH END
		this.getCanvasElement().bind("touchend", function(e) {
            var touches = e.originalEvent.touches || e.originalEvent.changedTouches;

			// zoom for tablets
		    if (!_this.canvas.mouse_down && touches && tmpTouches && tmpTouches.length==2) {
				if (delta > Math.sqrt(
							Math.pow(tmpTouches[1].pageX - tmpTouches[0].pageX,2) +
							Math.pow(tmpTouches[1].pageY - tmpTouches[0].pageY,2))) 
					delta = -1;
				else 
					delta = 1;
		    	
				_this.zoom(e, delta);		    		
		    	return;	
			};
		
			// call pan move
			_this.panEnd();
		});
		
		//DOUBLE TAP TO ENLARGE IMAGES
		if (TissueStack.phone)
			this.getCanvasElement().bind('doubletap', function(e) {
			_this.zoom(e, 1);
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
			if (delta == 0) {
				return;
			}
			
			// annoying opera
			if ($.browser.opera) {
				delta = delta > 0 ? 1.5 : -1.5; 
			}

			if (!e.altKey) {
				// call zoom
				_this.zoom(e, delta);
				return;
			}

			if (delta > 0) _this.canvas.data_extent.slice++; else  _this.canvas.data_extent.slice--;
			
			var slider = $("#" + _this.canvas.dataset_id + "_canvas_main_slider");
			try {
				slider.val(_this.canvas.data_extent.slice).slider("refresh");
            } catch(ignored) {}
			
			//_this.changeSliceForPlane(_this.canvas.data_extent.slice);
			setTimeout(function(){_this.updateCoordinateDisplay();}, 500);
			e.stopPropagation();
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
	}, unbindAllEvents : function() {
		// UNBIND COMMON EVENTS
		this.getCanvasElement().unbind("touchend mouseup");
		this.getCanvasElement().unbind("click");
		$(document).unbind("sync");
		$(document).unbind("zoom");
		
		this.getCanvasElement().unbind("mousedown");			
		this.getCanvasElement().unbind("mousemove");
		this.getCanvasElement().unbind('mousewheel');

		this.getCanvasElement().unbind("touchstart");			
		this.getCanvasElement().unbind("touchmove");
		this.getCanvasElement().unbind('doubletap');
	},panStart : function(e) {
		var coords = TissueStack.Utils.getRelativeMouseCoords(e);
		
		this.canvas.mouse_down = true;
		this.canvas.mouse_x = coords.x;
		this.canvas.mouse_y = coords.y;
	}, panEnd : function() {
		this.canvas.mouse_down = false;
		this.updateCoordinateDisplay();		
	}, panAndMove : function(e) {
		var now =new Date().getTime(); 

		var dataSet = TissueStack.dataSetStore.getDataSetById(this.canvas.getDataExtent().data_id);
		var coords = TissueStack.Utils.getRelativeMouseCoords(e);
		var relCoordinates = this.canvas.getDataCoordinates(coords);

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

            var aniso_factor_x = 1;
            var aniso_factor_y = 1;
            
            if (this.canvas.data_extent.one_to_one_x != this.canvas.data_extent.origX)
                aniso_factor_x = (this.canvas.data_extent.one_to_one_x / this.canvas.data_extent.origX);
            if (this.canvas.data_extent.one_to_one_y != this.canvas.data_extent.origY)
                aniso_factor_y = (this.canvas.data_extent.one_to_one_y / this.canvas.data_extent.origY);

			// queue events 
			this.canvas.queue.addToQueue(
					{	
						data_id : this.canvas.data_extent.data_id,
						dataset_id : this.canvas.dataset_id,	 
						timestamp : now,
						action: 'PAN',
						plane: this.canvas.getDataExtent().plane,
						zoom_level : this.canvas.getDataExtent().zoom_level,
						slice : this.canvas.getDataExtent().slice,
						coords: relCoordinates,
						max_coords_of_event_triggering_plane : 
                            {max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y,
                             aniso_factor_x: aniso_factor_x, aniso_factor_y: aniso_factor_y, step: this.canvas.getDataExtent().step},
						upperLeftCorner : upper_left_corner,
						crossCoords : cross_coords,
						canvasDims : canvas_dims
					});
			
			// send message out to others that they need to redraw as well
			this.canvas.getCanvasElement().trigger("sync", 
						[	this.canvas.data_extent.data_id,
						 	this.canvas.dataset_id,	 
						 	now,
						 	'PAN',
						 	this.canvas.getDataExtent().plane,
						 	this.canvas.getDataExtent().zoom_level,
						 	this.canvas.getDataExtent().slice,
						 	this.canvas.getRelativeCrossCoordinates(),
						 	    {max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y,
                                aniso_factor_x: aniso_factor_x, aniso_factor_y: aniso_factor_y, step: this.canvas.getDataExtent().step},
						 	upper_left_corner,
						 	cross_coords,
						 	canvas_dims
			            ]);
		} else {
			if (this.canvas.isDragging) this.updateCoordinateDisplay();
			this.canvas.isDragging = false;
		}
	}, changeSliceForPlane : function(slice) {
		var now =new Date().getTime(); 
		if (typeof(slice) != "number") {
			slice = parseInt(slice);
		}
		if (slice < 0) slice = 0;
		if (slice > this.canvas.data_extent.max_slices) slice = this.canvas.data_extent.max_slices;
		
		this.canvas.data_extent.slice = slice;
		
		var upper_left_corner = {x: this.canvas.upper_left_x, y: this.canvas.upper_left_y};
		var cross_coords = {x: this.canvas.cross_x, y: this.canvas.cross_y};
		var canvas_dims = {x: this.canvas.dim_x, y: this.canvas.dim_y};
		
        var aniso_factor_x = 1;
        var aniso_factor_y = 1;

        if (this.canvas.data_extent.one_to_one_x != this.canvas.data_extent.origX)
            aniso_factor_x = (this.canvas.data_extent.one_to_one_x / this.canvas.data_extent.origX);
        if (this.canvas.data_extent.one_to_one_y != this.canvas.data_extent.origY)
            aniso_factor_y = (this.canvas.data_extent.one_to_one_y / this.canvas.data_extent.origY);
        
		// queue events 
		this.canvas.queue.addToQueue(
				{	data_id : this.canvas.data_extent.data_id,
					dataset_id : this.canvas.dataset_id,	 
					timestamp : now,
					action: 'PAN',
					plane: this.canvas.getDataExtent().plane,
					zoom_level : this.canvas.getDataExtent().zoom_level,
					slice : this.canvas.getDataExtent().slice,
					coords: {x: 0, y: 0},
					max_coords_of_event_triggering_plane : 
                        {max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y,
                        aniso_factor_x: aniso_factor_x, aniso_factor_y: aniso_factor_y, step: this.canvas.getDataExtent().step},
					upperLeftCorner : upper_left_corner,
					crossCoords : cross_coords,
					canvasDims : canvas_dims
				});

		// send message out to others that they need to redraw as well
		this.canvas.getCanvasElement().trigger("sync", 
					[	this.canvas.data_extent.data_id,
					 	this.canvas.dataset_id,	 
					 	now,
					 	'SLICE',
					 	this.canvas.getDataExtent().plane,
					 	this.canvas.getDataExtent().zoom_level,
					 	this.canvas.getDataExtent().slice,
					 	{x: 0, y: 0},
					 	{max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y,
					 	 aniso_factor_x: aniso_factor_x, aniso_factor_y: aniso_factor_y, step: this.canvas.getDataExtent().step},
					 	upper_left_corner,
					 	cross_coords,
					 	canvas_dims
		            ]);
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

		if (TissueStack.sync_datasets && !TissueStack.overlay_datasets && TissueStack.dataSetNavigation.selectedDataSets.count > 1) {
			this.canvas.queue.latestDrawRequestTimestamp = now;
			this.canvas.queue.drawLowResolutionPreview(now);
			this.canvas.queue.drawRequestAfterLowResolutionPreview(null, now);
		}

        var aniso_factor_x = 1;
        var aniso_factor_y = 1;

        if (this.canvas.data_extent.one_to_one_x != this.canvas.data_extent.origX)
            aniso_factor_x = (this.canvas.data_extent.one_to_one_x / this.canvas.data_extent.origX);
        if (this.canvas.data_extent.one_to_one_y != this.canvas.data_extent.origY)
            aniso_factor_y = (this.canvas.data_extent.one_to_one_y / this.canvas.data_extent.origY);
		
		// send message out to others that they need to redraw as well
		this.canvas.getCanvasElement().trigger("sync",
								[this.canvas.data_extent.data_id,
								 this.canvas.dataset_id,	 
								 now,
								'CLICK',
								this.canvas.getDataExtent().plane,
								this.canvas.getDataExtent().zoom_level,
								this.canvas.getDataExtent().slice,
								this.canvas.getRelativeCrossCoordinates(),
		                          {max_x: this.canvas.getDataExtent().x, max_y: this.canvas.getDataExtent().y,
		                           aniso_factor_x :aniso_factor_x, aniso_factor_y: aniso_factor_y, step: this.canvas.getDataExtent().step},
		                        upper_left_corner,
		                        cross_coords,
		                        canvas_dims
		                       ]);

		// update coordinate info displayed
		this.updateCoordinateDisplay();
	}, sync : function(e, data_id, dataset_id, timestamp, action, plane, zoom_level, slice, coords, max_coords_of_event_triggering_plane, upperLeftCorner, crossCoords, canvasDims) {
		// ignore one's own events
		var thisHerePlane = this.canvas.getDataExtent().plane;
		if (thisHerePlane === plane) {
			return;
		}
		
		// if more than 1 data set is displayed, we stop propagation to the other here !
		var thisHereDataSet = this.canvas.dataset_id;
		if (thisHereDataSet != dataset_id) {
			return false;
		}
		
		// queue events 
		this.canvas.queue.addToQueue(
				{	data_id : data_id,
					dataset_id : dataset_id,	 
					timestamp : timestamp,
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
		delta = Math.ceil(delta);
		
		if (delta < 1) {
			delta = -1;
		} else if (delta > 1) {
			delta = 1;
		} 
		
		var now = new Date().getTime();					
		var newZoomLevel = this.canvas.getDataExtent().zoom_level + delta;
		
		if (TissueStack.phone
				&& (newZoomLevel == this.canvas.data_extent.zoom_level ||  newZoomLevel < 0 || newZoomLevel >= this.canvas.data_extent.zoom_levels.length)) {
			newZoomLevel = 0;
		} else if (newZoomLevel < 0 || newZoomLevel >= this.canvas.data_extent.zoom_levels.length)
			return;

		this.canvas.eraseCanvasContent();
		this.canvas.queue.addToQueue(
				{	data_id : this.canvas.data_extent.data_id,
					dataset_id : this.canvas.dataset_id,	 
					timestamp : now,
					action : "ZOOM",
					plane: this.canvas.getDataExtent().plane,
					zoom_level : newZoomLevel,
					slice : this.canvas.getDataExtent().slice
					
				});
		e.stopPropagation();
	}, sync_zoom : function(e, data_id, dataset_id, timestamp, action, plane, zoom_level, slice) {
		// ignore one's own events
		var thisHerePlane = this.canvas.getDataExtent().plane;
		if (thisHerePlane === plane) {
			return;
		}

		// if more than 1 data set is displayed, we stop propagation to the other here !
		var thisHereDataSet = this.canvas.dataset_id;
		if (thisHereDataSet != dataset_id) {
			return false;
		}
		
		this.canvas.queue.addToQueue(
				{	data_id : this.canvas.data_extent.data_id,
					dataset_id : this.canvas.dataset_id,	 
					timestamp : timestamp,
					action : action,
					plane: plane,
					zoom_level : zoom_level,
					slice : slice
				});
		
	}, updateCoordinateDisplay : function(clearTextInputs, exitAfterReset) {
		if (clearTextInputs) {// reset
			$("#canvas_point_x").val("");
			$("#canvas_point_y").val("");
			$("#canvas_point_z").val("");
			$("#canvas_point_value").val("");
			
			var ontTree = $("#ontology_tree");
            if (ontTree && ontTree.length > 0 && ontTree.empty) {
                ontTree.empty();
            }
			
			if (exitAfterReset)
				return;
		}
		
		var relCrossCoords = this.canvas.getRelativeCrossCoordinates();
		relCrossCoords.z = this.canvas.data_extent.slice;
		var worldCoordinates = this.canvas.getDataExtent().getWorldCoordinatesForPixel(relCrossCoords);
		
		// update coordinate info displayed
		var __target = this;
		if (!TissueStack.phone && !__target.canvas.is_main_view) { // if we are not the main view, delegate there...
			var ds = TissueStack.dataSetStore.getDataSetById(__target.canvas.getDataExtent().data_id);
			if (ds && ds.planes)
				for (p in ds.planes)
					if (ds.planes[p].is_main_view) {
						__target = ds.planes[p];
						break;
					}
			if (__target) {
				relCrossCoords = __target.getRelativeCrossCoordinates();
				relCrossCoords.z = __target.getDataExtent().slice;
				setTimeout(function() {
					__target.updateCoordinateInfo(
							relCrossCoords, 
							__target.getDataExtent().getWorldCoordinatesForPixel(relCrossCoords));}
					, 500);
			}
			return;
		}
		
		setTimeout(function() {
		__target.canvas.updateCoordinateInfo(relCrossCoords, worldCoordinates);}
		, 500);
	}
};
