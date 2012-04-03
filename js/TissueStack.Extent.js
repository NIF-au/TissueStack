TissueStack.Extent = function () {
		return {
			tile_size : 256,	
			x : 0,
			y : 0,
			one_to_one_x : 0,
			one_to_one_y : 0,
			data_id: "",
			one_to_one_zoom_level : 0,
			zoom_level : 0,
			zoom_level_factor : 1,
			zoom_levels : [],
			plane: 'z',
			max_slices : 0,
			slice: 0,
			init : function (data_id, zoom_level, plane, max_slices, x, y, zoom_levels) {
				this.setDataId(data_id);
				this.setZoomLevel(zoom_level);
				this.setPlane(plane);
				this.max_slices = max_slices;
				this.setSlice(Math.floor(max_slices / 2));
				this.setDimensions(x, y);
				this.rememberOneToOneZoomLevel(this.x, this.y, this.zoom_level);
				this.setZoomLevels(zoom_levels);
			}, setDataId : function(data_id) {
				if (typeof(data_id) != "string" || data_id.trim().length == 0) {
					throw "data_id has to be a non-empty string";
				}
				this.data_id = data_id;
			}, setZoomLevel : function(zoom_level) {
				if (typeof(zoom_level) != "number" || Math.floor(zoom_level) < 0) {
					throw "zoom_level has to be a non-negative integer";
				}
				this.zoom_level = Math.floor(zoom_level);
			}, setZoomLevels : function(zoom_levels) {
				if (typeof(zoom_levels) != "object" || zoom_levels.length == 0) {
					throw "zoom_levels are missing";
				}
				this.zoom_levels = zoom_levels;
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
				this.x = Math.floor(x);
				if (typeof(y) != "number" || Math.floor(y) < 0) {
					throw "y has to be a non-negative integer";
				}
				this.y = Math.floor(y);
			}, rememberOneToOneZoomLevel : function(x, y, zoom_level) {
				if (typeof(x) != "number" || Math.floor(x) < 0 || typeof(y) != "number" || Math.floor(y) < 0 || typeof(zoom_level) != "number" || Math.floor(zoom_level) < 0) {
					return;
				}
				this.one_to_one_zoom_level = zoom_level;
				this.one_to_one_x = x;
				this.one_to_one_y = y;
			}, changeToZoomLevel : function(zoom_level) {
				// zoom level is not a number greater than 0
				if (typeof(zoom_level) != "number" || Math.floor(zoom_level) < 0) {
					return;
				}
				zoom_level =  Math.floor(zoom_level);
				// if requested zoom level matches the present, there is nothing to do
				if (zoom_level == this.zoom_level) {
					return;
				}
				
				var zoom_level_factor = this.zoom_levels[zoom_level];
				// if requested zoom level factor does not exist or is not a number greater than 0, say good bye
				if (typeof(zoom_level_factor) != 'number' || zoom_level_factor <= 0) {
					return;
				}
				
				// calculate bounds for extent (relative to 1:1 bounds given) if they haven't already been calculated once before
				// we floor the values by default
				this.zoom_level_factor = zoom_level_factor;
				this.setZoomLevel(zoom_level);
				this.setDimensions(Math.floor(this.one_to_one_x * this.zoom_level_factor), Math.floor(this.one_to_one_y * this.zoom_level_factor));
			}, getSliceWithRespectToZoomLevel : function() {
				return Math.floor(this.slice * this.zoom_level_factor);
			}, getCenter : function () {
				return TissueStack.Utils.getCenter(this.x,this.y);
			}
		};
};
