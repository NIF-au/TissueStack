TissueStack.Utils = {
		supportsCanvas : function() {
			var elem = document.createElement('canvas');
			return !!(elem.getContext && elem.getContext('2d'));
		},		
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
		var db_color_map = TissueStack.configuration['color_maps'];
		if (!db_color_map || !db_color_map.value) {
			return;
		}
		
		if (typeof(db_color_map.value) != 'string') {
			return;
		}
		
		TissueStack.color_maps = $.parseJSON( db_color_map.value);
		
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
	},adjustScreenContentToActualScreenSize : function (datasets){	
		if (TissueStack.phone) {
			$('#canvasBox').css({"width" : 300, "height" : 300});
			$('.plane').attr("width", "300");
			$('.plane').attr("height", "300");
			return;
		}
		
		if (typeof(datasets) != "number") {
			datasets = 0;
		}
		
		if (datasets > 2) {
			datasets = 2;
		}

		// we hide everything if there are no data sets selected
		if (datasets == 0) {
		   // clear input fields
		   $("#canvas_point_x,#canvas_point_y,#canvas_point_z").attr("value", "");
		   $("#canvas_point_x,#canvas_point_y,#canvas_point_z").attr("disabled", "disabled");
		   // hide everything
		   $('#dataset_1_center_point_in_canvas, #dataset_2_center_point_in_canvas').closest('.ui-btn').hide();
		   $(".dataset, .right_panel").addClass("hidden");
		   return;
		}
		
		// get screen dimensions
		var screenWidth = $(window).width();
		var screenHeight = $(window).height();
		
		// get the height of the menu header
		var menuHeight = $('#menu_header').height();
		// define some tolerance span
		var widthTolerance = Math.floor(screenWidth * 0.05);
		var heightTolerance = Math.floor(screenWidth * 0.0175);
		
		// get the width of the left panel 
		var leftPanelWidth = Math.floor(screenWidth * 0.15);
		var leftPanelHeight = screenHeight - menuHeight;
		var rightPanelWidth = Math.floor(screenWidth * 0.05);
		
		TissueStack.canvasDimensions = {width: (screenWidth - leftPanelWidth - rightPanelWidth - widthTolerance), height: Math.floor(leftPanelHeight / datasets) -  heightTolerance};
		leftPanelHeight -=  heightTolerance;
		
		$('.left_panel').css({"width" : leftPanelWidth, "height": leftPanelHeight});
		$('.right_panel').css({"width" : rightPanelWidth, "height": TissueStack.canvasDimensions.height});
		$(".ui-slider-vertical").height(TissueStack.canvasDimensions.height - heightTolerance);
		$(".ui-slider-horizontal").height(TissueStack.canvasDimensions.height - heightTolerance);

		if (TissueStack.desktop) {
			var treeHeight = 
				leftPanelHeight - 
				$("#canvas_extent").height() - $("#canvas_point_x").height() * 8 - $("#dataset_1_center_point_in_canvas").height() * (datasets == 2 ? 5 : 4);
			$("#treedataset").css({"height": treeHeight});
		}

		$('.dataset').css({"width" : TissueStack.canvasDimensions.width, "height" : TissueStack.canvasDimensions.height});
		for (var x=1;x<=datasets;x++) {
			$('#dataset_' + x + '_main_view_canvas').css({"width" : TissueStack.canvasDimensions.width, "height" : TissueStack.canvasDimensions.height});
			$('#dataset_' + x + '_main_view_canvas canvas').attr("width", TissueStack.canvasDimensions.width);
			$('#dataset_' + x + '_main_view_canvas canvas').attr("height", TissueStack.canvasDimensions.height);
		}

		// apply screen and canvas size changes
		var sideCanvasDims = {width: Math.floor(TissueStack.canvasDimensions.width * 0.3), height: Math.floor(TissueStack.canvasDimensions.height * 0.2)};
		$('.left_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.right_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.left_side_view canvas').attr("width", sideCanvasDims.width);
		$('.left_side_view canvas').attr("height", sideCanvasDims.height);
		$('.right_side_view canvas').attr("width", sideCanvasDims.width);
		$('.right_side_view canvas').attr("height", sideCanvasDims.height);
	},
	verifyUrlSyntax : function(url) {
		if (typeof(url) != "string") {
			return null;
		}

		// trim whitespace
		url = $.trim(url);
		if (url.length == 0) {
			return null;
		}
		
		if (url.indexOf("http://") != 0 && url.indexOf("https://") != 0 && url.indexOf("ftp://") != 0) {
			url = "http://" + url;
		}
		
		// check
	    var validUrlRules = /^(http:\/\/|https:\/\/|ftp:\/\/|www.){1}([0-9A-Za-z]+\.)/;
	    if (validUrlRules.test(url) && url.substring(url.length-1) != ".") {
	        return url;
	    }

	    return null;
	},
	extractHostNameFromUrl : function(url) {
		if (typeof(url) != "string") {
			return null;
		}

		// trim whitespace
		url = $.trim(url);

		// take off protocol 
		if (url.indexOf("http://") == 0 || url.indexOf("file://") == 0) {
			url = url.substring(7, url.length);
		} else if (url.indexOf("https://") == 0 || url.indexOf("file:///") == 0) {
			url = url.substring(8, url.length);
		} else if (url.indexOf("ftp://") == 0) {
			url = url.substring(6, url.length);
		}
		// strip it off a potential www
		if (url.indexOf("www.") == 0) {
			url = url.substring(4, url.length);
		}
			
		//now cut off anything after the initial '/' if exists to preserve the domain
		var slashPosition = url.indexOf("/");
		if (slashPosition > 0) {
			url = url.substring(0, slashPosition);
		}

		return url;
	},
	assembleTissueStackImageRequest : function(
			protocol, host, isTiled, filename, dataset_id, is_preview,
			zoom, plane, slice, image_extension, tile_size, row, col) {
		if (typeof(protocol) != "string") {
			protocol = "http";
		} 
		protocol = $.trim(protocol);
		if (typeof(host) != "string" || $.trim(host) == "localhost") {
			host = "";
		}
		host = $.trim(host);
		if (typeof(dataset_id) != "number" || dataset_id <= 0) {
			return null;
		}
		//are we tiled or not
		if (typeof(isTiled) != "boolean") {
			isTiled = false; // if in doubt => false
		}
		if (!isTiled && typeof(filename) != "string") {
			return null;
		}
		if (typeof(is_preview) != "boolean") {
			return null;
		}
		if (!is_preview && (typeof(tile_size) != "number" || zoom < 0)) {
			tile_size = 256;
		}
		if (typeof(zoom) != "number" || zoom < 0) {
			return null;
		}
		if (typeof(plane) != "string") {
			return null;
		}
		if (typeof(slice) != "number" || slice < 0) {
			return null;
		}
		if (typeof(image_extension) != "string") {
			image_extension = "png";
		}

		// assemble what we have so far
		var url = (host != "" ? (protocol + "://" + host.replace(/[_]/g,".")) : "");
		var path = isTiled ? TissueStack.configuration['tile_directory'].value : TissueStack.configuration['image_service_directory'].value;

		if (isTiled) {
			url += ("/" + path + "/" + dataset_id + "/" + zoom + "/" + plane + "/" + slice);

			// for preview we don't need all the params 
			if (is_preview) {
				return url + ".low.res." + image_extension;
			}

			// for tiling we need the row/col pair in the grid
			if (typeof(row) != "number" || row < 0) {
				return null;
			}
			if (typeof(col) != "number" || col < 0) {
				return null;
			}

			return url + "/" + row + '_' + col + "." + image_extension;
		} else {
			// TODO: change back to apache rewrite when working
		    url = ("http://localhost:4242/?volume=" + filename + "&scale=" + zoom + "&dimension=" + plane + "space" + "&slice=" + slice);
			
			if (is_preview) {
				return url + "&quality=10";
			}
			
			return url  + "&quality=1&service=tiles&square=" + tile_size + '&y=' + col + "&x=" + row;
		}
	}, adjustBorderColorWhenMouseOver : function () {
			if (TissueStack.phone || TissueStack.tablet) {
				return;
			}
	
			$('.dataset').mouseover(function(){
			    $(this).css("border-color","#efff0b").css("border-width",1);
			    $('.left_panel').css("color","#efff0b").css("font-color","#efff0b");
			}).mouseout(function(){
			   	$(this).css("border-color","white").css("border-width",1);
			   	$('.left_panel').css("color","white");
			});
	},sendAjaxRequest : function(url, method, async, success, error) {
		if (typeof(url) != "string" || $.trim(url) == '') {
			return;
		}
		if (typeof(method) != "string" || $.trim(method) == '') {
			method = 'GET';
		}

		if (typeof(async) != "boolean" || async == true) {
			async = true;
		} else {
			async = false;
		}

		if (typeof(success) != 'function') {
			success = null;
		}

		if (typeof(error) != 'function') {
			error = null;
		}
		
		$.ajax({
			async : async,
			url : url,
			type : method,
			cache : false,
			timeout : 30000,
			dataType : "json",
			success: success,
			error: error
		});
	}, generateSessionId : function() {
		var timestampPart = "" + new Date().getTime();
		var randomPart = Math.floor((Math.random()*100000));
		
		return timestampPart + randomPart;
	}
};