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
	}
};