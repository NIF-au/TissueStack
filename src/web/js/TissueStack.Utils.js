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
				"/" + TissueStack.configuration['server_proxy_path'].value + "/?service=services&sub_service=colormaps&action=all", 'GET', true,
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

			// for continuous lookup
			var valueRangeRow = 1;
			var offsetRGB = [
			                 TissueStack.color_maps[map][0][1],
			                 TissueStack.color_maps[map][0][2],
			                 TissueStack.color_maps[map][0][3]
			                 ];
			var valueRangeStart = TissueStack.color_maps[map][valueRangeRow-1][0] * 255;
			var valueRangeEnd = TissueStack.color_maps[map][valueRangeRow][0] * 255;

			for (var index = 0; index < 256 ; index++) {
				// create RGB value array for byte value
				TissueStack.indexed_color_maps[map][index] = [];

				// check if we have a discrete lookup to begin with, we identify it because the array length is 5
				if (TissueStack.color_maps[map][0].length == 5) {
					// set up RGB mapping
					var grayValueToBeMapped =
						(index > TissueStack.color_maps[map].length-1) ? index : TissueStack.color_maps[map][index][1];

					TissueStack.indexed_color_maps[map][grayValueToBeMapped][0] =
						(index > TissueStack.color_maps[map].length-1) ?
								grayValueToBeMapped : TissueStack.color_maps[map][index][2];
					TissueStack.indexed_color_maps[map][grayValueToBeMapped][1] =
						(index > TissueStack.color_maps[map].length-1) ?
								grayValueToBeMapped : TissueStack.color_maps[map][index][3];
					TissueStack.indexed_color_maps[map][grayValueToBeMapped][2] =
						(index > TissueStack.color_maps[map].length-1) ?
								grayValueToBeMapped : TissueStack.color_maps[map][index][4];

					continue;
				}

				// continuous to discrete mapping
				// first check if we are within desired range up the present end,
				// if not => increment row index to move on to next range
				if (index > valueRangeEnd) {
					valueRangeRow++;
					valueRangeStart = valueRangeEnd;
					valueRangeEnd = TissueStack.color_maps[map][valueRangeRow][0] * 255;
					offsetRGB[0] = TissueStack.color_maps[map][valueRangeRow-1][1];
					offsetRGB[1] = TissueStack.color_maps[map][valueRangeRow-1][2];
					offsetRGB[2] = TissueStack.color_maps[map][valueRangeRow-1][3];
				}
				var valueRangeDelta = valueRangeEnd - valueRangeStart;

				// iterate over RGB channels
				for (var rgb = 1; rgb < 4 ; rgb++) {
					var rgbRangeStart = TissueStack.color_maps[map][valueRangeRow-1][rgb];
					var rgbRangeEnd = TissueStack.color_maps[map][valueRangeRow][rgb];
					var rgbRangeDelta = rgbRangeEnd - rgbRangeStart;
					var rangeRemainder = index % valueRangeDelta;
					if (rangeRemainder == 0 && rgbRangeDelta != 0 && rgbRangeDelta != 1 && valueRangeEnd == 255) {
						offsetRGB[rgb-1] += rgbRangeDelta;
					} else if (rangeRemainder == 0 && rgbRangeDelta != 0 && valueRangeEnd == 255 && index == 255) {
						offsetRGB[rgb - 1] += rgbRangeDelta;
					}
					var rangeRatio = (rgbRangeDelta * rangeRemainder / valueRangeDelta) + offsetRGB[rgb-1];

					TissueStack.indexed_color_maps[map][index][rgb-1] =	Math.round(rangeRatio * 255);
				}
			}

		}
	}, updateColorMapChooser : function() {
		if (typeof(TissueStack.indexed_color_maps) != 'object')
			$(".color_map_select").html("<option>N/A</option>");

		var html = "";

		if(TissueStack.phone){
			for (var c in TissueStack.indexed_color_maps)
				html += ('<input type="radio" name="color_map" id="colormap_'+ c + '" value="'+ c +'"/>'
					 +  '<label for="colormap_' + c +'">' + c + '</label>');
			$(".color_map_select").html(html);
			return;
		} else {
			for (var c in TissueStack.indexed_color_maps)
				html += ("<option>" + c + "</option>");
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

		// we hide everything if there are no data sets selected
		if (datasets == 0) {
            // clear input fields
            $("#canvas_point_x,#canvas_point_y,#canvas_point_z,#canvas_point_value").attr("value", "");
            $("#canvas_point_x,#canvas_point_y,#canvas_point_z").attr("disabled", "disabled");
            // hide everything
		    $('#dataset_1_center_point_in_canvas, #dataset_2_center_point_in_canvas').closest('.ui-btn').hide();
		    $(".dataset, .right_panel").addClass("hidden");
			try {
			    $("#ontology_tree_div").collapsible("collapse");
				$("#coords_collapsible").collapsible("collapse");
			} catch(ignored) {}

		    return;
		} else {
			try {
				$("#coords_collapsible").collapsible("expand");
			} catch(ignored) {}
		}

        TissueStack.Utils.captureScreenDimensions(datasets);
		//leftPanelHeight -=  heightTolerance;

		//$('.left_panel').css({"width" : TissueStack.canvasDimensions.leftPanelWidth, "height": TissueStack.canvasDimensions.leftPanelWidth});
		$('.left_panel').css({"left" : 0,
                              "top" : TissueStack.canvasDimensions.menuHeight,
                              "width" : TissueStack.canvasDimensions.leftPanelWidth,
                              "height": TissueStack.canvasDimensions.leftPanelHeight * 0.99});
		var sliderLength = (TissueStack.canvasDimensions.height - $('.canvasslider').outerHeight()) * 0.99;
		$('.dataset').css({"width" : TissueStack.canvasDimensions.width, "height" : TissueStack.canvasDimensions.height * 0.99});

		for (var x=1;x<=datasets;x++) {
            if (x == 1) {
                $('#dataset_' + x).css({"left" : TissueStack.canvasDimensions.leftPanelWidth + 10, "top" : TissueStack.canvasDimensions.menuHeight});
                $('#dataset_' + x + '_right_panel').css({"left" : TissueStack.canvasDimensions.width + TissueStack.canvasDimensions.leftPanelWidth + 20,
                                         "top" : TissueStack.canvasDimensions.menuHeight});
                $('#dataset_' + x + '_right_panel').css({"width" : TissueStack.canvasDimensions.rightPanelWidth,
                                         "height": TissueStack.canvasDimensions.height  * 0.99});
            } else if (!TissueStack.overlay_datasets && x>1) {
				$('#dataset_' + x).css({"left" : TissueStack.canvasDimensions.leftPanelWidth + 10,
                                        "top" : TissueStack.canvasDimensions.menuHeight + 10 + TissueStack.canvasDimensions.height * 0.99});
				$('#dataset_' + x + '_right_panel').css({"left" : TissueStack.canvasDimensions.width + TissueStack.canvasDimensions.leftPanelWidth + 20,
                                                         "top" : TissueStack.canvasDimensions.menuHeight + 10 + TissueStack.canvasDimensions.height * 0.99});
			} else if (TissueStack.overlay_datasets) {
                $('#dataset_' + x).css({"left" : TissueStack.canvasDimensions.leftPanelWidth + 10, "top" : TissueStack.canvasDimensions.menuHeight});
                $('#dataset_' + x + '_right_panel').css({"left" : TissueStack.canvasDimensions.width + TissueStack.canvasDimensions.leftPanelWidth + 20,
                                             "top" : TissueStack.canvasDimensions.menuHeight});
                $('#dataset_' + x + '_right_panel').css({"width" : TissueStack.canvasDimensions.rightPanelWidth,
                                             "height": TissueStack.canvasDimensions.height  * 0.99});
            }

			$('#dataset_' + x + '_right_panel').css({"width" : TissueStack.canvasDimensions.rightPanelWidth, "height": sliderLength});
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

            var div = "dataset_" + x;
            TissueStack.ComponentFactory.resizeProgressBar(div);
            TissueStack.ComponentFactory.resizeDataSetSlider(TissueStack.canvasDimensions.height);
		}

		// apply screen and canvas size changes
		var sideCanvasDims = {width: Math.floor(TissueStack.canvasDimensions.width * 0.3), height: Math.floor(TissueStack.canvasDimensions.height * 0.2)};
		$('.left_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.right_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('.left_side_view canvas').attr("width", sideCanvasDims.width);
		$('.left_side_view canvas').attr("height", sideCanvasDims.height);
		$('.right_side_view canvas').attr("width", sideCanvasDims.width);
		$('.right_side_view canvas').attr("height", sideCanvasDims.height);

		// adjust Tree Height
		if (TissueStack.desktop) {
			//TissueStack.Utils.adjustCollapsibleSectionsHeight('ontology_tree');
			TissueStack.Utils.adjustCollapsibleSectionsHeight('treedataset');
		}
		else TissueStack.Utils.adjustCollapsibleSectionsHeight('menutransition');

		// apply scroll screen for admin upload direcory
		$('.settings-right-column, .settings-left-column').css({"height": TissueStack.canvasDimensions.screenHeight/1.7});
	}, adjustCollapsibleSectionsHeight : function(elem_id, upper_limit) {
		if (typeof(elem_id) != 'string') return;
		if (!$("#" + elem_id) || typeof($("#" + elem_id).length) != 'number' || $("#" + elem_id).length == 0) return;

		var treeHeight = $('.left_panel').height();
		var elCount = 0;
		$('.left_panel').children().each(function() {
			  elCount++;
	      if ($(this).attr('id') != elem_id && $(this).attr('id') != elem_id + '_div' && $(this).css('display') != 'none') {
	    	  treeHeight -= $(this).outerHeight();
	      }
	    });

	    var finalHeight = treeHeight - $("#" + elem_id + "_div .ui-collapsible-heading").outerHeight() - elCount*10;
	    if (finalHeight <=0) // sanity check for minimal height
            finalHeight = 150;

	    if (typeof(upper_limit) != 'number' || upper_limit <= 0 || upper_limit >= finalHeight)
			$('#' + elem_id).css({"height": finalHeight});
		else
			$('#' + elem_id).css({"height": upper_limit});
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
        dataSet, canvas, is_preview, colormap, row, col) {
        var ret = { url: "", cache_key: null};

        var host = dataSet.host;
		if (typeof(host) != "string" || $.trim(host) == "localhost")
			host = "";
		host = $.trim(host);

        var dataset_id = dataSet.local_id;
        var isTiled = canvas.getDataExtent().getIsTiled();
		if (typeof(isTiled) != "boolean")
			isTiled = false; // if in doubt => false

        var filename = dataSet.filename;
		if (typeof(is_preview) != "boolean")
			is_preview = false;

        var tile_size = canvas.getDataExtent().tile_size;
		if (!is_preview && (typeof(tile_size) != "number" || zoom < 0))
			tile_size = 256;

        var zoom =
            canvas.getDataExtent().getIsTiled() ?
                canvas.getDataExtent().zoom_level :
                canvas.getDataExtent().getZoomLevelFactorForZoomLevel(canvas.getDataExtent().zoom_level);

		var plane = canvas.getDataExtent().getOriginalPlane();
		if (typeof(colormap) != "string")
			colormap = "grey";

        var image_extension = canvas.image_format;
		if (typeof(image_extension) != "string")
			image_extension = "png";

		// assemble what we have so far
		ret.url = (host != "" ? ("http://" + host.replace(/[_]/g,".")) : "");
		var path = isTiled ?
			TissueStack.configuration['tile_directory'].value : TissueStack.configuration['server_proxy_path'].value;

        var slice = canvas.getDataExtent().slice
		if (zoom == 1) slice = Math.floor(slice);
		else slice = Math.ceil(slice);

		if (isTiled) {
			ret.url += ("/" + path + "/" + dataset_id + "/" + zoom + "/" + plane + "/" + slice + "/");

			// for preview we don't need all the params
			if (is_preview) {
				return ret.url + slice + ".low.res." +
					((colormap == 'grey' || colormap == 'gray') ? "" : (colormap + ".")) + image_extension;
			}
			ret.url += row + '_' + col + ((colormap == 'grey' || colormap == 'gray') ?
                    "" : ("_" + colormap)) + "." + image_extension;
            return ret;
		} else {
            var min = canvas.contrast ? canvas.contrast.getMinimum() : 0;
            var max = canvas.contrast ? canvas.contrast.getMaximum() : 255;

            ret.cache_key =
                canvas.getDataExtent().zoom_level + "/" + slice + "/" +
                colormap + "/" + row + "/" + col+ "/" + min + "/" + max;

            ret.url += "/" + path + "/?service=";

		    if (is_preview)
		    	ret.url += "image_preview&quality=0.05";
		    else
		    	ret.url += "image" + "&square=" + tile_size + '&y=' + col + "&x=" + row
		    ret.url += "&dataset=" + filename + "&image_type=PNG&scale=" + zoom + "&dimension="
		    	+ plane + "&slice=" + slice + "&colormap=" + colormap;

            if (canvas.contrast &&
                (min != canvas.contrast.dataset_min ||
                 max != canvas.contrast.dataset_max)) {
                ret.url += ("&min=" + min);
                ret.url += ("&max=" + max);
            }
			return ret;
		}
	}, adjustBorderColorWhenMouseOver : function () {
			if (TissueStack.phone || TissueStack.tablet) {
				return;
			}

			$('.dataset').unbind('mouseover');
			$('.dataset').unbind('mouseout');
			$('.dataset').mouseover(function(){
				var id = $(this).attr('id');
				if (!id || id.length != "dataset_X".length) return;
				if (TissueStack.mouseOverDataSet == id) return;

				TissueStack.mouseOverDataSet = id;
				var dataSet = TissueStack.dataSetStore.getDataSetById(
								TissueStack.dataSetNavigation.selectedDataSets[id]);

				for (var p in dataSet.planes)
					if (dataSet.planes[p].is_main_view) {
						dataSet.planes[p].updateExtentInfo(dataSet.realWorldCoords[p]);
						dataSet.planes[p].events.updateCoordinateDisplay();
						break;
					}

			    $(this).css("border-color","#efff0b").css("border-width",1);
			    $('.left_panel').css("color","#efff0b").css("font-color","#efff0b");
			}).mouseout(function(){
				TissueStack.mouseOverDataSet = -1;
			   	$(this).css("border-color","white").css("border-width",1);
			   	$('.left_panel').css("color","white");
			});
	},sendAjaxRequest : function(url, method, async, success, error) {
		if (typeof(url) != "string" || $.trim(url) == '') return;
		if (typeof(method) != "string" || $.trim(method) == '') method = 'GET';

		async = (typeof(async) != "boolean" || async == true);

		if (typeof(success) != 'function') success = null;

		if (typeof(error) != 'function') error = null;

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
	},transitionToDataSetView :  function(reselect) {
        $.mobile.changePage("#data", {allowSamePageTransition: true});

        if (typeof reselect !== 'boolean') reselect = false;
        if (TissueStack.dataSetNavigation.selectedDataSets.count > 1 && reselect) {
            var sel = TissueStack.dataSetNavigation.selectedDataSets["dataset_1"];
            setTimeout(function() {
                TissueStack.dataSetNavigation.getDynaTreeObject().selectKey(sel, false);
                TissueStack.dataSetNavigation.getDynaTreeObject().selectKey(sel, true);
                TissueStack.swappedOverlayOrder = false;
            }, 250);
    	}
	}, testHttpFileExistence : function(url) {
		var succeeded = false;

		$.ajax({
			async : false,
			url : url,
			type : "HEAD",
			cache : false,
			timeout : 30000,
			success: function() {succeeded = true;}
		});

		return succeeded;
	}, queryVoxelValue : function(dataset, canvas, coords) {
		if (typeof(dataset) != 'object' || typeof(canvas) != 'object' || typeof(coords) != 'object') return null;

		// the original data set/file
		var files = dataset.filename;
		var planes = canvas.getDataExtent().plane;
		var slices = Math.round(coords.z);
        var host = dataset.host;
        if (typeof(host) != "string" || $.trim(host) == "localhost")
            host = "";
        host = ($.trim(host) != "" ? ("http://" + host.replace(/[_]/g,".")) : "");

		var xes = coords.x;
		var ys = coords.y;

	 	if (canvas.getDataExtent().one_to_one_x != canvas.getDataExtent().origX)
            xes *= ( canvas.getDataExtent().origX / canvas.getDataExtent().one_to_one_x);
	 	if (canvas.getDataExtent().one_to_one_y != canvas.getDataExtent().origY)
            ys *= ( canvas.getDataExtent().origY / canvas.getDataExtent().one_to_one_y);

		xes = xes;
		ys	= ys;

		// augment it with any associated data sets that need to be queried as well
		if (TissueStack.desktop)
			if (dataset.associatedDataSets && dataset.associatedDataSets.length > 0)
				for (i=0;i<dataset.associatedDataSets.length;i++) {
					files += (":" + dataset.associatedDataSets[i].associatedDataSet.filename);
				}

		// assemble url
		var url = host + "/" + TissueStack.configuration['server_proxy_path'].value +
         "/?service=query&dataset=" + files + "&dimension=" + planes +
          "space&slice=" + slices + "&x=" + xes + "&y=" + ys;

		// in case of an error we rely on the canvas pixel querying
		var errorHandler = function(canvas) {
			var value = canvas.getCanvasPixelValue({x: canvas.cross_x, y: canvas.cross_y});
			var value_wrapped = {};
			value_wrapped[dataset.filename] = value;
			canvas.displayPixelValue(dataset, TissueStack.dataSetStore.lookupValueForRGBTriple(dataset, value_wrapped));
		};

		if (canvas.hasColorMapOrContrastSetting()) {
			setTimeout(function() {	errorHandler(canvas); } , 200);
			return;
		}

  		// send ajax request
 		TissueStack.Utils.sendAjaxRequest(url, 'GET', true,
			function(data, textStatus, jqXHR) {
				if (!data.response && !data.error) {
					errorHandler(canvas);
					return;
				}
				if (data.error) {
					errorHandler(canvas);
					return;
				}

				// fall back if no response
				if (!data || !data.response || !data.response[dataset.filename]) {
					errorHandler(canvas);
					return;
				}

				var value =
					TissueStack.dataSetStore.lookupValueForRGBTriple(dataset, data.response);
				canvas.displayPixelValue(dataset, value);
			},
			function(jqXHR, textStatus, errorThrown) {
				errorHandler(canvas);
				return;
			}
		);
	},
    captureScreenDimensions : function(datasets) {
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

		TissueStack.canvasDimensions =
            { width: (screenWidth - leftPanelWidth - rightPanelWidth - widthTolerance),
              height: Math.floor(leftPanelHeight / (TissueStack.overlay_datasets ? 1 : datasets)) -  heightTolerance,
              menuHeight: menuHeight,
              leftPanelWidth: leftPanelWidth,
              leftPanelHeight: leftPanelHeight,
              rightPanelWidth: rightPanelWidth,
              screenHeight: screenHeight};
    },
    findMainCanvasInDataSet : function(dataSet) {
        if (typeof dataSet !== 'object' || typeof dataSet.planes !== 'object')
            return null;

        for (var c in dataSet.planes)
            if (dataSet.planes[c].is_main_view)
                return dataSet.planes[c];

        return null;
    }
};
