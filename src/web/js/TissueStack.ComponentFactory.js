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
    initDataSetWidget : function(div, dataSet, includeCrossHair, addScaleBar, useImageService) {
        if (!TissueStack.ComponentFactory.checkDivExistence(div)) {
            alert("ComponentFactory::resizeDataSetWidget => given div does not exist!");
            return;
        }

        if (!TissueStack.ComponentFactory.checkDataSetObjectIntegrity(dataSet)) {
            alert("ComponentFactory::resizeDataSetWidget => given data set is invalid!");
            return;
        }
        
        if (typeof(includeCrossHair) !== 'boolean')
            includeCrossHair = false;

        if (typeof(useImageService) !== 'boolean')
            useImageService = false;

        if (typeof(addScaleBar) !== 'boolean')
            addScaleBar = false;

        // we use that for the image service to be able to abort pending requests
		var now = new Date().getTime();
		
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
					zoomLevels,
					transformationMatrix,
					dataForPlane.resolutionMm);
			
			// create canvas
			var plane = new TissueStack.Canvas(
					extent,
					"canvas_" + planeId + "_plane",
					div,
					includeCrossHair);

			// for scalebar to know its parent
			if (i == 0) plane.is_main_view = true;
			if (plane.is_main_view && addScaleBar) plane.updateScaleBar();
			
			// store plane  
			dataSet.planes[planeId] = plane;

			// get the real world coordinates 
			dataSet.realWorldCoords[planeId] = plane.getDataExtent().getExtentCoordinates();
			
			// display data extent info on page
			plane.updateExtentInfo(dataSet.realWorldCoords[planeId]);

			// if we have more than 1 plane => show y as the main plane and make x and z the small views
			if (i != 0) {
				plane.changeToZoomLevel(0);
			} 

			plane.queue.drawLowResolutionPreview(now);
			plane.queue.drawRequestAfterLowResolutionPreview(null, now);
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
            maximizePngPath = "http://" + server + "/" + maximizePngPath;
        
        var dataSetPrefix = "dataset_" + dataSetOrdinal;
        
        var html = '<div id="' + dataSetPrefix + '" class="dataset">';
		
        if (typeof(includeCrossHair) !== 'boolean')
            includeCrossHair = false;

        if (typeof(useImageService) !== 'boolean')
            useImageService = false;

        if (typeof(addScaleBar) !== 'boolean')
            addScaleBar = false;

		// loop over all planes in the data
		for (var i=0; i < dataSet.data.length; i++) {
			var planeId = dataSet.data[i].name;
			
			switch(i) {
				case 0:
					html +=
							'<div id="' + dataSetPrefix + '_main_view_canvas" class="canvasview canvas_' + planeId + '">'
						+ 	'<canvas id="' + dataSetPrefix + '_canvas_' + planeId + '_plane" class="plane"></canvas>'
						+ (includeCrossHair ? 
								'<canvas id="' + dataSetPrefix + '_canvas_'  + planeId + '_plane_cross_overlay" class="cross_overlay"></canvas>'
								: '')
                        + '</div>';
					break;
				case 1:
					html +=
							'<div id="' + dataSetPrefix + '_left_side_view_canvas" class="left_side_view ui-bar-a">'
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
							'<div id="' + dataSetPrefix + '_right_side_view_canvas" class="right_side_view ui-bar-a">'
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
        
        if (mainViewPlaneId == sideViewPlaneId) {
            return;
        }
        
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
        $("#" + div + "_canvas_main_slider").blur();

        dataSet.planes[sideViewPlaneId].updateExtentInfo(dataSet.planes[sideViewPlaneId].getDataExtent().getExtentCoordinates());

        // Given TissueStack.overlay_datasets => trigger swap event for other dataset if exists
        if (!TissueStack.planes_swapped && TissueStack.overlay_datasets && TissueStack.dataSetNavigation.selectedDataSets.count > 1) {
            var elementIdForMaximizingOtherDataSet = event.target.id;
            var thisHereDataSet = elementIdForMaximizingOtherDataSet.substring(0, "dataset_x".length);

            var thisOtherDataSet = "dataset_2";
            if (thisHereDataSet == 'dataset_2')
                thisOtherDataSet = "dataset_1";
            elementIdForMaximizingOtherDataSet = elementIdForMaximizingOtherDataSet.replace(thisHereDataSet, thisOtherDataSet);
            // set to swapped so that we avoid cycle
            TissueStack.planes_swapped = true;

            setTimeout(function() {$('#' + elementIdForMaximizingOtherDataSet).click();}, 200);
        } else 
            TissueStack.planes_swapped = false;
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
        
        if (!TissueStack.phone)
            TissueStack.ComponentFactory.swapWithMainCanvas("dataset_1", dataSet, plane_id);

        var plane = dataSet.planes[plane_id];

        setTimeout(function() {
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
                    $(".color_map_select").val(initOpts['color']);
                    try {
                        $(".color_map_select").selectmenu("refresh");
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
            plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(givenCoords, false, now);
            plane.events.changeSliceForPlane(givenCoords.z);
            plane.queue.drawLowResolutionPreview(now);
            plane.queue.drawRequestAfterLowResolutionPreview(null, now);

            var slider = TissueStack.phone ? 
                    $("#canvas_" + plane.data_extent.plane + "_slider") :
                    $("#" + (plane.dataset_id == "" ? "" : plane.dataset_id + "_") + "canvas_main_slider");
            if (slider && slider.length == 1) {
                slider.attr("value", givenCoords.z);
                 if (!TissueStack.phone) slider.slider("refresh");
            };
        }, timeout);
    }
};