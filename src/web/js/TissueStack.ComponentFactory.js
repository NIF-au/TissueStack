TissueStack.ComponentFactory = {
    checkDivExistence : function (div) {
        if (typeof (div) !== 'string' || $.trim(div) === '') return false;

        div = $("#" + div);
        if (div.length == 0) return false;

        return true;
    },
    checkDataSetObjectIntegrity : function(dataSet) {
        if (typeof(dataSet) != 'object') return false;

        // data member
        if (typeof(dataSet.data) != 'object' || typeof(dataSet.data.length) != 'number') return false;

        if (dataSet.data.length == 0) return false;

        return true;
    },
    checkDataSetOrdinal : function (ordinal, checkForExistence) {
        if (typeof (ordinal) !== 'number' || ordinal < 0) return false;

        if (typeof (checkForExistence) !== 'boolean' || !checkForExistence) return true;

        return !TissueStack.ComponentFactory.checkDivExistence("dataset_" + ordinal);
    },
    checkStandardInput : function(div, dataSet, dataSetOrdinal, checkForExistence) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::createDataSet => Given div id is invalid!");
            return false;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::createDataSet => Given dataSet object is invalid!");
            return false;
        }


        if (!TissueStack.ComponentFactory.checkDataSetOrdinal(dataSetOrdinal, checkForExistence)) {
            alert("ComponentFactory::createDataSet => Given dataSet ordinal is invalid!");
            return false;
        }

        return true;
    },
    redrawDataSet : function(dataSet) {
        if (!dataSet || !dataSet.planes) {
            alert("ComponentFactory::redrawDataSet => given data set does not have planes!");
            return;
        }

        for (var plane in dataSet.planes) {
            dataSet.planes[plane].eraseCanvasContent();
            if (dataSet.planes[plane].contrast)
                dataSet.planes[plane].contrast.initContrastSlider();
            dataSet.planes[plane].resizeCanvas(new Date().getTime());
        }
    },
    initDataSetWidget : function(div, dataSet, includeCrossHair, addScaleBar, useImageService) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::initDataSetWidget => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::initDataSetWidget => given data set is invalid!");
            return;
        }

        if (typeof(includeCrossHair) !== 'boolean')
            includeCrossHair = false;

        if (typeof(useImageService) !== 'boolean')
            useImageService = false;

        if (typeof(addScaleBar) !== 'boolean')
            addScaleBar = false;

        var is2D = TissueStack.ComponentFactory.is2Ddata(dataSet.data);
		// loop over all planes in the data, create canvas and extent objects, then display them
		for (var i=0; i < dataSet.data.length; i++) {
			var dataForPlane = dataSet.data[i];
			var planeId = dataForPlane.name;

			var zoomLevels = eval(dataForPlane.zoomLevels);
			transformationMatrix = eval(dataForPlane.transformationMatrix);

			// create extent
			var extent =
				new TissueStack.Extent(
					dataSet.id,
					!useImageService,
					dataForPlane.oneToOneZoomLevel,
					planeId,
					dataForPlane.maxSlices,
					dataForPlane.maxX,
					dataForPlane.maxY,
					dataForPlane.origX,
					dataForPlane.origY,
					dataForPlane.step,
					zoomLevels,
					transformationMatrix,
					dataForPlane.resolutionMm);

			// create canvas
			var plane = new TissueStack.Canvas(
					extent,
					"canvas_" + planeId + "_plane",
					div,
					includeCrossHair);
            if (is2D)
                plane.flag2D();

			// for scalebar to know its parent
			if (i == 0 || is2D) plane.is_main_view = true;
			if (plane.is_main_view && addScaleBar) plane.updateScaleBar();

            // set the internal db id
			plane.id = dataForPlane.id;

			var localHost = document.location.host;
			if (!localHost)
				localHost = dataSet.host;

			// query for overlays (if exist) TODO: extend to tablet and phone
			if (TissueStack.desktop && dataSet.overlays) {
				plane.overlays = [];
				for (var z=0;z<dataSet.overlays.length;z++) {
					var type = dataSet.overlays[z].type;
					if (type === 'CANVAS')
						plane.overlays[z] = new TissueStack.CanvasOverlay(z, plane, "http", localHost, dataSet.local_id, plane.id);
					else if (type === 'SVG')
						plane.overlays[z] = new TissueStack.SVGOverlay(z, plane, "http", localHost, dataSet.local_id, plane.id);
					else if (type === 'DATASET')
						plane.overlays[z] = new TissueStack.DataSetOverlay(z, plane, "http", localHost, dataSet.local_id, plane.id, dataSet.host);
				}
			}

			// set original value range
			plane.setValueRange(dataForPlane.valueRangeMin, dataForPlane.valueRangeMax);

			// store plane
			dataSet.planes[planeId] = plane;

			// get the real world coordinates
			dataSet.realWorldCoords[planeId] = plane.getDataExtent().getExtentCoordinates();

			// display data extent info on page
			plane.updateExtentInfo(dataSet.realWorldCoords[planeId]);

			// if we have more than 1 plane => show y as the main plane and make x and z the small views
			if (i != 0 && !is2D) {
				plane.changeToZoomLevel(0);
			}

            if (is2D)
                break;
		}
	},
    resizeDataSetWidget : function(parent, div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(parent)) {
            alert("ComponentFactory::resizeDataSetWidget => parent div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::resizeDataSetWidget => given div does not exist!");
            return;
        }

        // adjust sizes based on surrounding div
        var width = $('#' + parent).width();
		var height = $('#' + parent).height();

		$('#' + div).css({"width" : width, "height" : height});

		// set main canvas dimensions
		$('#' + div + '_main_view_canvas').css({"width" : width, "height" : height});
		$('#' + div + '_main_view_canvas canvas').attr("width", width);
		$('#' + div + '_main_view_canvas canvas').attr("height", height);

		// set main canvas dimensions
		var sideCanvasDims = {width: Math.floor(width * 0.3), height: Math.floor(height * 0.2)};
		$('#' + div + ' .left_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('#' + div + ' .right_side_view').css({"width" : sideCanvasDims.width, "height" : sideCanvasDims.height});
		$('#' + div + ' .left_side_view canvas').attr("width", sideCanvasDims.width);
		$('#' + div + ' .left_side_view canvas').attr("height", sideCanvasDims.height);
		$('#' + div + ' .right_side_view canvas').attr("width", sideCanvasDims.width);
		$('#' + div + ' .right_side_view canvas').attr("height", sideCanvasDims.height);
    },
    createDataSetWidget : function(div, dataSet, dataSetOrdinal, server, includeCrossHair, addScaleBar, useImageService) {
        if (!TissueStack.ComponentFactory.checkStandardInput(div, dataSet, dataSetOrdinal, true))
            return;

        if (typeof(server) === 'string' && server !== 'localhost')
            server = TissueStack.Utils.extractHostNameFromUrl(server);

        var maximizePngPath = 'images/maximize.png';
        if (server != null)
            maximizePngPath = "http://" + server.replace(/[_]/g,".") + "/" + maximizePngPath;
        else
        	maximizePngPath = "/" + maximizePngPath;

        var dataSetPrefix = "dataset_" + dataSetOrdinal;

        var html = '<div id="' + dataSetPrefix + '" class="dataset">';

        if (typeof(includeCrossHair) !== 'boolean')
            includeCrossHair = false;

        if (typeof(useImageService) !== 'boolean')
            useImageService = false;

        if (typeof(addScaleBar) !== 'boolean')
            addScaleBar = false;

        var is2D = TissueStack.ComponentFactory.is2Ddata(dataSet.data);

		// loop over all planes in the data
		for (var i=0; i < dataSet.data.length; i++) {
			var planeId = dataSet.data[i].name;

            if (is2D && i > 0) {
                 alert("ComponentFactory::createDataSetWidget => More than 1 dim data for 2D!");
                 break;
            }

			switch(i) {
				case 0:
					html +=
							'<div id="' + dataSetPrefix + '_main_view_canvas" class="canvasview canvas_' + planeId + ' background_canvas">'
						+ 	'<canvas id="' + dataSetPrefix + '_canvas_' + planeId + '_plane" class="plane"></canvas>'
						+ (includeCrossHair ?
								'<canvas id="' + dataSetPrefix + '_canvas_'  + planeId + '_plane_cross_overlay" class="cross_overlay"></canvas>'
								: '')
                        + '</div>';
					break;
				case 1:
					html +=
							'<div id="' + dataSetPrefix + '_left_side_view_canvas" class="left_side_view background_canvas">'
						+	'<img id="' + dataSetPrefix + '_left_side_view_maximize" class="canvas_' + planeId
						+ 	' maximize_view_icon" src="' + maximizePngPath + '" alt="Maximize View" />'
						+	'<canvas id="' + dataSetPrefix + '_canvas_' + planeId + '_plane" class="side_canvas"></canvas>'
						+ 	(includeCrossHair ?
								'<canvas id="' + dataSetPrefix + '_canvas_' + planeId +
								'_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>'
								: '')
                        + '</div>';
					break;
				case 2:
					html +=
							'<div id="' + dataSetPrefix + '_right_side_view_canvas" class="right_side_view background_canvas">'
						+	'<img id="' + dataSetPrefix + '_right_side_view_maximize" class="canvas_' + planeId
						+ 	' maximize_view_icon" src="' + maximizePngPath + '" alt="Maximize View" />'
						+	'<canvas id="' + dataSetPrefix + '_canvas_' + planeId + '_plane" class="side_canvas"></canvas>'
						+ 	(includeCrossHair ?
								'<canvas id="' + dataSetPrefix + '_canvas_' + planeId +
								'_plane_cross_overlay" class="side_canvas side_canvas_cross_overlay"></canvas>'
								: '')
                        + '</div>';
					break;
			}
        }

		html += "</div>";

        $("#" + div).append(html);

        if (addScaleBar)
            TissueStack.ComponentFactory.addScaleToDataSetWidget(dataSetPrefix);

        TissueStack.ComponentFactory.resizeDataSetWidget(div, dataSetPrefix);
        TissueStack.ComponentFactory.initDataSetWidget(dataSetPrefix, dataSet, includeCrossHair, addScaleBar, useImageService);
        if (dataSet.data.length > 1) TissueStack.ComponentFactory.registerMaximizeEventsForDataSetWidget(dataSetPrefix, dataSet);

        return dataSetPrefix;
    },
    destroyDataSetWidget : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::destroyDataSetWidget => Failed to add scale bar!");
            return;
        }
        // unregister slider events
        $('#' + div + '_left_side_view_maximize, #' + div + '_right_side_view_maximize').unbind("click");
        $('#' + div).remove();
    },
    createDataSetSlider : function(div, dataSet, dataSetOrdinal, plane) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div) ||
            !TissueStack.ComponentFactory.checkDivExistence("dataset_" + dataSetOrdinal) ||
            !(plane == 'x' || plane == 'y' || plane == 'z')) {
            alert("ComponentFactory::createDataSetSlider => Failed to add slider!");
            return;
        }

        var html = '<div id="dataset_' + dataSetOrdinal + '_right_panel" class="right_panel hidden">'
            + '<input data-mini="true" id="dataset_' + dataSetOrdinal + '_canvas_main_slider" class="canvasslider canvas_'
            + plane + ' ui-input-text ui-body-a ui-corner-all ui-shadow-inset ui-mini ui-slider-input" type="number" data-type="range" min="0" max="1000" value="500" step="1" orientation="vertical"  /></div>';
        $("#" + div).append(html);

        //TissueStack.ComponentFactory.initDataSetSlider("dataset_" + dataSetOrdinal, dataSet);
    },
    initDataSetSlider : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::initDataSetSlider => given data set is invalid!");
            return;
        }

        var extractCanvasId = function(sliderId, actualDataSet) {
			if (!sliderId) {
				return;
			}

			var planeId = null;
			var plane =
				TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#" + sliderId).attr("class").split(" "), "^canvas_");
			if (!plane) {
				return;
			}

			var startPos = "canvas_".length;
			planeId = plane.substring(startPos, startPos + 1);

			var dataset_prefixEnd = sliderId.lastIndexOf("_canvas_main_slider");
			if (dataset_prefixEnd > 0 && sliderId.substring(0,dataset_prefixEnd) != actualDataSet.planes[planeId].dataset_id) {
				return null;
			}

			return planeId;
		};
		var triggerQueuedRedraw = function(id, slice, actualDataSet) {
			if (!id || !actualDataSet.planes[id]) {
				return;
			}

			slice = parseInt(slice);

			if (slice < 0) slice = 0;
			else if (slice > actualDataSet.planes[id].data_extent.max_slices) slice = actualDataSet.planes[id].data_extent.max_slices;

            if (slice == actualDataSet.planes[id].getDataExtent().slice)
                return;

			actualDataSet.planes[id].events.changeSliceForPlane(slice);
			setTimeout(function(){actualDataSet.planes[id].events.updateCoordinateDisplay();}, 500);
		};

        div = TissueStack.phone ? '.canvasslider' : '#' + div + '_canvas_main_slider';
        if (TissueStack.phone) {
            for (var p in dataSet.planes) {
                (function(p) {
                    // initial page create that is necessary sadly
                    $("#tissue" + p.toUpperCase()).off("pagecreate");
                    $("#tissue" + p.toUpperCase()).on("pagecreate", function () {
                        if (!dataSet.planes[p]) return;

                        // unbind previous change
                        $('#canvas_' + p + '_slider').unbind("change");
                        $('#canvas_' + p + '_slider').bind("change", function (event, ui)  {
                            var id = extractCanvasId(this.id);
                            if (!id) {
                                return;
                            }
                            triggerQueuedRedraw(id, this.value, dataSet);
                        });
                    });

                    $("#tissue" + p.toUpperCase()).off("pagebeforeshow");
                    $("#tissue" + p.toUpperCase()).on("pagebeforeshow", function () {
                        if (!dataSet.planes[p]) {
                            $('#canvas_' + p + '_slider').attr("min", 0);
                            $('#canvas_' + p + '_slider').attr("max", 0);
                            try {
                                $('#canvas_' + p + '_slider').val(0).slider("refresh");
                            } catch(ignored) {}

                            return;
                        }


                        $('#canvas_' + p + '_slider').attr("min", 0);
                        $('#canvas_' + p + '_slider').attr("max", dataSet.planes[p].data_extent.max_slices);

                        try {
                            $('#canvas_' + p + '_slider').val(dataSet.planes[p].data_extent.slice).slider("refresh");
                        } catch(ignored) {}
                    });
                })(p);
            }
        } else {
            	var id = extractCanvasId($(div).attr("id"), dataSet);

					if (!id) {
						return;
					}

                    if (!dataSet.planes[id]) {
                        $(div).attr("min", 0);
                        $(div).attr("max", 0);
                        $(div).attr("value", 0);
                        return;
                    }

                    $(div).attr("min", 0);
                    $(div).attr("max", dataSet.planes[id].data_extent.max_slices);

                     try {
                            $(div).slider();
                        } catch(ignored) {}
        }

        $(div).unbind("change");
        // rebind
        $(div).bind ("change", function (event, ui)  {
            var id = extractCanvasId(this.id, dataSet);
            if (!id) return;
            triggerQueuedRedraw(id, this.value, dataSet);
        });
    },
    destroyDataSetSlider : function(div) {
        var rightPanelDiv = div + "_right_panel";
        if (!TissueStack.ComponentFactory.checkDivExistence(rightPanelDiv))
            return;

        // unregister maximize events
        $('#' + div + '_canvas_main_slider').unbind("change");
        $('#' + rightPanelDiv).remove();
    },
    addScaleToDataSetWidget : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::addScaleToDataSet => Failed to add scale bar!");
            return;
        }

        var html = '<div class="scalecontrol_main"><div id="' + div + '_scale_middle" class="scalecontrol_middle">'
            + '<div class="scalecontrol_image" style="left: 0px; top: -424px; width: 89px;"></div></div>'
            + '<div id="' + div + '_scale_left" class="scalecontrol_left">'
			+ '<div class="scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
			+ '<div id="' + div + '_scale_center_left" class="scalecontrol_center_left">'
	    	+ '<div class="scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + div + '_scale_center_right" class="scalecontrol_center_right">'
	    	+ '<div class="scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + div + '_scale_up" class="scalecontrol_up">'
    		+ '<div class="scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + div + '_scale_text_up" class="scalecontrol_text_up"></div></div>';

        $("#" + div).prepend(html);
	},
    swapWithMainCanvas : function(div, dataSet, sideViewPlaneId) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::swapWithMainCanvas => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::swapWithMainCanvas => given data set is invalid!");
            return;
        }

        if (sideViewPlaneId !== 'x' &&
            sideViewPlaneId !== 'y' &&
            sideViewPlaneId !== 'z') {
            alert("ComponentFactory::swapWithMainCanvas => side plane was not x,y or z!");
            return;
        }

        var plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($('#' + div + '_main_view_canvas').attr("class").split(" "), "^canvas_");
        if (!plane) {
            alert("ComponentFactory::swapWithMainCanvas => could not find main canvas plane!");
            return;
        }
        var startPos = "canvas_".length;
        var mainViewPlaneId = plane.substring(startPos, startPos + 1);
        if (mainViewPlaneId !== 'x' &&
            mainViewPlaneId !== 'y' &&
            mainViewPlaneId !== 'z') {
            alert("ComponentFactory::swapWithMainCanvas => main plane was not x,y or z!");
            return;
        }

        if (mainViewPlaneId == sideViewPlaneId)
            return;

        // with the id we get the can get the main canvas and the side canvas and swap them, including their dimensions and zoom levels
        var mainCanvas = $('#' + div + '_main_view_canvas');
        var mainCanvasChildren = mainCanvas.children("canvas");
        if (!mainCanvasChildren || mainCanvasChildren.length == 0) {
            alert("ComponentFactory::swapWithMainCanvas => main canvas has no children!");
            return;
        }
        mainCanvasChildren.detach();

        // find canvas of sideViewPlaneId we want to swap with, try left one first
        var leftSideCanvas =  $('#' + div + '_left_side_view_maximize');
        if (!leftSideCanvas || leftSideCanvas.length == 0) {
            alert("ComponentFactory::swapWithMainCanvas => could not find left view canvas!");
            return;
        }
        var plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray(leftSideCanvas.attr("class").split(" "), "^canvas_");
        if (!plane) {
          alert("ComponentFactory::swapWithMainCanvas => could not find left view canvas plane id!");
		  return;
        }
		var startPos = "canvas_".length;
		var pId = plane.substring(startPos, startPos + 1);
        var sideCanvasId = div + '_left_side_view';
        if (pId !== sideViewPlaneId)
            sideCanvasId = div + '_right_side_view';

        var sideCanvas = $("#" + sideCanvasId + '_canvas');
        var sideCanvasChildren = sideCanvas.children("canvas");
        if (!sideCanvasChildren || sideCanvasChildren.length == 0) {
            alert("ComponentFactory::swapWithMainCanvas => side canvas has no children!");
            return;
        }
        sideCanvasChildren.detach();

        TissueStack.Utils.swapOverlayAndUnderlyingCanvasPlanes(dataSet, mainViewPlaneId, sideViewPlaneId);

        // swap dimensions
        var sideCanvasRelativeCross = dataSet.planes[sideViewPlaneId].getRelativeCrossCoordinates();
        sideCanvasRelativeCross.z = dataSet.planes[sideViewPlaneId].getDataExtent().slice;
        var mainCanvasRelativeCross = dataSet.planes[mainViewPlaneId].getRelativeCrossCoordinates();
        mainCanvasRelativeCross.z = dataSet.planes[mainViewPlaneId].getDataExtent().slice;

        var sideCanvasDims = {x: sideCanvasChildren[0].width, y: sideCanvasChildren[0].height};
        var mainCanvasDims = {x: mainCanvasChildren[0].width, y: mainCanvasChildren[0].height};

        var tmpAttr = [];
        for (var i=0; i < sideCanvasChildren.length; i++) {
            tmpAttr[i] = sideCanvasChildren[i].getAttribute("class");
            sideCanvasChildren[i].setAttribute("class", mainCanvasChildren[i].getAttribute("class"));
            sideCanvasChildren[i].width = mainCanvasDims.x;
            sideCanvasChildren[i].height = mainCanvasDims.y;
        }
        dataSet.planes[sideViewPlaneId].setDimensions(mainCanvasDims.x, mainCanvasDims.y);
        // store zoom level for side view
        var zoomLevelSideView = dataSet.planes[sideViewPlaneId].getDataExtent().zoom_level;

        for (var i=0; i < mainCanvasChildren.length; i++) {
            mainCanvasChildren[i].setAttribute("class", tmpAttr[i]);
            mainCanvasChildren[i].width = sideCanvasDims.x;
            mainCanvasChildren[i].height = sideCanvasDims.y;
        }
        dataSet.planes[mainViewPlaneId].setDimensions(sideCanvasDims.x, sideCanvasDims.y);

        mainCanvas.append(sideCanvasChildren);
        sideCanvas.append(mainCanvasChildren);

        // remember change in class
        $("#" + sideCanvasId + "_maximize").addClass("canvas_" + mainViewPlaneId);
        $("#" + sideCanvasId  + "_maximize").removeClass("canvas_" + sideViewPlaneId);
        $("#" + div +"_main_view_canvas").addClass("canvas_" + sideViewPlaneId);
        $("#" + div +"_main_view_canvas").removeClass("canvas_" + mainViewPlaneId);
        $("#" + div + "_canvas_main_slider").addClass("canvas_" + sideViewPlaneId);
        $("#" + div + "_canvas_main_slider").removeClass("canvas_" + mainViewPlaneId);
        // swap slice dimension values
        $("#" + div + "_canvas_main_slider").attr("value", dataSet.planes[sideViewPlaneId].data_extent.slice);
        $("#" + div + "_canvas_main_slider").attr("max", dataSet.planes[sideViewPlaneId].data_extent.max_slices);

        // swap progress display
        var disp1 = $("#" + div + " .tile_count_div span." + mainViewPlaneId);
        var disp2 = $("#" + div + " .tile_count_div span." + sideViewPlaneId);
        disp1.removeClass(mainViewPlaneId);
        disp1.addClass(sideViewPlaneId);
        disp2.removeClass(sideViewPlaneId);
        disp2.addClass(mainViewPlaneId);

        // swap main view
        dataSet.planes[mainViewPlaneId].is_main_view = false;
        dataSet.planes[sideViewPlaneId].is_main_view = true;

        // redraw and change the zoom level as well
        var now = new Date().getTime();

        dataSet.planes[sideViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(sideCanvasRelativeCross, false, now);
        dataSet.planes[mainViewPlaneId].redrawWithCenterAndCrossAtGivenPixelCoordinates(mainCanvasRelativeCross, false, now);
        dataSet.planes[sideViewPlaneId].events.changeSliceForPlane(dataSet.planes[sideViewPlaneId].data_extent.slice);
        dataSet.planes[sideViewPlaneId].changeToZoomLevel(dataSet.planes[mainViewPlaneId].getDataExtent().zoom_level);
        dataSet.planes[mainViewPlaneId].changeToZoomLevel(zoomLevelSideView);
        try {
            $("#" + div + "_canvas_main_slider").val(dataSet.planes[sideViewPlaneId].data_extent.slice).slider("refresh");
        } catch(ign) {}

        dataSet.planes[sideViewPlaneId].updateExtentInfo(dataSet.planes[sideViewPlaneId].getDataExtent().getExtentCoordinates());

        // Given TissueStack.overlay_datasets => trigger swap event for other dataset if exists
        var underlyingCanvas = dataSet.planes[mainViewPlaneId].underlying_canvas;
        if (TissueStack.overlay_datasets && underlyingCanvas) {
            var descDataSet =
                TissueStack.dataSetStore.getDataSetById(
                    TissueStack.dataSetNavigation.selectedDataSets[underlyingCanvas.dataset_id]);
            if (descDataSet)
                TissueStack.ComponentFactory.swapWithMainCanvas(underlyingCanvas.dataset_id, descDataSet, sideViewPlaneId);
        }
    },
	registerMaximizeEventsForDataSetWidget : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::registerMaximizeEventsForDataSetWidget => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::registerMaximizeEventsForDataSetWidget => given data set is invalid!");
            return;
        }

        // avoid potential double binding by un-binding at this stage
        $('#' + div + '_left_side_view_maximize, #' + div + '_right_side_view_maximize').unbind("click");
        // rebind
        $('#' + div + '_left_side_view_maximize, #' + div + '_right_side_view_maximize').bind("click", function(event) {
            // what side view and canvas called for maximization
            if (!event.target.id || !$("#" + event.target.id).attr("class")) {
                alert("ComponentFactory::registerMaximizeEventsForDataSetWidget => could not associate event target with maximize button!");
                return;
            }

            var plane = TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray($("#" + event.target.id).attr("class").split(" "), "^canvas_");
            if (!plane) {
                alert("ComponentFactory::registerMaximizeEventsForDataSetWidget => could not associate plane with maximize event!");
                return;
            }
            var startPos = "canvas_".length;
            var sideViewPlaneId = plane.substring(startPos, startPos + 1);

            TissueStack.ComponentFactory.swapWithMainCanvas(div, dataSet, sideViewPlaneId);
        });
	},
    applyUserParameters : function(initOpts, dataSet, timeout) {
        if (!initOpts) return;

        var plane_id = typeof(initOpts['plane']) === 'string' ?  initOpts['plane'] : 'y';
        // plane or data set does not exist => good bye
        if (!dataSet || !dataSet.planes[plane_id]) {
            alert("TissueStack::applyUserParameters => given initialization parameters are invalid!");
            return;
        }

        if (typeof(timeout) != 'number')
            timeout = 0;

        // this is an adjustment for 2D data (3 dims with one having 1 slice only)
        var is2D = TissueStack.ComponentFactory.is2Ddata(dataSet.data);
        if (is2D)
            plane_id = dataSet.data[0].name;

        if (!TissueStack.phone)
            TissueStack.ComponentFactory.swapWithMainCanvas(dataSet.planes[plane_id].dataset_id, dataSet, plane_id);

        var plane = dataSet.planes[plane_id];

        setTimeout(function() {
            if (is2D)
        		for (var p in dataSet.planes)
        			if (p != plane_id &&
        				!(TissueStack.overlay_datasets && TissueStack.dataSetNavigation.selectedDataSets.count > 1))
        				dataSet.planes[p].hideCanvas();

            if (initOpts['zoom'] != null && initOpts['zoom'] >= 0 && initOpts['zoom'] < plane.data_extent.zoom_levels.length) {
                plane.changeToZoomLevel(initOpts['zoom']);
            }

            if (initOpts['color'] && initOpts['color'] != 'grey' && TissueStack.indexed_color_maps[initOpts['color']]) {
                // change color map collectively for all planes
                for (var id in dataSet.planes) {
                    dataSet.planes[id].color_map = initOpts['color'];
                    dataSet.planes[id].is_color_map_tiled = null;
                }

                // set right radio button
                if (TissueStack.phone) {
                    try {
                        $("#colormap_choice input").removeAttr("checked").checkboxradio("refresh");
                        $("#colormap_" + initOpts['color']).attr("checked", "checked").checkboxradio("refresh");
                    } catch (e) {
                        // we don't care, stupid jquery mobile ...
                        $("#colormap_" + initOpts['color']).attr("checked", "checked");
                    }
                } else {
                    $("#" + plane.dataset_id + "_color_map .color_map_select").val(initOpts['color']);
                    try {
                        $("#" + plane.dataset_id + "_color_map .color_map_select").selectmenu("refresh");
                    } catch (any){
                        // we don't care, stupid jquery mobile ...
                    }
                }
            }
            if (typeof(initOpts['min']) === 'number' &&  typeof(initOpts['max']) === 'number') {
                // change contrast collectively for all planes
                for (var id in dataSet.planes) {
                    if (dataSet.planes[id].contrast) {
                        dataSet.planes[id].contrast.drawContrastSlider();
                        dataSet.planes[id].contrast.moveBar('min',
                                dataSet.planes[id].contrast.getMinimumBarPositionForValue(initOpts['min']));
                        dataSet.planes[id].contrast.moveBar('max',
                                dataSet.planes[id].contrast.getMaximumBarPositionForValue(initOpts['max']));
                        dataSet.planes[id].contrast.drawMinMaxValues();
                    }
                }
            }

            var givenCoords = {};
            if (initOpts['x'] != null || initOpts['y'] != null || initOpts['z'] != null) {
                givenCoords = {x: initOpts['x'] != null ? initOpts['x'] : 0,
                        y: initOpts['y'] != null ? initOpts['y'] : 0,
                        z: initOpts['z'] != null ? initOpts['z'] : 0};

                if (plane.getDataExtent().worldCoordinatesTransformationMatrix) {
                    givenCoords = plane.getDataExtent().getPixelForWorldCoordinates(givenCoords);
                }
            } else {
                givenCoords = plane.getRelativeCrossCoordinates();
                givenCoords.z = plane.getDataExtent().slice;
            }
            var now = new Date().getTime();
            plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(givenCoords, true, now);
            plane.events.changeSliceForPlane(givenCoords.z);
            plane.queue.drawLowResolutionPreview(now);
            plane.queue.drawRequestAfterLowResolutionPreview(null, now);

            var slider = TissueStack.phone ?
                    $("#canvas_" + plane.data_extent.plane + "_slider") :
                    $("#" + (plane.dataset_id == "" ? "" : plane.dataset_id + "_") + "canvas_main_slider");
            if (slider && slider.length == 1) {
                try {
                    slider.val(givenCoords.z);
                    slider.blur();
                } catch(ignored) {}
            };
            plane.events.updateCoordinateDisplay();
        }, timeout);
    },
    addMeasuringContextMenu : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::addProgressBar => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::addMeasuringContextMenu => given data set is invalid!");
            return;
        }

        if (dataSet.data.length == 0)
            return;

		var myMeasuringContext = $("#measuringContextMenu");
		if (!myMeasuringContext || myMeasuringContext.length === 0) return;

        $("#" + div + "_main_view_canvas").off("contextmenu");
		$("#" + div + "_main_view_canvas").on("contextmenu",
			function(event) {
                // we measure only within main canvas
                if (event.currentTarget.id !== ("" + div + "_main_view_canvas"))
                    return;

                var mainCanvas =
                    TissueStack.Utils.findMainCanvasInDataSet(dataSet);
                if (mainCanvas === null) return;

                var offsets = { x: event.offsetX, y: mainCanvas.dim_y - event.offsetY};
                // check if we are within the image boundaries
                var withinImage =
                    (offsets.x >= mainCanvas.upper_left_x &&
                        offsets.y <= mainCanvas.upper_left_y &&
                        offsets.x <= mainCanvas.upper_left_x + mainCanvas.data_extent.x &&
                        offsets.y >= mainCanvas.upper_left_y - mainCanvas.data_extent.y)
                        ? true : false;

                // context menu style and behavior
                myMeasuringContext.css({top: event.clientY, left: event.clientX});
                myMeasuringContext.off("mouseover mouseout");
                myMeasuringContext.on("mouseover", function() {
                    myMeasuringContext.show();
                });
                myMeasuringContext.on("mouseout", function() {
                    myMeasuringContext.hide();
                });

                if (withinImage) {
                    var point =
                        {x: offsets.x - mainCanvas.upper_left_x,
                         y: mainCanvas.upper_left_y - offsets.y,
                         z: mainCanvas.data_extent.slice};
                    mainCanvas.checkMeasurements(point);
                    myMeasuringContext.children(".distance").html(
                        "Distance: " + mainCanvas.measureDistance().toFixed(5));
                    myMeasuringContext.children(".area").html(
                        "Area: " + mainCanvas.measureArea().toFixed(5));
                    myMeasuringContext.children(".distance,.area").show();
                    if (mainCanvas.measurements.length > 0)
                        myMeasuringContext.children(".menue_item").show();
                    else {
                        myMeasuringContext.children(".addPoint").show();
                        myMeasuringContext.children(".resetPath").hide();
                    }
                    myMeasuringContext.children(".outside_image").hide();

                    myMeasuringContext.children(".menue_item").off("mouseover mouseout click");
                    myMeasuringContext.children(".menue_item").on("mouseover", function(event) {
                        $(this).addClass("hover");
                    });
                    myMeasuringContext.children().on("mouseout", function(event) {
                        $(this).removeClass("hover");
                    });

                    // add point action
                    myMeasuringContext.children(".addPoint").on("click", function(event) {
                        mainCanvas.addMeasure(point);
                        $(this).hide();
                        myMeasuringContext.children(".resetPath").show();
                        myMeasuringContext.children(".distance").html(
                            "Distance: " +  mainCanvas.measureDistance().toFixed(5));
                        myMeasuringContext.children(".area").html(
                            "Area: " + mainCanvas.measureArea().toFixed(5));
                        if (mainCanvas.measurements.length <= 1)
                            myMeasuringContext.hide();
                    });
                    // add reset action
                    myMeasuringContext.children(".resetPath").on("click", function(event) {
                        mainCanvas.resetMeasurements();
                        myMeasuringContext.children(".addPoint").show();
                        $(this).hide();
                        myMeasuringContext.hide();
                    });
                } else {
                    myMeasuringContext.children(".menue_item,.distance,.area").hide();
                    myMeasuringContext.children(".outside_image").show();
                }

                myMeasuringContext.show();

				// stop browser from showing you its context menu
				if (event.stopPropagation)
					event.stopPropagation();
				else event.cancelBubble = true;
				return false;
		});
    },
    addProgressBar : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::addProgressBar => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::addProgressBar => given data set is invalid!");
            return;
        }

        if (dataSet.data.length == 0)
            return;

        var placeHolder = [null,null,null];
        for (var x=0;x<dataSet.data.length;x++){
            // this becomes main by default
            if (x==0)
                placeHolder[1] =
                    '<span class="plane_2 ' + dataSet.data[0].name + '">0%</span>';
            else if (x==1)
                placeHolder[0] =
                    '<span class="plane_1 ' + dataSet.data[1].name + '">0%</span>';
            else if (x==2)
                placeHolder[3] =
                    '<span class="plane_3 ' + dataSet.data[2].name + '">0%</span>';
        }

        var html = '<div class="tile_count_div">';
        for (var x=0;x<placeHolder.length;x++)
            if (placeHolder[x])
                html += placeHolder[x];
        html += '</div>';

        $("#" + div).prepend(html);
        TissueStack.ComponentFactory.resizeProgressBar(div);
    },
    resizeProgressBar : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            return;
        }

        // adjust position of progress bar
        var wDiv = $("#" + div).width();
        var wProgress = $("#" + div + " .tile_count_div").width();

        $("#" + div + " .tile_count_div").css("left", function( style ) {
            return (wDiv / 2) - (wProgress / 2);
        });
    },
    resizeDataSetSlider : function(height) {
        $('.ui-slider-vertical').css({"height": height - 55});
    },
    createColorMapSwitcher : function(div) {
      if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::createColorMapSwitcher => Failed to add color map control!");
            return;
        }

        var html = '<div id="' + div + '_color_map" class="color_map_main" title="Choose color For Data Set">'
                +  '<select class="color_map_select color_map_style" data-role="none"><option>Uninitialized</option>'
                +  '</select></div>';
        $("#" + div).append(html);
        TissueStack.Utils.updateColorMapChooser();
    }, initColorMapSwitcher : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::initColorMapSwitcher => given data set is invalid!");
            return;
        }

        if (TissueStack.phone) { // we use this one only for the phone
            $('input[name="color_map"]').unbind("click");
            // rebind
            $('input[name="color_map"]').bind("click", function(e) {
                    var now = new Date().getTime();
                    for (var id in dataSet.planes) {
                        dataSet.planes[id].color_map = e.target.value;
                        dataSet.planes[id].is_color_map_tiled = null;
                        dataSet.planes[id].queue.drawLowResolutionPreview(now);
                        dataSet.planes[id].queue.drawRequestAfterLowResolutionPreview(null, now);
                    }
            });
            return;
        }

        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
        alert("ComponentFactory::initColorMapSwitcher => could not find data set div!");
        return;
        }

        // unbind
        $('#' + div + '_color_map').unbind("change");
        // rebind
        $('#' + div + '_color_map').bind("change", function(event) {
            var now = new Date().getTime();
            for (var id in dataSet.planes) {
                dataSet.planes[id].color_map = event.target.value;
                dataSet.planes[id].is_color_map_tiled = null;
                dataSet.planes[id].queue.drawLowResolutionPreview(now);
                dataSet.planes[id].queue.drawRequestAfterLowResolutionPreview(null, now);
            }
        });
    },
    createUrlLink : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::createUrlLink => Failed to add url link control!");
            return;
        }

        var html =
            '<div class="url_link_main"><a id="' + div + '_url_button" class="top_right_button" style="color: white;" href="#">URL</a>'
            + '<div id="' + div + '_url_box" class="url_box"><div class="url_arrow"></div>'
            + '<div class="url_arrow-border"></div><div id="' + div + '_link_message" class="url_link_message"></div></div></div>';
        $("#" + div).append(html);

        TissueStack.ComponentFactory.initUrlLink(div);
    }, initUrlLink : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div))
            return;

        $('#'+ div + '_url_button').unbind('click').click(function(){
		  $('#'+ div + '_url_box').toggle();
        });
    }, createContrastSlider : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div, dataSet)) {
            alert("ComponentFactory::createContrastSlider => Failed to add contrast control!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::createContrastSlider => given data set is invalid!");
            return;
        }

        var html =
            '<div class="contrast_main"><a id="' + div + '_toolbox_canvas_button" class="top_right_button" style="color: white;" href="#">CONTRAST</a>'
            + '<div id="' + div + '_contrast_box" class="contrast_box"><div class="url_arrow"></div><div class="url_arrow-border"></div><canvas id="'
            + div + '_toolbox_canvas" class="contrast_slider "></canvas></div></div>';
        $("#" + div).append(html);

        TissueStack.ComponentFactory.initContrastSlider(div, dataSet);
    }, initContrastSlider : function(div, dataSet) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div) ||
            !TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet))
            return;

        var contrastSlider = new TissueStack.ContrastCanvas(div + "_toolbox_canvas");
        for (var p in dataSet.planes) {
            dataSet.planes[p].contrast = contrastSlider;
            dataSet.planes[p].contrast.canvas =  dataSet.planes[p];
        }

        $('#'+ div + '_toolbox_canvas_button').unbind('click');
		$('#'+ div + '_toolbox_canvas_button').click(function(){
		  $('#'+ div + '_contrast_box').toggle();
        });
    },
    addTransparencyWheel : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::addTransparencyWheel => Failed to add transparency control!");
            return;
        }

        var html =
            '<div class="transparency_knob_div" title="Adjust Transparency of Overlay">'
            + '<input  id="transparency_knob" type="text" data-width="75" data-height="75" data-fgColor="#ffec03" data-min="0" data-max="100" data-thickness=".2" value="50">'
            + '</div>';

        $("#" + div).append(html);
    },
    initTransparencyWheel : function(handler) {
        if (typeof(handler) != 'function')
            return;

        TissueStack.transparency = 0.5; // reset
		$("#transparency_knob").unbind('change');
        $("#transparency_knob").knob({'change' : handler});
		$("#transparency_knob").val(TissueStack.transparency * 100).trigger('change');
    },
    addDataSetSwapper : function(div) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::addDataSetSwapper => Failed to add dataset swapper control!");
            return;
        }

        var html =
            '<div class="overlay_swapper" title="Swap Data Sets">'
            + '<a href="#"><img alt="Swap Data Sets" src="images/swap_button.png" ></a></div>';

        $("#" + div).append(html);
    },
    initDataSetSwapper : function(handler) {
        if (typeof(handler) != 'function')
            return;

		$(".overlay_swapper").unbind("click");
		$(".overlay_swapper").bind("click", handler);
    }, is2Ddata : function(planes) {
        if (typeof(planes) != 'object')
            return false;

        if (!planes.length || planes.length > 1)
            return false;

        if (typeof(planes[0].is2D) === 'boolean')
            return planes[0].is2D;

        return false;
    }
};
