TissueStack.Utils = {
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
	}
};