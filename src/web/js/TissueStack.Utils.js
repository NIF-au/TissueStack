TissueStack.Utils = {
		forceWindowScrollY : -1,
		preventBrowserWindowScrollingWhenInCanvas : function() {
			$(window).scroll(function(event) {
				if(TissueStack.Utils.forceWindowScrollY != -1 && window.scrollY != TissueStack.Utils.forceWindowScrollY) {
					$(window).scrollTop(TissueStack.Utils.forceWindowScrollY);    
				}
			});
		},
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
		},
	returnFirstOccurranceOfPatternInStringArray : function(someArray, pattern) {
		if (!someArray) {
			return;
		}
		
		for (var i=0 ; i<someArray.length;i++) {
			if (someArray[i].match(pattern)) {
				return someArray[i];
			}
		}
	},
	transformPixelCoordinatesToWorldCoordinates : function(pixelVector, transformationMatrix) {
		// not everything we need has been supplied
		if (!pixelVector || !transformationMatrix || pixelVector.length == 0 || transformationMatrix.length == 0) {
			return;
		}
		// lengths need to match
		if (pixelVector.length != transformationMatrix.length) {
			return;
		}
		
		// use sylvester library since we also want to go the opposite way which requires the inverse which I don't want to code.
		transformationMatrix = Matrix.create(transformationMatrix);
		pixelVector = Vector.create(pixelVector);
		
		// multiply to get results
		var results = transformationMatrix.multiply(pixelVector);
		if (results == null || results.elements.length != 4) {
			return;
		}
		
		results = [results.elements[0], results.elements[1], results.elements[2]];
		
		return results;
	},
	transformWorldCoordinatesToPixelCoordinates : function(worldVector, transformationMatrix) {
		// not everything we need has been supplied
		if (!worldVector || !transformationMatrix || worldVector.length == 0 || transformationMatrix.length == 0) {
			return;
		}
		// lengths need to match
		if (worldVector.length != transformationMatrix.length) {
			return;
		}

		// use sylvester library since we also want to go the opposite way which requires the inverse which I don't want to code.
		transformationMatrix = Matrix.create(transformationMatrix).inverse();
		worldVector = Vector.create(worldVector);
		
		// multiply to get results
		var results = transformationMatrix.multiply(worldVector);
		if (results == null || results.elements.length != 4) {
			return;
		}
		
		results = [results.elements[0], results.elements[1], results.elements[2]];
		
		return results;
	},indexColorMaps : function() {
		if (!TissueStack.color_maps) {
			return;
		}
		
		for (var map in TissueStack.color_maps) {
			if (!map || map === 'grey') {
				continue;
			}
			
			// create new array
			TissueStack.indexed_color_maps[map] = [];
			var index = 0;
			
			// we precompute the desired rgb for every byte value in the 256 range
			// Note: 255 is reserved for no data value
			for ( var y = 1; y < TissueStack.color_maps[map].length; y++) {
				// find start and end of value range
				var valueRangeStart = Math.floor(TissueStack.color_maps[map][y-1][0] * 255);
				var valueRangeEnd = Math.floor(TissueStack.color_maps[map][y][0] * 255);
				var valueRangeDelta = valueRangeEnd - valueRangeStart;
				// find start and end of RGB range
				var redRangeStart = TissueStack.color_maps[map][y-1][1];
				var redRangeEnd = TissueStack.color_maps[map][y][1];
				var greenRangeStart = TissueStack.color_maps[map][y-1][2];
				var greenRangeEnd = TissueStack.color_maps[map][y][2];
				var blueRangeStart = TissueStack.color_maps[map][y-1][3];
				var blueRangeEnd = TissueStack.color_maps[map][y][3];
				// compute deltas to get values in between color ranges 
				var redDelta = (redRangeEnd - redRangeStart) / valueRangeDelta;
				var greenDelta = (greenRangeEnd - greenRangeStart) / valueRangeDelta;
				var blueDelta = (blueRangeEnd - blueRangeStart) / valueRangeDelta;

				// compute and store associated RGB values for byte value
				var endOfLoop = index + valueRangeDelta;
				for (; index < endOfLoop ; index++) {
					// create RGB value array for byte value
					TissueStack.indexed_color_maps[map][index] = [];
					
					// set new red value
					TissueStack.indexed_color_maps[map][index][0] = Math.round((index * redRangeStart) + ((redRangeEnd - index) * redDelta));
					// set new green value
					TissueStack.indexed_color_maps[map][index][1] = Math.round((index * greenRangeStart) + ((greenRangeEnd - index) * greenDelta));
					// set new blue value
					TissueStack.indexed_color_maps[map][index][2] = Math.round((index * blueRangeStart) + ((blueRangeEnd - index) * blueDelta));
				} 
				
			}
			// no data: 255
			TissueStack.indexed_color_maps[map][255] = [255, 255, 255];
		}
	},adjustScreenContentToActualScreenSize : function (){	
		if (!TissueStack || typeof(TissueStack.phone) === 'undefined' || TissueStack.phone) {
			console.info("return 1");
			return;
		}
		
		var screenWidth = $(document).width();
		var screenHeight = $(document).	height();

		//$('.ui-slider-input').css('-webkit-transform','rotate('+270+'deg)')
		//$('.ui-slider-vertical').css({"height": Math.floor(screenHeight * 0.68)});	
		$('.canvaslocate, .canvaslocate *').css({"width" : Math.floor(screenWidth * 0.70), "height" : Math.floor(screenHeight * 0.75)});
		$('canvas').attr("width", Math.floor(screenWidth * 0.70));
		$('canvas').attr("height", Math.floor(screenHeight * 0.75));
		$('.coords_show_left').css({"width" : Math.floor(screenWidth * 0.20)});
	
	}
};