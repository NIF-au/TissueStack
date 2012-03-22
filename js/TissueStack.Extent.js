TissueStack.Extent = function () {
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
				return TissueStack.Utils.getCenter(this.x,this.y);
			}
		};
};
