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
TissueStack.Init = function () {
	var afterLoadingRoutine = function() {
		// create an instance of the navigation
		TissueStack.dataSetNavigation = new TissueStack.DataSetNavigation();

		var ds = TissueStack.dataSetStore.getDataSetByIndex(0);
					
		// see if we have received initial values for data set incl. coords and zoom level
		var initOpts = TissueStack.Utils.readQueryStringFromAddressBar();
		if (initOpts) {
            TissueStack.useUserParameters = true;
			if (initOpts['ds']) {
				var dsAlt = TissueStack.dataSetStore.getDataSetById('localhost_' + initOpts['ds']);
				if (dsAlt)
					ds = dsAlt;
			}
		}

		// initialize ui and events
		if (TissueStack.desktop)
            TissueStack.dataSetNavigation.getDynaTreeObject().selectKey(ds.id);
        else if (TissueStack.tablet)
            $("#tabletTreeDiv-" + ds.local_id + "-" + ds.host).collapsible("expand");
        else { // phone
			TissueStack.dataSetNavigation.addDataSet(ds.id, 0);
			TissueStack.InitPhoneUserInterface();
			TissueStack.BindDataSetDependentEvents();
            TissueStack.dataSetNavigation.showDataSet(1);
		}
        TissueStack.Utils.adjustBorderColorWhenMouseOver();	
			
        TissueStack.useUserParameters = false;
        
		TissueStack.BindGlobalEvents();
		if (initOpts) {
            TissueStack.ComponentFactory.applyUserParameters(initOpts, ds);
            TissueStack.dataSetNavigation.setDataSetVisibility(ds.id, true);
        }

		//if (TissueStack.dataSetStore && TissueStack.dataSetStore.datasetCount && TissueStack.dataSetStore.datasetCount > 1) TissueStack.dataSetNavigation.showDataSet(1);
		
		// add admin functionality to all versions
		TissueStack.admin = new TissueStack.Admin();
	};

	TissueStack.Utils.sendAjaxRequest(
		"/" + TissueStack.configuration['server_proxy_path'].value +
		"/?service=services&sub_service=configuration&action=all", 'GET', false,
		function(data, textStatus, jqXHR) {
			if (!data.response && !data.error) {
				alert("Did not receive anyting, neither success nor error ....");
				return;
			}
			
			if (data.error) {
				var message = "Application Error: " + (data.error.description ? data.error.description : " no more info available. check logs.");
				alert(message);
				return;
			}
			
			if (data.response.noResults) {
				alert("No configuration info found in database");
				return;
			}
			var configuration = data.response;
			
			for (var x=0;x<configuration.length;x++) {
				if (!configuration[x] || !configuration[x].name || $.trim(!configuration[x].name.length) == 0) {
					continue;
				}
				TissueStack.configuration[configuration[x].name] = {};
				TissueStack.configuration[configuration[x].name].value = configuration[x].value;
				TissueStack.configuration[configuration[x].name].description = configuration[x].description ? configuration[x].description : "";
			};
			
			// hide second jquery coordinate search button  
			if (TissueStack.desktop) {
				$('#dataset_2_center_point_in_canvas').closest('.ui-btn').hide();
			}
		
			// prepare color maps
			TissueStack.Utils.loadColorMaps();
		
			// add phone menu for page navigation
			if (TissueStack.phone) {
				new TissueStack.PhoneMenu();
			}
		
			// create data store and load it with backend data
			TissueStack.dataSetStore = new TissueStack.DataSetStore(afterLoadingRoutine);
			
			// handle window resizing
			$(window).resize(function() {
				// this needs to be checked in cases where the resize fires before the creation of the dataSetNavigation
				if (typeof(TissueStack.dataSetNavigation) == "undefined" || typeof(TissueStack.dataSetNavigation.selectedDataSets) == 'undefined') {
					return;
				} 
				
				var dataSetCount = TissueStack.dataSetNavigation.selectedDataSets.count;
				var now = new Date().getTime();
				TissueStack.lastWindowResizing = now;
				
				setTimeout(function() {
					if (now < TissueStack.lastWindowResizing) return;
					TissueStack.Utils.adjustScreenContentToActualScreenSize(dataSetCount);
					// set new canvas dimensions
					for (var i=0;i<dataSetCount;i++) {
						var dataSet = TissueStack.dataSetStore.getDataSetById(TissueStack.dataSetNavigation.selectedDataSets["dataset_" + (i+1)]);
						for (var plane in dataSet.planes) {
							dataSet.planes[plane].resizeCanvas(TissueStack.lastWindowResizing);
						}
					}
				}, 250);
			});
		},
		function(jqXHR, textStatus, errorThrown) {
			alert("Error connecting to backend: " + textStatus + " " + errorThrown);
		}
	);
};

TissueStack.InitPhoneUserInterface = function (drawMe) {
	if (TissueStack.dataSetNavigation.selectedDataSets.count == 0) {
		return;
	}

    if (typeof(drawMe) != 'boolean')
        drawMe = true;
    
	TissueStack.Utils.adjustScreenContentToActualScreenSize(1);
	
	// initialize the color map chooser for desktop and tablet and (now in phone version)
	TissueStack.Utils.updateColorMapChooser();
	
	var dataSet =
		TissueStack.dataSetStore.getDataSetById(
			TissueStack.dataSetNavigation.selectedDataSets["dataset_1"]);
	
	if (!dataSet.data || dataSet.data.length == 0) {
		alert("Data set '" + dataSet.id + "' does not have any planes associated with it!");
		return; 
	}
	
	// crate a contrast slider per data set
	var contrast = null;
	if (TissueStack.phone)
		contrast = new TissueStack.ContrastCanvas("dataset_1_toolbox_canvas_phone");
	
	// loop over all planes in the data, create canvas and extent objects, then display them
	var main_view_plane = null;
	for (var i=0; i < dataSet.data.length; i++) {
		var dataForPlane = dataSet.data[i];
		var planeId = dataForPlane.name;

        var zoomLevels = eval(dataForPlane.zoomLevels);
		transformationMatrix = eval(dataForPlane.transformationMatrix);
		
		// create extent
		var extent = 
			new TissueStack.Extent(
				dataSet.id,
				dataForPlane.isTiled,
				dataForPlane.oneToOneZoomLevel,
				planeId,
				dataForPlane.maxSlices,
				dataForPlane.maxX,
				dataForPlane.maxY,
				zoomLevels,
				transformationMatrix, 
				dataForPlane.resolutionMm);

		// create canvas
		var plane = new TissueStack.Canvas(extent, "canvas_" + planeId + "_plane", "dataset_1", true);

        // this is a bit of a hack to not have to change the fixed html layout if we have only 1 plane (classic 2D data):
		// in order to use the main view which is hard-coded to plane with id 'y', we'll make the only plane that we have 'y' 
		if (dataSet.data.length == 1) {
            main_view_plane = plane;
			planeId = 'y';
			extent.plane = planeId;
		}
        
		// set the internal db id
		plane.id = dataForPlane.id;
		
		var localHost = document.location.host;
		if (!localHost)
			localHost = dataSet.host;
		
		// set original value range 
		plane.setValueRange(dataForPlane.valueRangeMin, dataForPlane.valueRangeMax);
		
		// set bidirectional relationship for contrast
		plane.contrast = contrast;
		if (contrast) contrast.canvas = plane;
		
		// for scalebar to know its parent
		if (planeId == 'y') {
			main_view_plane = plane;
			plane.is_main_view = true;
		}
		plane.updateScaleBar();
		
		// store plane  
		dataSet.planes[planeId] = plane;
		
		// get the real world coordinates 
		dataSet.realWorldCoords[planeId] = plane.getDataExtent().getExtentCoordinates();
		
		plane.changeToZoomLevel(0);
        plane.eraseCanvasContent();
		
        if (!drawMe) continue;
        
		var now = new Date().getTime();
		
		plane.queue.drawLowResolutionPreview(now);
		plane.queue.drawRequestAfterLowResolutionPreview(null, now);

       if (main_view_plane) {
           main_view_plane.updateExtentInfo(main_view_plane.getDataExtent().getExtentCoordinates());
           main_view_plane.events.updateCoordinateDisplay();
       }
	}
};

TissueStack.BindGlobalEvents = function () {
	// for the desktop, we want to resize stuff once the individual sections have been expanded
	$(".left_panel div[data-role='collapsible']").each(
			function() {
				$(this).bind("collapsibleexpand", function() {
					//TissueStack.Utils.adjustCollapsibleSectionsHeight('ontology_tree', 150);
					TissueStack.Utils.adjustCollapsibleSectionsHeight('treedataset');
				});
				$(this).bind("collapsiblecollapse", function() {
					//TissueStack.Utils.adjustCollapsibleSectionsHeight('ontology_tree', 150);
					TissueStack.Utils.adjustCollapsibleSectionsHeight('treedataset');
				});
			}
	);
	
	// DATA SET SEARCH AND ADDITION
	// avoid potential double binding by un-binding
	$('#server_search_button').unbind("click");
	//rebind
	$('#server_search_button').bind("click", function() {
		var val = $('#server_search_input').val();
		var url = TissueStack.Utils.verifyUrlSyntax(val);
		if (!url) {
			alert("You entered an invalid url!");
			return;
		}
		
		if (typeof(window.location.hostname) == "string" && val.indexOf(window.location.hostname) >=0) {
			alert("Your local instance's configuration is already in the list!");
			return;
		}
		
		$('#server_search_input').val(url);
		
		// get the actual host/domain name
		var domain = TissueStack.Utils.extractHostNameFromUrl(val);
		if (!domain) {
			alert("Could not read domain from url");
			return;
		}
		// replace . with _
		domain = domain.replace(/[.]/g,"_");
		
		if (url.substring(url.length-1) != '/') {
			url += "/";
		}
		
		url += (TissueStack.configuration['server_proxy_path'].value + "/?service=services&sub_service=data&action=all&include_planes=true");
		
		// contact server
		TissueStack.Utils.sendAjaxRequest(
			url, 'GET', true,
			function(data, textStatus, jqXHR) {
				if (!data.response && !data.error) {
					alert("Did not receive anyting, neither success nor error ....");
					return;
				}
				
				if (data.error) {
					var message = "Application Error: " + (data.error.description ? data.error.description : " no more info available. check logs.");
					alert(message);
					return;
				}
				
				if (data.response.noResults) {
					alert("No data sets found in configuration database");
					return;
				}
				
				var dataSets = data.response;
				
				for (var x=0;x<dataSets.length;x++) {
					var addedDataSet = TissueStack.dataSetStore.addDataSetToStore(dataSets[x], domain);
					if (addedDataSet) {
						if(TissueStack.desktop){
							TissueStack.dataSetNavigation.addDataSetToDynaTree(addedDataSet);
						}
						if (TissueStack.tablet || TissueStack.phone){
							TissueStack.dataSetNavigation.addDataSetToTabletTree(addedDataSet);
						}
					}
				}
			},
			function(jqXHR, textStatus, errorThrown) {
				alert("Error connecting to backend: " + textStatus + " " + errorThrown);
			}
		);
	});
};

TissueStack.BindDataSetDependentEvents = function () {
	if (TissueStack.dataSetNavigation.selectedDataSets.count == 0) {
		return;
	}

	// get all data sets that have been selected from the store and stuff them into the array for binding its events
	var datasets = [];
	for (var x=0;x<TissueStack.dataSetNavigation.selectedDataSets.count;x++) {
		var selectedKey = TissueStack.dataSetNavigation.selectedDataSets["dataset_" + (x+1)]; 
		datasets.push(TissueStack.dataSetStore.getDataSetById(selectedKey)); 
	}

	// determine the maximum number of data sets that are displayed. depends on the type of display
	var maxDataSets = (TissueStack.phone || TissueStack.tablet) ? 1 : 2;
	if (maxDataSets > datasets.length) {
		maxDataSets = datasets.length;
	}

	// SYNC AND OVERLAY DATA_SETS CHECKBOX CHANGE HANDLER
    if (TissueStack.desktop) {
   		$('#sync_data_sets').unbind("change");
        $('#sync_data_sets').bind("change", function() {
        	TissueStack.sync_datasets = $('#sync_data_sets')[0].checked;
        	$('#sync_data_sets').checkboxradio("refresh");
        	if (!TissueStack.sync_datasets && TissueStack.overlay_datasets) {
        		TissueStack.overlay_datasets = false;
        		$('#overlay_data_sets').removeAttr("checked").checkboxradio("refresh");
        	}
        	TissueStack.Utils.transitionToDataSetView();
        });
		$('#overlay_data_sets').unbind("change");
        $('#overlay_data_sets').bind("change", function() {
        	TissueStack.overlay_datasets = $('#overlay_data_sets')[0].checked;
    		$('#overlay_data_sets').checkboxradio("refresh");
        	if (TissueStack.overlay_datasets) {
	        	$('#sync_data_sets').attr("checked", "checked").checkboxradio("refresh");
	        	TissueStack.sync_datasets = true;
        	}
        	TissueStack.Utils.transitionToDataSetView();
        });
    }
    
	// DRAWING INTERVAL CHANGE HANDLER
	// avoid potential double binding by un-binding at this stage
	$('#drawing_interval_button').unbind("click");
	//rebind
	$('#drawing_interval_button').bind("click", function() {
		var oldVal = (TissueStack.configuration['default_drawing_interval'] ?
				TissueStack.configuration['default_drawing_interval'].value : 150);
		var val = ($('#drawing_interval') && $('#drawing_interval').length > 0) ?
				parseInt($('#drawing_interval').val()) : oldVal;
		if (typeof(val) != 'number' || isNaN(val)) val = 150;
		if (val < 10 || val > 1000) {
			alert("Please enter a number in between 10ms and 1000ms (=1s)");
			$('#drawing_interval').val(oldVal);
			return false;
		}
				
		for (var x=0;x<maxDataSets;x++) {
			var dataSet = datasets[x];
			
			for (var id in dataSet.planes) {	
				dataSet.planes[id].queue.setDrawingInterval(val);
			}
		}
	});
	
	// now let's bind events that are intimately linked to their own data set
	for (var y=0;y<maxDataSets;y++) {
		var dataSet = datasets[y];

		if (TissueStack.desktop || TissueStack.tablet) {
            // COORDINATE CENTER FUNCTIONALITY FOR DESKTOP
			// avoid potential double binding by un-binding at this stage
			$('#dataset_' + (y+1) + '_center_point_in_canvas').unbind("click");
			// rebind
			$('#dataset_' + (y+1) + '_center_point_in_canvas').bind("click", [{actualDataSet: dataSet,x: y}], function(event) {
				var plane =
					TissueStack.Utils.returnFirstOccurranceOfPatternInStringArray(
							$("#" + (TissueStack.desktop || TissueStack.tablet || TissueStack.phone? "dataset_" + (event.data[0].x + 1) + "_": "") + "main_view_canvas").attr("class").split(" "), "^canvas_");
				if (!plane) {
					return;
				}
				var startPos = "canvas_".length;
				var planeId = plane.substring(startPos, startPos + 1);
				
				var xCoord = parseFloat($('#canvas_point_x').val());
				var yCoord = parseFloat($('#canvas_point_y').val());
				var zCoord = parseFloat($('#canvas_point_z').val());
				
				// this is a hack for 1 plane (simple 2D) data
				// we set z min/max to +/- infinity to pass the test
				if (event.data[0].actualDataSet.data.length == 1) {
					zCoord = 0;
					event.data[0].actualDataSet.realWorldCoords[planeId].min_z = Number.NEGATIVE_INFINITY;
					event.data[0].actualDataSet.realWorldCoords[planeId].max_z = Number.POSITIVE_INFINITY;
				}
				
				if (isNaN(xCoord) || isNaN(yCoord) || isNaN(zCoord)) {
					alert("Illegal coords");
					return;
				}
				
				// if we had a transformation matrix, we know we have been handed in real word coords and therefore need to convert back to pixel
				var givenCoords = {x: xCoord, y: yCoord, z: zCoord};
				plane = event.data[0].actualDataSet.planes[planeId];
				if (plane.getDataExtent().worldCoordinatesTransformationMatrix) {
					givenCoords = plane.getDataExtent().getPixelForWorldCoordinates(givenCoords);
				}

				// this is for slight floating point inaccuracies (+/- 1 slice/pixel)
				givenCoords.x = (givenCoords.x < 0 && givenCoords.x > -1) ? 0 : givenCoords.x;  
				givenCoords.y = (givenCoords.y < 0 && givenCoords.y > -1) ? 0 : givenCoords.y;
				givenCoords.z = (givenCoords.z < 0 && givenCoords.z > -1) ? 0 : givenCoords.z;
				givenCoords.x = 
					(givenCoords.x > event.data[0].actualDataSet.planes[planeId].data_extent.x && 
							givenCoords.x < (event.data[0].actualDataSet.planes[planeId].data_extent.x + 1)) ?
									Math.floor(givenCoords.x) : Math.round(givenCoords.x);
				givenCoords.y = 
					(givenCoords.y > event.data[0].actualDataSet.planes[planeId].data_extent.y && 
							givenCoords.y < (event.data[0].actualDataSet.planes[planeId].data_extent.y + 1)) ?
									Math.floor(givenCoords.y) : Math.round(givenCoords.y);
				givenCoords.z = 
					(givenCoords.z > event.data[0].actualDataSet.planes[planeId].data_extent.max_slices && 
							givenCoords.z < (event.data[0].actualDataSet.planes[planeId].data_extent.max_slices + 1)) ?
									Math.floor(givenCoords.z) : Math.round(givenCoords.z);
									

				if ((event.data[0].actualDataSet.planes[planeId] && (givenCoords.x < 0
						|| givenCoords.x > event.data[0].actualDataSet.planes[planeId].data_extent.x)) 
						|| (event.data[0].actualDataSet.planes[planeId] && (givenCoords.y < 0
								|| givenCoords.y > event.data[0].actualDataSet.planes[planeId].y))
								|| (event.data[0].actualDataSet.planes['z'] && (givenCoords.z < 0
										|| givenCoords.z > event.data[0].actualDataSet.planes[planeId].data_extent.max_slices))	) {
					alert("Illegal coords");
					return;
				}
				
				var now = new Date().getTime();
				plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(givenCoords, true, now, true);
				setTimeout(function() {plane.events.updateCoordinateDisplay();},500);
				
				if (event.data[0].actualDataSet.data.length > 1) {
					var slider = $("#" + (plane.dataset_id == "" ? "" : plane.dataset_id + "_") + "canvas_main_slider");
					if (slider) {
						slider.val(givenCoords.z);
						slider.blur();
						setTimeout(function() {
							plane.events.changeSliceForPlane(givenCoords.z);
							}, 150);
					}
				}
			});

			// if we have only one plangetDataSetByIndexe, we don't need to register maximize or slider 
			if (dataSet.data.length == 1) {
				continue;
			}

			// MAXIMIZING SIDE VIEWS
            TissueStack.ComponentFactory.registerMaximizeEventsForDataSetWidget("dataset_" + (y+1), dataSet);
        }

        // COLOR MAP SWITCHER
        TissueStack.ComponentFactory.initColorMapSwitcher('dataset_' + (y+1), dataSet);
        
        // Z PLANE AKA SLICE SLIDER 
        TissueStack.ComponentFactory.initDataSetSlider("dataset_" + (y+1), dataSet);
        if (!TissueStack.PhoneMenu) TissueStack.ComponentFactory.resizeDataSetSlider(TissueStack.canvasDimensions.height);
	}
};

TissueStack.CommonBootStrap = function() {
    if (!TissueStack.Utils.supportsCanvas()) {
        alert("Your browser does not support the HTML5 feature 'Canvas'!\n\n" +
                "This means that this site will be of very limited use for you.\n\n" +
                "We recommend upgrading your browser: Latest versions of Chrome, Firefox, Safari and Opera support the canvas element," +
                " so does IE from version 9 on.");
    }
      // override cross domain behavior
      var options = {
          allowCrossDomainPages : true
      };
      // override form submission behavior for desktop version
      if (TissueStack.desktop) {
          options.ajaxEnabled = false;
      }

      $.extend(  $.mobile , options);

    TissueStack.Init();
}

if (TissueStack.phone) {
    $(document).bind ("pageinit", function(event) {
        if (TissueStack.initTimeStamp) return;
        
        TissueStack.CommonBootStrap();
        TissueStack.initTimeStamp = event.timeStamp;
    });    
}

if (!TissueStack.phone) {
    $(document).ready(function() {
        TissueStack.CommonBootStrap();
    });
}