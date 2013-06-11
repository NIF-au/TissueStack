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
	}, loadColorMaps : function() {
		TissueStack.Utils.sendAjaxRequest(
				"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/colormaps/all", 'GET', true,
				function(data, textStatus, jqXHR) {
					if (!data) {
						alert("Failed To Load Colormaps ....");
						return;
					}
					
					TissueStack.configuration['color_maps'].description = "Colormaps Loaded From BackEnd";
					TissueStack.configuration['color_maps'].value = data;
					TissueStack.Utils.indexColorMaps();
				},
				function(jqXHR, textStatus, errorThrown) {
					alert("Error connecting to backend: " + textStatus + " " + errorThrown);
				}
			);
	},indexColorMaps : function() {
		var db_color_map = TissueStack.configuration['color_maps'];
		if (!db_color_map || !db_color_map.value) {
			return;
		}
		
		if (typeof(db_color_map.value) == 'string') 
			TissueStack.color_maps = $.parseJSON( db_color_map.value);
		else 
			TissueStack.color_maps = db_color_map.value;
				
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
				var valueRangeStart = Math.ceil(TissueStack.color_maps[map][y-1][0] * 255);
				var valueRangeEnd = Math.ceil(TissueStack.color_maps[map][y][0] * 255);
				var valueRangeDelta = valueRangeEnd - valueRangeStart;
				// find start and end of RGB range
				var redRangeStart = TissueStack.color_maps[map][y-1][1];
				var redRangeEnd = TissueStack.color_maps[map][y][1];
				var greenRangeStart = TissueStack.color_maps[map][y-1][2];
				var greenRangeEnd = TissueStack.color_maps[map][y][2];
				var blueRangeStart = TissueStack.color_maps[map][y-1][3];
				var blueRangeEnd = TissueStack.color_maps[map][y][3];
				// compute deltas to get values in between color ranges 
				var redDelta = Math.round((redRangeEnd - redRangeStart) / valueRangeDelta);
				var greenDelta = Math.round((greenRangeEnd - greenRangeStart) / valueRangeDelta);
				var blueDelta = Math.round((blueRangeEnd - blueRangeStart) / valueRangeDelta);
				if (valueRangeDelta == 255) {
				  redDelta = Math.round((redRangeEnd - redRangeStart)) * valueRangeDelta;
				  greenDelta = Math.round((greenRangeEnd - greenRangeStart)) * valueRangeDelta;
				  blueDelta = Math.round((blueRangeEnd - blueRangeStart)) * valueRangeDelta;
				}

				// compute and store associated RGB values for byte value
				var endOfLoop = index + valueRangeDelta;
				for (; index < endOfLoop ; index++) {
					// create RGB value array for byte value
					TissueStack.indexed_color_maps[map][index] = [];

					if (valueRangeDelta == 255) {
						// set new red value
						TissueStack.indexed_color_maps[map][index][0] = Math.abs(Math.round((index * redRangeStart) + ((redRangeEnd - index) * redDelta))) / 255;
						// set new green value
						TissueStack.indexed_color_maps[map][index][1] = Math.abs(Math.round((index * greenRangeStart) + ((greenRangeEnd - index) * greenDelta))) / 255;
						// set new blue value
						TissueStack.indexed_color_maps[map][index][2] = Math.abs(Math.round((index * blueRangeStart) + ((blueRangeEnd - index) * blueDelta))) / 255;
					} else {
						// set new red value
						TissueStack.indexed_color_maps[map][index][0] = Math.round((index * redRangeStart) + ((redRangeEnd - index) * redDelta));
						// set new green value
						TissueStack.indexed_color_maps[map][index][1] = Math.round((index * greenRangeStart) + ((greenRangeEnd - index) * greenDelta));
						// set new blue value
						TissueStack.indexed_color_maps[map][index][2] = Math.round((index * blueRangeStart) + ((blueRangeEnd - index) * blueDelta));
					}
				} 
				
			}
			// no data: 255
			TissueStack.indexed_color_maps[map][255] = [255, 255, 255];
		}
	}, updateColorMapChooser : function() {
		if (typeof(TissueStack.indexed_color_maps) != 'object')
			$(".color_map_select").html("<option>N/A</option>");

		var html = "";
			
		if(TissueStack.desktop || TissueStack.tablet){
			for (var c in TissueStack.indexed_color_maps)
				html += ("<option>" + c + "</option>");		
		}
		
		if(TissueStack.phone){
			for (var c in TissueStack.indexed_color_maps)
				html += ('<input type="radio" name="color_map" id="colormap_'+ c + '" value="'+ c +'"/>'
					 +  '<label for="colormap_' + c +'">' + c + '</label>');
			$(".color_map_select").html(html);
			return;
		}
		
		$(".color_map_select").html(html);
		//$(".color_map_select").selectmenu("refresh");	
		
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
		
		// clamp to max
		if (datasets > 2) datasets = 2;

		// we hide everything if there are no data sets selected
		if (datasets == 0) {
		   // clear input fields
		   $("#canvas_point_x,#canvas_point_y,#canvas_point_z,#canvas_point_value").attr("value", "");
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
		var widthTolerance = Math.floor(screenWidth * 0.01);
		var heightTolerance = Math.floor(screenHeight * 0.01);
		
		// get the width of the left panel 
		var leftPanelWidth = Math.floor(screenWidth * 0.15);
		var leftPanelHeight = screenHeight - menuHeight;
		var rightPanelWidth = Math.floor(screenWidth * 0.05);
		
		TissueStack.canvasDimensions = {width: (screenWidth - leftPanelWidth - rightPanelWidth - widthTolerance), height: Math.floor(leftPanelHeight / (TissueStack.overlay_datasets ? 1 : datasets)) -  heightTolerance};		
		//leftPanelHeight -=  heightTolerance;
		
		//$('.left_panel').css({"width" : leftPanelWidth, "height": leftPanelHeight});
		$('.left_panel').css({"left" : 0, "top" : menuHeight,"width" : leftPanelWidth, "height": leftPanelHeight * 0.99});
		$('#dataset_1_right_panel').css({"left" : TissueStack.canvasDimensions.width + leftPanelWidth + 20, "top" : menuHeight});
		$('#dataset_1_right_panel').css({"width" : rightPanelWidth, "height": TissueStack.canvasDimensions.height  * 0.99});
		if (TissueStack.overlay_datasets) {
			$('#dataset_2_right_panel').css({"left" : TissueStack.canvasDimensions.width + leftPanelWidth + 20, "top" : menuHeight});
			$('#dataset_2_right_panel').css({"width" : rightPanelWidth, "height": TissueStack.canvasDimensions.height  * 0.99});
		}		
		var sliderLength = (TissueStack.canvasDimensions.height - $('.canvasslider').outerHeight()) * 0.99;
		$(".ui-slider-vertical").height(sliderLength);
		$(".ui-slider-horizontal").height(sliderLength);

		$('#dataset_1').css({"left" : leftPanelWidth + 10, "top" : menuHeight});
		if (TissueStack.overlay_datasets)
			$('#dataset_2').css({"left" : leftPanelWidth + 10, "top" : menuHeight});
		$('.dataset').css({"width" : TissueStack.canvasDimensions.width, "height" : TissueStack.canvasDimensions.height * 0.99});
		
		for (var x=1;x<=datasets;x++) {
			if (!TissueStack.overlay_datasets && x>1) { 
				$('#dataset_' + x).css({"left" : leftPanelWidth + 10, "top" : menuHeight + 10 + TissueStack.canvasDimensions.height * 0.99});
				$('#dataset_' + x + '_right_panel').css({"left" : TissueStack.canvasDimensions.width + leftPanelWidth + 20, "top" : menuHeight + 10 + TissueStack.canvasDimensions.height * 0.99});
			}
			$('#dataset_' + x + '_right_panel').css({"width" : rightPanelWidth, "height": sliderLength});
			$("#dataset_" + x + "_toolbox_canvas").css({"width" : TissueStack.canvasDimensions.width * 0.8, "height" : 75});
			$("#dataset_" + x + "_contrast_box").css({"width" : TissueStack.canvasDimensions.width * 0.8, "height" : 55});
			$("#dataset_" + x + "_toolbox_canvas").attr("width", TissueStack.canvasDimensions.width * 0.8);
			$("#dataset_" + x + "_toolbox_canvas").attr("height", 75);
			$("#dataset_" + x + "_contrast_box").attr({"width" : TissueStack.canvasDimensions.width * 0.8});
			$("#dataset_" + x + "_contrast_box").attr({"height" : 75});
			if (TissueStack.dataSetNavigation.selectedDataSets["dataset_" + x]) {
				var ds = TissueStack.dataSetStore.getDataSetById(TissueStack.dataSetNavigation.selectedDataSets["dataset_" + x]);
				if (ds && ds.planes) {
					for (var p in ds.planes) {
						// use the first we can get and init the contrast slider
						if (ds.planes[p].contrast) ds.planes[p].contrast.initContrastSlider(); 
						break;
					}
				}
			}; 
			// add crosshair canvases programmatically (if it does not exist yet
			var crosshairCanvas = $('#dataset_' + x + '_main_view_canvas .cross_overlay');
			if (!crosshairCanvas || (crosshairCanvas && crosshairCanvas.length == 0))
				$('#dataset_' + x + '_main_view_canvas').append('<canvas id="dataset_' + x + '_canvas_y_plane_cross_overlay" class="cross_overlay"></canvas>');
			crosshairCanvas = $('#dataset_' + x + '_left_side_view_canvas .side_canvas_cross_overlay');
			if (!crosshairCanvas || (crosshairCanvas && crosshairCanvas.length == 0))
				$('#dataset_' + x + '_left_side_view_canvas').append('<canvas id="dataset_' + x + '_canvas_x_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>');
			crosshairCanvas = $('#dataset_' + x + '_right_side_view_canvas .side_canvas_cross_overlay');
			if (!crosshairCanvas || (crosshairCanvas && crosshairCanvas.length == 0))
				$('#dataset_' + x + '_right_side_view_canvas').append('<canvas id="dataset_' + x + '_canvas_z_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>');

			TissueStack.dataSetNavigation.handleOverlaidDataSets(x, TissueStack.overlay_datasets && datasets>1);
			
			$('#dataset_' + x + '_main_view_canvas').css({"width" : TissueStack.canvasDimensions.width, "height" : TissueStack.canvasDimensions.height * 0.99});
			$('#dataset_' + x + '_main_view_canvas canvas').attr("width", TissueStack.canvasDimensions.width);
			$('#dataset_' + x + '_main_view_canvas canvas').attr("height", TissueStack.canvasDimensions.height * 0.99);
		}

		// apply screen and canvas size changes
		var sideCanvasDims = {width: Math.floor(TissueStack.canvasDimensions.width * 0.3), height: Math.floor(TissueStack.canvasDimensions.height * 0.2)};
		$('.left_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.right_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.left_side_view canvas').attr("width", sideCanvasDims.width);
		$('.left_side_view canvas').attr("height", sideCanvasDims.height);
		$('.right_side_view canvas').attr("width", sideCanvasDims.width);
		$('.right_side_view canvas').attr("height", sideCanvasDims.height);
		
		var treeHeight = $('.left_panel').height();
		$('.left_panel').children().each(function() {
	      if ($(this).attr('id') != 'treedataset' && $(this).css('display') != 'none') 
	      treeHeight -= $(this).outerHeight();
	    });
		$('#treedataset').css({"height": treeHeight - 50});
		// apply scroll screen for admin upload direcory
		$('.settings-right-column, .settings-left-column').css({"height": screenHeight/1.7});
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
	    var validUrlRules = /^(http:\/\/|https:\/\/|ftp:\/\/|www.){1}([0-9A-Za-z\-]+\.)/;
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
			zoom, plane, slice, colormap, image_extension, tile_size, row, col) {
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
		if (typeof(colormap) != "string") {
			colormap = "grey";
		}
		if (typeof(image_extension) != "string") {
			image_extension = "png";
		}

		// assemble what we have so far
		var url = (host != "" ? (protocol + "://" + host.replace(/[_]/g,".")) : "");
		var path = isTiled ? TissueStack.configuration['tile_directory'].value : TissueStack.configuration['image_service_proxy_path'].value;

		if (zoom == 1) slice = Math.floor(slice);
		else slice = Math.ceil(slice);
		
		if (isTiled) {
			url += ("/" + path + "/" + dataset_id + "/" + zoom + "/" + plane + "/" + slice + "/");

			// for preview we don't need all the params 
			if (is_preview) {
				return url + slice + ".low.res." + image_extension;
			}

			// for tiling we need the row/col pair in the grid
			if (typeof(row) != "number" || row < 0) {
				return null;
			}
			if (typeof(col) != "number" || col < 0) {
				return null;
			}

			return url + row + '_' + col + "." + image_extension + "?" + new Date().getTime();
		} else {
			// seems to work for server so why not use it
		    url = url + "/" + path + "/?volume=" + filename + "&image_type=JPEG&scale=" + zoom + "&dimension="
		    	+ plane + "space" + "&slice=" + slice + "&colormap=" + colormap;
			
			if (is_preview) {
				return url + "&quality=8";
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
			timeout : 60000,
			dataType : "json",
			success: success,
			error: error
		});
	}, generateSessionId : function() {
		var timestampPart = "" + new Date().getTime();
		var randomPart = Math.floor((Math.random()*100));
		
		return timestampPart + randomPart;
	}, readQueryStringFromAddressBar : function() {
		if (!document.location.search || document.location.search.length == 0 || document.location.search === '?') {
			return null;
		}
		
		// prepare query string
		var queryString = $.trim(document.location.search);
		// omit '?'
		var queryStringStart = queryString.lastIndexOf('?');
		if (queryStringStart >= 0) queryString = queryString.substring(queryStringStart + 1);
		// do a URIdecode
		queryString = decodeURIComponent(queryString);
		//extract potential params by splitting into tokens delimited by '&'
		var tokens = queryString.split('&');
		if (!tokens || tokens.length == 0) return null;
		
		// potential args
		var args = ['ds','plane', 'x', 'y', 'z', 'zoom', 'color', 'min', 'max'];
		var ret = {}; // return object

		var c = 0;
		for (var i=0;i<tokens.length; i++) {
			for (var j=0;j<args.length;j++) {
				var index = tokens[i].indexOf(args[j] + '=');
				if (index ==0) {
					if (args[j] === 'plane' || args[j] === 'color') ret[args[j]] = tokens[i].substring(args[j].length + 1);
					else ret[args[j]] = parseFloat(tokens[i].substring(args[j].length + 1));
					c++;
				}
			}			
		}

		return c == 0 ? null : ret;
	}, getResolutionString : function(resolution_in_mm_times_scale_bar_width_in_pixels) {
		if (typeof(resolution_in_mm_times_scale_bar_width_in_pixels) != 'number') return "NaN";
		if (resolution_in_mm_times_scale_bar_width_in_pixels < 0) return "negative value";
		
		var unit_lookup = ['mm', '&micro;m', 'nm'];
		var unit_step = 0;
		var newRes = resolution_in_mm_times_scale_bar_width_in_pixels;
		// start at mm and see if we need to go smaller
		while (true) {
			if (newRes == 0 || Math.floor(newRes) > 0 || unit_step + 1 > unit_lookup.length - 1) break;
			
			newRes *= 1000;
			unit_step++;
		}

		return ((newRes - Math.floor(newRes) > 0.00001) ? newRes.toFixed(3) : newRes) + '&nbsp;' + unit_lookup[unit_step];
	}, swapOverlayAndUnderlyingCanvasPlanes : function(dataset, plane1, plane2, recursive_call) {
		// only for sync but not in combination with overlay
		if (!TissueStack.sync_datasets) return;
			
		// prelim checks of existence
		if (typeof(dataset) != 'object' || !dataset.planes || typeof(plane1) != 'string' || typeof(plane2) != 'string') return;
		
		var plane1Found = dataset.planes[plane1];
		var plane2Found = dataset.planes[plane2];
		
		// both planes need to be found in the dataset
		if (!plane1Found || !plane2Found) return;

		// neither underlying canvas nor overlay exists => nothing to do => exit 
		if (!plane1Found.underlying_canvas && !plane1Found.overlay_canvas
				&& !plane2Found.underlying_canvas && !plane2Found.overlay_canvas) return;
		
		var whichEverCanvas1 = plane1Found.underlying_canvas ? plane1Found.underlying_canvas : plane1Found.overlay_canvas;
		var whichEverCanvas2 = plane2Found.underlying_canvas ? plane2Found.underlying_canvas : plane2Found.overlay_canvas;
		
		// swap the 2 
		var tmp = whichEverCanvas1;
		whichEverCanvas1 = whichEverCanvas2;
		whichEverCanvas2 = tmp;
		
		if (plane1Found.underlying_canvas) {
			plane1Found.underlying_canvas = whichEverCanvas1;
			plane2Found.underlying_canvas = whichEverCanvas2;
		} else {
			plane1Found.overlay_canvas = whichEverCanvas1;
			plane2Found.overlay_canvas = whichEverCanvas2;
		}

		// so that we don't enter into an infinite recursion after our explicit second call (see below)
		if (recursive_call) return;

		// call ourselves one more time for the other bidirectional end (overlay/underlying)
		TissueStack.Utils.swapOverlayAndUnderlyingCanvasPlanes(
				TissueStack.dataSetStore.getDataSetById(
						TissueStack.dataSetNavigation.selectedDataSets[whichEverCanvas1.dataset_id]),
				plane1, plane2, true);
	},transitionToDataSetView :  function() {
    	if (TissueStack.dataSetNavigation.selectedDataSets.count > 0) {
    		var sel = TissueStack.dataSetNavigation.selectedDataSets["dataset_1"];
    		window.location.hash = '#data';
    		TissueStack.dataSetNavigation.getDynaTreeObject().selectKey(sel, false); 
    		setTimeout(function() {
    			TissueStack.dataSetNavigation.getDynaTreeObject().selectKey(sel, true);
        		TissueStack.swappedOverlayOrder = false;
    		}, 250);
    	}
	}
};
