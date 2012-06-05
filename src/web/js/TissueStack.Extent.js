TissueStack.Extent = function (data_id, zoom_level, plane, max_slices, x, y, zoom_levels, worldCoordinatesTransformationMatrix) {
	this.setDataId(data_id);
	this.setZoomLevel(zoom_level);
	this.setPlane(plane);
	this.max_slices = max_slices;
	this.setSlice(Math.floor(max_slices / 2));
	this.setDimensions(x, y);
	this.rememberOneToOneZoomLevel(this.x, this.y, this.zoom_level);
	this.setZoomLevels(zoom_levels);
	this.worldCoordinatesTransformationMatrix = worldCoordinatesTransformationMatrix;
};

TissueStack.Extent.prototype = {
	tile_size : 256,	
	x : 0,
	y : 0,
	one_to_one_y : 0,
	data_id: "",
	one_to_one_zoom_level : 0,
	zoom_level : 0,
	zoom_level_factor : 1,
	zoom_levels : [],
	plane: 'z',
	max_slices : 0,
	slice: 0,
	worldCoordinatesTransformationMatrix: null,
	setDataId : function(data_id) {
		if (typeof(data_id) != "string" || data_id.length == 0) {
			throw new Error("data_id has to be a non-empty string");
		}
		this.data_id = data_id;
	}, setZoomLevel : function(zoom_level) {
		if (typeof(zoom_level) != "number" || Math.floor(zoom_level) < 0) {
			throw new Error("zoom_level has to be a non-negative integer");
		}
		this.zoom_level = Math.floor(zoom_level);
	}, setZoomLevels : function(zoom_levels) {
		if (typeof(zoom_levels) != "object" || zoom_levels.length == 0) {
			throw new Error("zoom_levels are missing");
		}
		this.zoom_levels = zoom_levels;
	}, setPlane : function(plane) {
		if (typeof(plane) != "string" || !(plane == 'x' || plane == 'y' || plane == 'z')) {
			throw new Error("plane has to be one of the following: 'z', 'y' or 'z'");
		}
		this.plane = plane;
	}, setSlice : function(slice) {
		if (typeof(slice) != "number" || Math.floor(slice) < 0) {
			throw new Error("slice has to be a non-negative integer");
		}
		this.slice = slice;
	}, setDimensions : function(x,y) {
		if (typeof(x) == "string") {
			x = parseInt(x);
		}
		if (typeof(x) != "number" || Math.floor(x) < 0) {
			throw new Error("x has to be a non-negative integer");
		}
		this.x = Math.floor(x);
		if (typeof(y) == "string") {
			y = parseInt(y);
		}
		if (typeof(y) != "number" || Math.floor(y) < 0) {
			throw new Error("y has to be a non-negative integer");
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
		var zoom_level_factor = this.getZoomLevelFactorForZoomLevel(zoom_level);

		if (zoom_level_factor == -1) {
			return;
		}
		
		// calculate bounds for extent (relative to 1:1 bounds given) if they haven't already been calculated once before
		// we floor the values by default
		this.zoom_level_factor = zoom_level_factor;
		this.setZoomLevel(zoom_level);
		var zoom_level_dim = this.getZoomLevelDimensions(zoom_level);
		if (!zoom_level_dim) {
			return;
		}
		this.setDimensions(zoom_level_dim.x, zoom_level_dim.y);
	}, getZoomLevelFactorForZoomLevel : function(zoom_level) {
		if (typeof(zoom_level) != "number" || Math.floor(zoom_level) < 0 || Math.floor(zoom_level) >= this.zoom_levels.length) {
			return -1;
		}
		return this.zoom_levels[Math.floor(zoom_level)];
	}, getZoomLevelDelta : function(zoom_level1, zoom_level2) {
		if (typeof(zoom_level1) != "number" || typeof(zoom_level2) != "number") {
			return 0;
		}
		zoom_level1 = Math.floor(zoom_level1);
		zoom_level2 = Math.floor(zoom_level2);
		
		if (zoom_level1 == zoom_level2) {
			return 0;
		}
		
		var dimZoomLevel1 =  this.getZoomLevelDimensions(zoom_level1);
		var dimZoomLevel2 =  this.getZoomLevelDimensions(zoom_level2);
		
		if (!dimZoomLevel1 || !dimZoomLevel2) {
			return null;
		}
		
		return {x: dimZoomLevel1.x - dimZoomLevel2.x, y: dimZoomLevel1.y - dimZoomLevel2.y};
	}, getZoomLevelDimensions : function(zoom_level) {
		var zoomLevelFactor = this.getZoomLevelFactorForZoomLevel(zoom_level);
		
		if (zoomLevelFactor == -1) {
			return null;
		}
		
		return {x: Math.floor(this.one_to_one_x * zoomLevelFactor), y: Math.floor(this.one_to_one_y * zoomLevelFactor)};
	}, setSliceWithRespectToZoomLevel : function(slice) {
		if (this.zoom_level == 1) {
			this.slice = Math.floor(slice / this.zoom_level_factor);	
		} else {
			this.slice = Math.ceil(slice / this.zoom_level_factor);
		}

		var canvasSlider = $("#canvas_" + ((TissueStack.desktop || TissueStack.tablet) ? "main" : this.plane) + "_slider");
		var mainCanvas = this.plane;
		
		var planeId = 
			TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray(canvasSlider.attr("class").split(" "), "^canvas_");
		if (planeId) {
			var startPos = "canvas_".length;
			mainCanvas = planeId.substring(startPos, startPos + 1);
		}
		
		if (mainCanvas == this.plane && canvasSlider && canvasSlider.length > 0) {
			slice = this.slice < 0 ? this.max_slices : (this.slice > this.max_slices ? 0 : this.slice);
			canvasSlider.attr("value",slice > this.max_slices ? this.max_slices : slice);
			canvasSlider.blur();
		}
	}, getCenter : function () {
		return TissueStack.Utils.getCenter(this.x,this.y);
	}, getWorldCoordinatesForPixel : function(pixelCoords) {
		
		if (pixelCoords.x < 0 || pixelCoords.x > this.x - 1 
				|| pixelCoords.y < 0 || pixelCoords.y > this.y - 1 
				|| this.slice < 0 || this.slice > this.max_slices) {
			return null;
		}

		// check optional z aka slices
		if (typeof(pixelCoords.z) !== 'undefined') {
			if (pixelCoords.z < 0 || pixelCoords.z > this.max_slices) {
				return;
			}
		} else {
			pixelCoords.z = this.slice;
		}

		// now we'll have to correct x and y according to their zoom level to get the 1:1 pixel Coordinates which can then be transformed
		if (this.zoom_level == 1) {
			pixelCoords.x = Math.floor(pixelCoords.x * (this.one_to_one_x / this.x));
			pixelCoords.y = Math.floor(pixelCoords.y * (this.one_to_one_y / this.y));
		} else {
			pixelCoords.x = Math.ceil(pixelCoords.x * (this.one_to_one_x / this.x));
			pixelCoords.y = Math.ceil(pixelCoords.y * (this.one_to_one_y / this.y));
		}
		
		pixelCoords = TissueStack.Utils.transformPixelCoordinatesToWorldCoordinates(
				[pixelCoords.x, this.one_to_one_y - pixelCoords.y, pixelCoords.z, 1], 
				this.worldCoordinatesTransformationMatrix);

		if (!pixelCoords) {
			return null;
		}
		
		// return world coordinates
		return {x: pixelCoords[0], y: pixelCoords[1], z: pixelCoords[2]};

	},
	getPixelForWorldCoordinates : function(worldCoords) {
		if (worldCoords == null) {
			return null;
		}

		var pixelCoords = TissueStack.Utils.transformWorldCoordinatesToPixelCoordinates(
				[worldCoords.x, worldCoords.y, worldCoords.z, 1],
				this.worldCoordinatesTransformationMatrix);
		
		if (pixelCoords == null) {
			return null;
		}
		
		pixelCoords = {x: pixelCoords[0], y: pixelCoords[1], z: pixelCoords[2]};
		
		// now we have to correct x and y according to their zoom level
		if (this.zoom_level == 1) {
			pixelCoords.x = Math.floor(pixelCoords.x * (this.x / this.one_to_one_x));
			pixelCoords.y = this.y - Math.floor(pixelCoords.y * (this.y / this.one_to_one_y));
			pixelCoords.z = Math.floor(pixelCoords.z);
		} else {
			pixelCoords.x = Math.ceil(pixelCoords.x * (this.x / this.one_to_one_x));
			pixelCoords.y = this.y - Math.ceil(pixelCoords.y * (this.y /this.one_to_one_y));
			pixelCoords.z = Math.ceil(pixelCoords.z);
		}
		
		// because of rounding inaccuracies it can happen that exceed the image's pixel dimensions by 1,
		// not to mention that we could have been handed in extreme coordinates that exceeded the world coordinates to begin with 
		if (pixelCoords.x < 0) {
			pixelCoords.x = 0;
		} else if (pixelCoords.x >= this.x) {
			pixelCoords.x = this.x - 1;
		}
		if (pixelCoords.y < 0) {
			pixelCoords.y = 0;
		} else if (pixelCoords.y >= this.y) {
			pixelCoords.y = this.y - 1;
		}
		if (pixelCoords.z < 0) {
			pixelCoords.z = 0;
		} else if (pixelCoords.z >= this.max_slices) {
			pixelCoords.z = this.max_slices;
		}
		
		// return pixel coordinates
		return pixelCoords;
	},
	getExtentCoordinates : function() {
		// if world coords translation matrix is missing => use the pixel coords as a fallback 
		var realWorldCoords = {min_x: 0, max_x: this.x - 1, min_y: 0, max_y: this.y - 1, min_z: 0, max_z: this.max_slices};
		
		if (this.worldCoordinatesTransformationMatrix) {
			var tmpTranslatedCoords = this.getWorldCoordinatesForPixel({x:0, y:0, z:0});
			realWorldCoords.min_x = tmpTranslatedCoords.x;
			realWorldCoords.max_y = tmpTranslatedCoords.y;
			realWorldCoords.min_z = tmpTranslatedCoords.z;
			tmpTranslatedCoords = this.getWorldCoordinatesForPixel({x:this.x -1, y:this.y -1, z:this.max_slices});
			realWorldCoords.max_x = tmpTranslatedCoords.x;
			realWorldCoords.min_y = tmpTranslatedCoords.y;
			realWorldCoords.max_z = tmpTranslatedCoords.z;
		}
		
		return realWorldCoords;
	}, getXYCoordinatesWithRespectToZoomLevel : function(coords) {
		if (!coords) {
			return;
		}
		
		// correct x and y according to their zoom level to get the 1:1 pixel Coordinates which can then be transformed
		if (this.zoom_level == 1) {
			coords.x = Math.floor(coords.x * (this.x / this.one_to_one_x));
			coords.y = Math.floor(coords.y * (this.y / this.one_to_one_y));
		} else {
			coords.x = Math.ceil(coords.x * (this.x / this.one_to_one_x));
			coords.y = Math.ceil(coords.y * (this.y / this.one_to_one_y));
		}
	
		return coords;
	}
};
