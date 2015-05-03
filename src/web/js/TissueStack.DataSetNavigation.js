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
TissueStack.DataSetNavigation = function() {
	if (TissueStack.phone || TissueStack.tablet) {
		this.buildTabletMenu();
		this.loadScrollMenu();
	} else if (TissueStack.desktop) {
		this.buildDynaTree();
	}
};

TissueStack.DataSetNavigation.prototype = {
	selectedDataSets : { 
		count : 0,
		dataset_1 : null,
		dataset_2 : null
	},
	removeAllSelectedDataSets : function() {
		for (var i=0;i<this.selectedDataSets.count;i++) {
			if (this.selectedDataSets["dataset_" + (i+1)]) {
				this.selectedDataSets["dataset_" + (i+1)] = null;
				this.selectedDataSets.count--;
			}
		}
	},
	removeFromSelectedDataSetsByIndex : function(index) {
		if (typeof(index) != "number" || index < 0) {
			return;
		}
		if (TissueStack.tablet ||TissueStack.phone) {
			index = 1;
		}

		var oldEntry = this.selectedDataSets["dataset_" + index];
		if (oldEntry) {
			// old entry exists => unregister dataset
			this.unregisterDataSet(oldEntry, "dataset_" + index);
			this.selectedDataSets[oldEntry] = null;
			this.selectedDataSets.count--;
		}
	},
	removeFromSelectedDataSetsByKey : function(key) {
		if (typeof(key) != "string") {
			return;
		}

		for (var prop in this.selectedDataSets) {
			if (this.selectedDataSets[prop] == key) {
				// old key exists => unregister dataset
				this.unregisterDataSet(key,prop);
				this.selectedDataSets[prop] = null;
				this.selectedDataSets.count--;
			}
		}
	},
	addDataSet : function(key, index) {
		if (typeof(key) != "string") {
			return;
		}
		if (TissueStack.desktop && (typeof(index) != "number" || index < 0)) {
			return;
		} else if (TissueStack.tablet || TissueStack.phone) {
			index = 0;
		}
		
		if (key == this.selectedDataSets["dataset_" + index]) {
			// nothing to do, we have already that very same data set there
			return; 
		} 

		// add the new data set
		this.selectedDataSets["dataset_" + (index+1)] = key;
		this.selectedDataSets.count++;
	},
	addToOrReplaceSelectedDataSets : function(key, index) {
		if (typeof(key) != "string") {
			return;
		}
		if (TissueStack.desktop && (typeof(index) != "number" || index < 0)) {
			return;
		} else if (TissueStack.tablet || TissueStack.phone) {
			index = 0;
		}
		
		if (key == this.selectedDataSets["dataset_" + index]) {
			// nothing to do, we have already that very same data set there
			return; 
		} 

		// remove other selected data sets that use the same key
		this.removeFromSelectedDataSetsByKey(key);
		// remove another/previously selected data set at the same index
		this.removeFromSelectedDataSetsByIndex(index+1);
		
		// add the new data set
		this.selectedDataSets["dataset_" + (index+1)] = key;
		this.selectedDataSets.count++;
	},
	unregisterDataSet : function(key, dataset) {
		if (typeof(key) != "string") {
			return;
		}

		var dataSet = TissueStack.dataSetStore.getDataSetById(key);
		if (!dataSet) {
			return;
		}
        
		// for all canvases, unbind the events
		for (var plane in dataSet.planes) {
			if (dataSet.planes[plane].underlying_canvas) dataSet.planes[plane].underlying_canvas = null; 
			if (dataSet.planes[plane].overlay_canvas) dataSet.planes[plane].overlay_canvas = null;
			dataSet.planes[plane].queue.latestDrawRequestTimestamp = -1;
			dataSet.planes[plane].queue.stopQueue();
			if (dataSet.planes[plane].is_main_view) {
                dataSet.planes[plane].displayPixelValue();
                dataSet.planes[plane].events.updateCoordinateDisplay(true, true);
                dataSet.planes[plane].updateExtentInfo();
            }
			dataSet.planes[plane].events.unbindAllEvents();
			if (dataSet.planes[plane].contrast) dataSet.planes[plane].contrast.unregisterListeners();
			dataSet.planes[plane].overlays = null;
		}
		dataSet.planes = {};
		TissueStack.overlay_values = {};

        if (!TissueStack.phone) {
            TissueStack.ComponentFactory.destroyDataSetWidget(dataset);
            TissueStack.ComponentFactory.destroyDataSetSlider(dataset);
            $('#' + dataset + '_center_point_in_canvas').closest('.ui-btn').hide();

    		// reset overlay over 
            if (!TissueStack.overlay_datasets || (TissueStack.overlay_datasets && !TissueStack.swappedOverlayOrder))
                TissueStack.reverseOverlayOrder = false;

            if (TissueStack.desktop) {
                var ontTree = $("#ontology_tree");
                if (ontTree && ontTree.length > 0 && ontTree.empty) {
                    ontTree.empty();
                }
            }
            $("#canvas_point_value").show();
        } else { // phone
            try {
                $("#colormap_choice input").removeAttr("checked").checkboxradio("refresh");
                $("#colormap_grey").attr("checked", "checked").checkboxradio("refresh");
            } catch (e) {
                // we don't care, stupid jquery mobile ...
                $("#colormap_grey").attr("checked", "checked");
            }
        }
	},
	setDataSetVisibility : function(id, visible) {
        if (typeof(id) != 'string')
            return;
        if (typeof(visible) != 'boolean')
            visible = true;
        
        var index = -1;
        for (var i=0;i<this.selectedDataSets.count;i++) {
		  if (this.selectedDataSets["dataset_" + (i+1)] && 
                this.selectedDataSets["dataset_" + (i+1)] === id) 
          {
              index = i+1;
              break;
          }
		}

        if (index < 1) return;
        
        if (visible)
            $("#dataset_" + index).show();
        else 
            $("#dataset_" + index).hide();
    },
    showDataSet : function(index, overlaid) {
		if (typeof(index) != "number") {
			return;
		}
		if (TissueStack.desktop && (index < 0 || index > 2)) {
			return;
		}
		if (TissueStack.tablet || TissueStack.phone) {
			index = 1;
		}

		$("#canvas_point_x,#canvas_point_y,#canvas_point_z").removeAttr("disabled");
		$("#dataset_" + index).removeClass("hidden");
		$("#dataset_" + index  + "_left_side_view_canvas").addClass("background_canvas");
		$("#dataset_" + index  + "_right_side_view_canvas").addClass("background_canvas");

		if (!overlaid || (overlaid && index != 1)) {
			$('#dataset_' + index + '_center_point_in_canvas').closest('.ui-btn').show();
			$("#dataset_" + index  + "_color_map").removeClass("hidden");
		}
		$("#dataset_" + index  + " .cross_overlay").removeClass("hidden");
		$("#dataset_" + index  + " .side_canvas_cross_overlay").removeClass("hidden");		
		
		// we keep the slider and the cross-hair hidden for overlaid data sets
		var ds_id = TissueStack.dataSetNavigation.selectedDataSets['dataset_' + index];
		if (ds_id)
		{
			var ds = TissueStack.dataSetStore.getDataSetById(ds_id);
			if (ds && (!ds.is2DData || (ds.is2DData && ds.data[0].maxSlices > 0)))
                $("#dataset_" + index  + "_right_panel").removeClass("hidden");
		}
		
		this.handleOverlaidDataSets(index, overlaid);
	},
	handleOverlaidDataSets : function(index, overlaid) {
		if (overlaid && index == 1) {
			$("#dataset_" + index  + "_right_panel").addClass("hidden");
			$("#dataset_" + index  + " .cross_overlay").addClass("hidden");
			$("#dataset_" + index  + " .side_canvas_cross_overlay").addClass("hidden");
		} else if (overlaid && index ==2)   {
			$("#dataset_" + index  + " .tile_count_div").css("bottom", "40px");
			$("#dataset_" + index  + "_left_side_view_canvas").removeClass("background_canvas");
			$("#dataset_" + index  + "_right_side_view_canvas").removeClass("background_canvas");
		}
	},
	toggleTiles : function(node, flag) {
		if (!node.data.tiled && node.data.isBaseLayer)
			alert("You are switching to tile serving. Should you fail to see (intact) images, it could be that your data set has not been pre-tiled or only partially!");
		
		// toggle flag for overlays only tree related flag, for base layers including global dataStore flags and present planes 
		node.data.tiled = (flag ? false : true);
		
		if (node.data.isBaseLayer) {
			var dataset = TissueStack.dataSetStore.getDataSetById(node.parent.data.key);
			if (dataset && dataset.data) {
				for (var i=0; i< dataset.data.length;i++) { 
					dataset.data[i].isTiled = node.data.tiled;
				}
				if (dataset.planes)
					var k = 0;
					for (k in dataset.planes) {
						if (node.data.tiled) dataset.planes[k].eraseCanvasContent();
						dataset.planes[k].data_extent.is_tiled = node.data.tiled;
						dataset.planes[k].drawMe(new Date().getTime()); // force redraw
					}
				
				TissueStack.admin.togglePreTilingFlag(dataset.local_id, node.data.tiled);
			}
		}

		// flip icon
		if (node.data.tiled)
			node.data.icon = "pre_tiled.png";
		else
			node.data.icon = "image_service.png";
		
		node.render();
	},
	bindDynaTreeContextMenu : function(span) {
		if (!TissueStack.desktop) return;
		var myTreeContext = $("#dynaTreeContextMenu");
		if (!myTreeContext) return;
		
		var _this = this;
		
		$(span).contextMenu({menu: "dynaTreeContextMenu"}, function(action, el, pos) {
			var node = _this.getDynaTreeNode(el);

			switch( action ) {
				case "toggleTiling":
					if (node.data.isFolder || node.data.isBaseLayer) {
						if (node.data.isBaseLayer) node = node.parent;
						for (var i=0; i< node.childList.length;i++) { 
							_this.toggleTiles(node.childList[i], node.data.tiled);
						}
						node.data.tiled = (node.data.tiled ? false : true);
					} else if (node.data.isOverlay) {
						alert("Overlays cannot be changed!");
					}
				break;
			}
		});
	},
    checkCoordinateCompatibility : function(worldCoordinatesOne, worldCoordinatesTwo) {
        if (!worldCoordinatesOne || !worldCoordinatesTwo)
            return false;
        
        var plane = null;
        for (var p in worldCoordinatesOne) { // find matching plane
            if (!worldCoordinatesTwo[p]) {
                plane = null;
                break;
            }
            plane = p;
        }
        if (!plane) 
            return false;

        //check coords: if one data set is not fully contained int the other, we'll return false
        if (typeof(worldCoordinatesOne[plane].min_x) != 'number' || typeof(worldCoordinatesTwo[plane].min_x) != 'number' ||
            typeof(worldCoordinatesOne[plane].max_x) != 'number' || typeof(worldCoordinatesTwo[plane].max_x) != 'number' ||
            typeof(worldCoordinatesOne[plane].min_y) != 'number' || typeof(worldCoordinatesTwo[plane].min_y) != 'number' ||
            typeof(worldCoordinatesOne[plane].max_y) != 'number' || typeof(worldCoordinatesTwo[plane].max_y) != 'number' ||
            typeof(worldCoordinatesOne[plane].min_z) != 'number' || typeof(worldCoordinatesTwo[plane].min_z) != 'number' ||
            typeof(worldCoordinatesOne[plane].max_z) != 'number' || typeof(worldCoordinatesTwo[plane].max_z) != 'number' ||
            !(
                (
                    worldCoordinatesOne[plane].min_x >= worldCoordinatesTwo[plane].min_x && worldCoordinatesOne[plane].max_x <= worldCoordinatesTwo[plane].max_x && 
                    worldCoordinatesOne[plane].min_y >= worldCoordinatesTwo[plane].min_y && worldCoordinatesOne[plane].max_y <= worldCoordinatesTwo[plane].max_y &&
                    worldCoordinatesOne[plane].min_z >= worldCoordinatesTwo[plane].min_z && worldCoordinatesOne[plane].max_z <= worldCoordinatesTwo[plane].max_z
                )
              ||
                (
                    worldCoordinatesTwo[plane].min_x >= worldCoordinatesOne[plane].min_x && worldCoordinatesTwo[plane].max_x <= worldCoordinatesOne[plane].max_x &&
                    worldCoordinatesTwo[plane].min_y >= worldCoordinatesOne[plane].min_y && worldCoordinatesTwo[plane].max_y <= worldCoordinatesOne[plane].max_y &&
                    worldCoordinatesTwo[plane].min_z >= worldCoordinatesOne[plane].min_z && worldCoordinatesTwo[plane].max_z <= worldCoordinatesOne[plane].max_z
                )
            ))
            return false;

        return true;
        
    },
	buildDynaTree : function() {
		var treeData = [];

		if (TissueStack.dataSetStore.getSize() != 0) {
			var counter = 0;
			for (var dataSetKey in TissueStack.dataSetStore.datasets) {
				var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];
				treeData[counter] =	{
						title: (dataSet.description ? dataSet.description : (dataSet.local_id + '@' +dataSet.host)),
						key: dataSet.id,
						tooltip: (dataSet.description ? dataSet.description : (dataSet.local_id + '@' +dataSet.host)),
						select: false,
						isFolder: true,
						expand: counter == 0 ? true : false,
						tiled: dataSet.data[0].isTiled,
						icon: "dataset.png"
					};
				var children = [];
				children.push( // add base layer
						{
							title: 	this.stripFileNameOfDataDirectory(dataSet.filename),
							isBaseLayer : true,
							key: dataSet.id + "_base_layer",
							tooltip: this.stripFileNameOfDataDirectory(dataSet.filename),
							select: false,
							expand: false,
							tiled: dataSet.data[0].isTiled,
							icon: dataSet.data[0].isTiled ? "pre_tiled.png" : "image_service.png"
						});
				// add overlays (if exist)
				if (dataSet.overlays)
					for (var i=0;i<dataSet.overlays.length;i++)
						children.push(
								{
									title: 	dataSet.overlays[i].name,
									isOverlay : true,
									key: dataSet.id + "_overlay_" + i,
									tooltip: dataSet.overlays[i].type,
									select: false,
									expand: false,
									tiled: dataSet.data[0].isTiled,
									icon: dataSet.data[0].isTiled ? "pre_tiled.png" : "image_service.png"
								}
						);
				treeData[counter].children = children;
				counter++;
			}
		}

		// for closures
		_this = this;
		
		// create dyna tree and bind events 
		 $("#treedataset").dynatree({
		     checkbox: true,
		       selectMode: TissueStack.desktop ? 2 : 1,
		       children: treeData,
		       onSelect : function(flag, node) {
		    	   // if parent is selected, also select direct children. no deeper recursion needed so far.
		    	   if (node.data.isFolder && node.childList) {
		    		   for (var x=0;x<node.childList.length;x++) {
		    			   if (flag && node.childList[x].data.isOverlay) // don't switch on overlays if baselayer or folder is selected
		    				   continue;
	    			   		node.childList[x].select(flag);
		    		   }
		    	   } else if (node.data.isBaseLayer) {
		    		   // likewise if a base layer was selected/deselected, that results in the parent data set not being de/selected
		    		   node.parent.select(flag);
		    		   return;
		    	   } else if (node.data.isOverlay) {
		    		   if (flag) node.parent.select(flag); // we don't turn everything off overlay is deselected
		    		   
		    		   var dataSet = TissueStack.dataSetStore.getDataSetById(node.parent.data.key);
		    		   var overlayNumber = parseInt(node.data.key.substring(node.data.key.lastIndexOf("_")+1));
		    		   if (dataSet && dataSet.planes)
		    			   for (var p in dataSet.planes)
		    				   if (dataSet.planes[p].overlays)
		    					   for (var y=0;y<dataSet.planes[p].overlays.length;y++) {
		    						   if (y != overlayNumber) continue;
		    						   if (flag)
	    								   dataSet.planes[p].overlays[y].select();
		    						   else 
		    							   dataSet.planes[p].overlays[y].deselect();
		    					   }
		    		   return;
		    	   }
		    	   
		    	   // now as for the action we need to take, evaluate whether we have a check/uncheck of a data set
		    	   // but before receive key from node 
		    	   var key = node.data.isFolder ? node.data.key : node.data.key.substring(0, node.data.key.length - "_base_layer".length);
		    	   if (!flag) {
		    		   // remove dataset in question
		    		   _this.removeFromSelectedDataSetsByKey(key);
		    	   }

	    		   var selectedNodes = this.getSelectedNodes(true);
	    		   if (TissueStack.overlay_datasets && selectedNodes && selectedNodes.length > 1 && TissueStack.reverseOverlayOrder)
	    			    selectedNodes.reverse();
                    
                    // transparency wheel handler
                    var transparencyHandler = 
                        function (v) { 
                            TissueStack.transparency = (parseInt(v) / 100);
                            var ds = TissueStack.dataSetNavigation.selectedDataSets["dataset_" + selectedNodes.length];
                            if (!ds) return;
                            var actualDs = TissueStack.dataSetStore.getDataSetById(ds);
                            if (actualDs && actualDs.planes) {
                                var sync_time = new Date().getTime();
                                for (var p in actualDs.planes) {
                                    (function(pl) {
                                        actualDs.planes[pl].has_been_synced = true;
                                        actualDs.planes[pl].queue.latestDrawRequestTimestamp = sync_time;            					
                                        setTimeout(function() {
                                            actualDs.planes[pl].queue.drawLowResolutionPreview(sync_time);
                                            actualDs.planes[pl].queue.drawRequestAfterLowResolutionPreview(null, sync_time);
                                        },actualDs.planes[pl].queue.drawingIntervalInMillis);
                                    })(p);
                                }
                            }
                        };

                   var dataSetSwapperHandler = 
                       function() {
                                TissueStack.reverseOverlayOrder = TissueStack.reverseOverlayOrder ? false : true;
                                TissueStack.swappedOverlayOrder = true;
                                TissueStack.Utils.transitionToDataSetView();
                       };
                   
	    		   // brief check
	    		   if (/*!TissueStack.overlay_datasets && */ selectedNodes.length > 2) {
	    			   // we cannot display more than 2 data sets ... let the user know
	    			   alert("Please deselect a data set before you select a different one");
	    			   
	    			   node._select(false, false, true);
	    			   if (node.data.isFolder && node.childList) {
			    		   for (var x=0;x<node.childList.length;x++) {
			    			   node.childList[x]._select(false, false, true);
			    		   }
			    	   } else if (node.data.isBaseLayer) {
			    		   // likewise if a base layer was selected/deselected, that results in the parent data set not being de/selected
			    		   node.parent._select(false);
			    	   }	    			   
	    			   return;
	    		   } /* else if (TissueStack.overlay_datasets && selectedNodes.length > 2)
                       alert("To overlay more than 2 datasets is not yet properly implemented!!!!!"); */

	    		   if (selectedNodes.length == 0) {
	    			   TissueStack.Utils.adjustScreenContentToActualScreenSize(0);
	    			   return;
	    		   }

                   // used to check in case of sync/overlay if coordinates are compatible
                   var previousWorldCoordinates = null;
                   var coordinatesCompatible = true;
                   
	    		   // display/hide data sets left / not left
	    		   for (var n=0;n<selectedNodes.length;n++) {
                       var selectedDataSetKey = selectedNodes[n].data.key;
		    		   _this.addToOrReplaceSelectedDataSets(selectedDataSetKey, n);
                       var dataSetSelected =
                           TissueStack.dataSetStore.getDataSetById(selectedDataSetKey);
                       
                       var dataSetOrdinal = n+1;
                       var dsDiv = "dataset_" + dataSetOrdinal;
                       TissueStack.ComponentFactory.createDataSetWidget(
                           "datasets", dataSetSelected, dataSetOrdinal, null, true, true, !dataSetSelected.data[0].isTiled);
                       TissueStack.ComponentFactory.createDataSetSlider("datasets", dataSetSelected, dataSetOrdinal, "y");
                       TissueStack.ComponentFactory.addProgressBar(dsDiv, dataSetSelected);
                       TissueStack.ComponentFactory.createColorMapSwitcher(dsDiv);
                       TissueStack.ComponentFactory.createUrlLink(dsDiv);
                       TissueStack.ComponentFactory.createContrastSlider(dsDiv, dataSetSelected);
                       TissueStack.ComponentFactory.applyUserParameters({"plane" : "y"}, dataSetSelected);
                       
                       // last one gets the transparency wheel if overlays
                       if (TissueStack.overlay_datasets && selectedNodes.length > 1 && (n+1) == selectedNodes.length) {
                           TissueStack.ComponentFactory.addTransparencyWheel(dsDiv);
                           TissueStack.ComponentFactory.initTransparencyWheel(transparencyHandler);
                           TissueStack.ComponentFactory.addDataSetSwapper(dsDiv);
                           TissueStack.ComponentFactory.initDataSetSwapper(dataSetSwapperHandler);
                       }

                      // check if coordinates are compatible
                       if (coordinatesCompatible && previousWorldCoordinates &&
                           (TissueStack.overlay_datasets || TissueStack.sync_datasets) && selectedNodes.length > 1) {
                            coordinatesCompatible = _this.checkCoordinateCompatibility(previousWorldCoordinates, dataSetSelected.realWorldCoords);
                        }
                        previousWorldCoordinates = dataSetSelected.realWorldCoords;

                       _this.showDataSet(n + 1, TissueStack.overlay_datasets && selectedNodes.length > 1);
	    		   }
                   // adjust to screen size
                   TissueStack.Utils.adjustScreenContentToActualScreenSize(selectedNodes.length);
                   
                   if (!coordinatesCompatible)
                       alert("WARNING: Your datasets' coordinates dont't seem to be compatible for overlaying!!!");
                   
					// set new canvas dimensions
					for (var i=0;i<selectedNodes.length;i++) {
						var dataSet = TissueStack.dataSetStore.getDataSetById(selectedNodes[i].data.key);
                        TissueStack.ComponentFactory.redrawDataSet(dataSet);
					}

                   // link previous and succeeding ds
                   _this.setOverAndUnderLaysForNodes(selectedNodes);
                   
                   // bind events
	    		   TissueStack.BindDataSetDependentEvents();
                   TissueStack.Utils.adjustBorderColorWhenMouseOver();	
		       },
		       onCreate : function(node, span) {
		    	   _this.bindDynaTreeContextMenu(span);
		       },
		       onClick: function(node, event) {
					if( $(".contextMenu:visible").length > 0 )	$(".contextMenu").hide();
				},
				onKeydown: function(node, event) {
					// Eat keyboard events, when a menu is open
					if( $(".contextMenu:visible").length > 0 )
						return false;
				},
            dnd: {
                preventVoidMoves: true, // Prevent dropping nodes 'before self', etc.
                onDragStart: function(node) {
                    return true;
                },
                onDragEnter: function(node, sourceNode) {
                    if(node.parent !== sourceNode.parent) return false;
        
                    // Don't allow dropping *over* a node (would create a child)
                    return ["before", "after"];
                },
                onDrop: function(node, sourceNode, hitMode, ui, draggable) {
                    sourceNode.move(node, hitMode);

                    if (node.tree && node.tree.getSelectedNodes(true).length > 1) {
                        setTimeout(function() {
                            sourceNode.toggleSelect();}, 0);

                        setTimeout(function() {
                            sourceNode.toggleSelect();}, 50);
                    }
                }
            }
             
 		  });
	},
	buildTabletMenu : function() {
		//BUILD FISRT TABLET TREE WHEN BROWSER LOAD
		this.addDataSetToTabletTree();
	},
	getDynaTreeObject :function() {
		if (!$("#treedataset") || !$("#treedataset").dynatree) {
			return null;
		}
		
		return $("#treedataset").dynatree("getTree");
	}, 
	getSelectedDynaTreeNodes : function(stopAtParent) {
		if (typeof(stopAtParent) == 'undefined') {
			stopAtParent = true;
		}
		var dynaTree = this.getDynaTreeObject();
		if (!dynaTree) {
			return null;
		}
		if (TissueStack.desktop){
			return dynaTree.getSelectedNodes(stopAtParent);
		}
	},
	getDynaTreeNode :function(node) {
		if (!$("#treedataset") || !$("#treedataset").dynatree || !node) {
			return null;
		}
		
		return $.ui.dynatree.getNode(node);
	}, 	
	addDataSetToDynaTree : function(dataSet) {
		var tree = this.getDynaTreeObject();
		if (!tree) {
			return;
		}
		
		var root = tree.getRoot();
		var newNode = 
			root.addChild({
				title: (dataSet.description ? dataSet.description : (dataSet.local_id + '@' +dataSet.host)),
				key: dataSet.id,
				tooltip: (dataSet.description ? dataSet.description : (dataSet.local_id + '@' +dataSet.host)),
				select: false,
				isFolder: true,
				expand: false,
				tiled: dataSet.data[0].isTiled,
				icon: "dataset.png"
		});
		var children = [];
		children.push( // add base layer
				{
					title: 	this.stripFileNameOfDataDirectory(dataSet.filename),
					isBaseLayer : true,
					key: dataSet.id + "_base_layer",
					tooltip: this.stripFileNameOfDataDirectory(dataSet.filename),
					select: false,
					expand: false
				});
		newNode.addChild({
			title: this.stripFileNameOfDataDirectory(dataSet.filename),
			isBaseLayer : true,
			key: dataSet.id + "_base_layer",
			tooltip: this.stripFileNameOfDataDirectory(dataSet.filename),
			select: false,
			expand: false,
			tiled: dataSet.data[0].isTiled,
			icon: dataSet.data[0].isTiled ? "pre_tiled.png" : "image_service.png"
		});

		// add overlays (if exist)
		if (dataSet.overlays)
			for (var i=0;i<dataSet.overlays.length;i++)
				newNode.addChild(
						{
							title: 	dataSet.overlays[i].name,
							isOverlay : true,
							key: dataSet.id + "_overlay_" + i,
							tooltip: dataSet.overlays[i].type,
							select: false,
							expand: false,
							tiled: dataSet.data[0].isTiled,
							icon: dataSet.data[0].isTiled ? "pre_tiled.png" : "image_service.png"
						}
				);
	},
	addDataSetToTabletTree : function (dataSet) {
		
		$('#treedataset').empty();
		$('#treedataset').unbind("expand");
		var htmlString ="";

		
		for (var dataSetKey in TissueStack.dataSetStore.datasets) {
			var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];

			htmlString += '<div data-role="collapsible"'+' id="tabletTreeDiv-'+ dataSet.local_id + '-' + dataSet.host + '"' 
			  			 + 'data-transition="slide">' + '<h3>'+ dataSet.description +'</h3>'
  			  			 + '<p><span style="text-decoration:underline;">' + dataSet.description + '</span><br>[' + dataSet.local_id + '@' + dataSet.host +']<br>('+ this.stripFileNameOfDataDirectory(dataSet.filename) +')</p>'
  			  			 + '</div>';
		}		
		var treeSet = $('#treedataset').append(htmlString);
		treeSet.trigger("create").controlgroup();
        try {
            treeSet.controlgroup('refresh', true);
        } catch(err)
        {
			// ignored
        }

		var _this = this;
		for (var dataSetKey in TissueStack.dataSetStore.datasets) {
			var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];
			(function(dataSet,_this) {
                $("#tabletTreeDiv-" + dataSet.local_id + '-' + dataSet.host).trigger("collapsiblecollapse");
				$("#tabletTreeDiv-" + dataSet.local_id + '-' + dataSet.host).bind("collapsibleexpand", function(event) {
                    for (var oneDsKey in TissueStack.dataSetStore.datasets) // collapse all others
                    {
                        var oneDs = TissueStack.dataSetStore.datasets[oneDsKey];
                        if (event.target.id != ("tabletTreeDiv-" + oneDs.local_id + "-" + oneDs.host))
                            $("#tabletTreeDiv-" + oneDs.local_id + "-" + oneDs.host).collapsible( "option", "collapsed", true );
                    }
                        
					if(TissueStack.phone){
						TissueStack.Utils.adjustScreenContentToActualScreenSize(0);
					}
                    
                    _this.addToOrReplaceSelectedDataSets(dataSet.id, 0);
                    
                    if (TissueStack.tablet) {
                        var dataSetOrdinal = 1;
                        var dsDiv = "dataset_" + dataSetOrdinal;
                        TissueStack.ComponentFactory.createDataSetWidget(
                           "datasets", dataSet, dataSetOrdinal, null, true, true, !dataSet.data[0].isTiled);
                        TissueStack.ComponentFactory.createDataSetSlider("datasets", dataSet, dataSetOrdinal, "y");
                        TissueStack.ComponentFactory.addProgressBar(dsDiv, dataSet);
                        TissueStack.ComponentFactory.createColorMapSwitcher(dsDiv);
                        TissueStack.ComponentFactory.createUrlLink(dsDiv);
                        TissueStack.ComponentFactory.createContrastSlider(dsDiv, dataSet);
                        TissueStack.ComponentFactory.applyUserParameters({"plane" : "y"}, dataSet);
                    }
                       
					_this.showDataSet(1);
                    
                    if (TissueStack.phone)
					   TissueStack.InitPhoneUserInterface();
                    
					TissueStack.BindDataSetDependentEvents();
					
                    if (TissueStack.tablet) {
                        TissueStack.Utils.adjustScreenContentToActualScreenSize(1);
                        TissueStack.ComponentFactory.redrawDataSet(dataSet);
					}
                    
                    //redirect to x plane after expanded
					if (TissueStack.phone) window.location = document.location.href.split('#dataset')[0] + '#tissueY';
					return;
				});
                
				$("#tabletTreeDiv-" + dataSet.local_id + '-' + dataSet.host).bind("collapsiblecollapse", function(event) {
                    if (!TissueStack.desktop) 
                        TissueStack.dataSetNavigation.removeFromSelectedDataSetsByKey(dataSet.id);
                });
                
			})(dataSet, this);
		}
	},
	loadScrollMenu : function () {
		//add touch transition for phone menu and dataset menu so that it can scroll over and see contents.
		var LeftMenu, DataSetMenu, TableMenu, PhoneColorMapMenu;
		
		if(TissueStack.tablet) {
			LeftMenu = new iScroll('menutransition', { checkDOMChanges: true, useTransition:true }); //dataset menu
			TableMenu = new iScroll('tablettransition', { checkDOMChanges: true, useTransition:true }); //admin task menu
		}
		if(TissueStack.phone) {
			PhoneColorMapMenu = new iScroll('phone_colormap_transition', { checkDOMChanges: true, bounceLock: false }); //colormap menu
			DataSetMenu = new iScroll('phonetransition', { useTransition:true }); //setting menu
		}
		
		document.addEventListener('touchmove', function (e) { e.preventDefault(); }, false);
	}, 
	syncDataSetCoordinates : function(canvas, timestamp, eraseCanvas) {
		// basic checks whether the sync flag for desktop was set && we have more than 1 data sets selected 
		// && we have been handed in a canvas with all the properties we need to go on 
		if (!(
				TissueStack.dataSetNavigation.selectedDataSets.count > 1 && TissueStack.desktop && TissueStack.sync_datasets 
				&& typeof(canvas) === 'object' && typeof(canvas.dataset_id) === 'string'  
				&& typeof(canvas.getRelativeCrossCoordinates) === 'function' && typeof(canvas.data_extent) === 'object'
				&& typeof(canvas.data_extent.worldCoordinatesTransformationMatrix) === 'object' 
		)) return;

		// IMPORTANT !!!! we ALSO check whether we have been synced. we don't wanna trigger an ifinited syncing fest
		if (typeof(canvas.has_been_synced) != 'boolean' || canvas.has_been_synced) return;
		
		// create a common sync timestamp for all planes if it does not already exist
		var dataSet = TissueStack.dataSetStore.getDataSetById(TissueStack.dataSetNavigation.selectedDataSets[canvas.dataset_id]);
		var sync_time = canvas.queue.last_sync_timestamp < 0 ? new Date().getTime() : canvas.queue.last_sync_timestamp;
		for (var p in dataSet.planes) dataSet.planes[p].queue.last_sync_timestamp = sync_time;
		
		// extract the data set id from the given canvas and see if there is another data set displayed. if not => nothing to do and good bye
		//var other_ds_id = canvas.dataset_id === 'dataset_1' ? 'dataset_2' : 'dataset_1';
		
        for (var selectedDs in TissueStack.dataSetNavigation.selectedDataSets) {
            if (!selectedDs || !TissueStack.dataSetNavigation.selectedDataSets[selectedDs] || selectedDs == "count" || selectedDs == canvas.dataset_id)
                continue;
            
            // now fetch the other data set and see if we have a matching canvas or more accurately a matching plane with world coordinates
            var other_ds = TissueStack.dataSetStore.getDataSetById(TissueStack.dataSetNavigation.selectedDataSets[selectedDs]);
            if (typeof(other_ds) != 'object' || typeof(other_ds.planes) != 'object'
                ||  typeof(other_ds.planes[canvas.data_extent.plane]) != 'object'
                ||  typeof(other_ds.planes[canvas.data_extent.plane].data_extent.worldCoordinatesTransformationMatrix) != 'object') return;

            var other_plane = other_ds.planes[canvas.data_extent.plane];

            // we've got all we need, now let's transform the pixel coords of the given canvas into real world coords
            // so that we can then transform it back for the other data set using ITS real world transformation matrix
            // this will potentially give us out-of-bounds results for non-mapping/overlapping coordinate systems but what the heck,
            // the call to redraw will be able to handle it
            var pixel_coords_for_handed_in_plane = canvas.getRelativeCrossCoordinates();
            pixel_coords_for_handed_in_plane.z = canvas.getDataExtent().slice;
            var real_world_coords_for_handed_in_plane = canvas.getDataExtent().getWorldCoordinatesForPixel(pixel_coords_for_handed_in_plane);
            var pixel_coords_for_other_plane = other_plane.getDataExtent().getPixelForWorldCoordinates(real_world_coords_for_handed_in_plane);

            // THIS IS VITAL TO AVOID an infinite sync chain!!!
            for (var p in other_ds.planes) {
                other_ds.planes[p].has_been_synced = true;
                other_ds.planes[p].queue.last_sync_timestamp = -1;
            }

            if (eraseCanvas) {
                other_plane.eraseCanvasContent();
            } else {
                // sync coordinate adjustment now absolute, not relative
                if (canvas.upper_left_x + canvas.data_extent.x <= canvas.cross_x)
                    pixel_coords_for_other_plane.x = Math.abs(pixel_coords_for_other_plane.x) + other_plane.data_extent.x;
                if (canvas.upper_left_x > canvas.cross_x) {
                    pixel_coords_for_other_plane.x = canvas.cross_x - canvas.upper_left_x
                    if (TissueStack.overlay_datasets) {
                        pixel_coords_for_other_plane.x = canvas.cross_x - canvas.upper_left_x
                        if (canvas.getDataExtent().zoom_level != other_plane.getDataExtent().zoom_level) {
                            pixel_coords_for_other_plane.y = canvas.upper_left_y - canvas.cross_y;
                            other_plane.changeToZoomLevel(canvas.getDataExtent().zoom_level);
                        }
                    }
                }
                other_plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(
                        pixel_coords_for_other_plane, false, timestamp);
                if (TissueStack.overlay_datasets && canvas.getDataExtent().zoom_level != other_plane.getDataExtent().zoom_level)
                    other_plane.changeToZoomLevel(canvas.getDataExtent().zoom_level);
                other_plane.drawMe(timestamp);

                if (other_plane.is_main_view) {
                    var slider = $("#" + (other_plane.dataset_id == "" ? "" : other_plane.dataset_id + "_") + "canvas_main_slider");
                    if (slider && slider.val() != pixel_coords_for_other_plane.z)
                        slider.val(pixel_coords_for_other_plane.z);
                        slider.blur();
                }
                setTimeout(function() {
                    other_plane.queue.tidyUp();
                }, 250);
            }
        }
		canvas.queue.last_sync_timestamp = -1; // reset 
	},
	stripFileNameOfDataDirectory : function(filename) {
		if (!filename) return null;
		
		var file = 	filename;
		if (typeof(TissueStack.configuration['data_directory']) == 'object' 
				&& typeof(TissueStack.configuration['data_directory'].value) == 'string') {
			var pos = file.indexOf(TissueStack.configuration['data_directory'].value);
			if (pos >=0)
				file = file.substring(TissueStack.configuration['data_directory'].value.length);
			if (file[0] == '/')
				file = file.substring(1);
		}
		return file;
	},
    setOverAndUnderLaysForNodes : function(selectedNodes) {
        if (!TissueStack.overlay_datasets || !TissueStack.desktop) return;
        
        if (!selectedNodes || !selectedNodes.length || selectedNodes.length < 2) return;
        
        var numberOfSelectedNodes = selectedNodes.length;
        
        for (var i=0;i<numberOfSelectedNodes;i++) {
            var dataSet = TissueStack.dataSetStore.getDataSetById(selectedNodes[i].data.key);
            
            if (i>0 && i<numberOfSelectedNodes) { // underlying data sets
                var prevDataSet = TissueStack.dataSetStore.getDataSetById(selectedNodes[i-1].data.key);
                if (prevDataSet)
                    for (var p in dataSet.planes)
                        dataSet.planes[p].underlying_canvas = 
                            prevDataSet.planes[p];
            }

            if (i < numberOfSelectedNodes - 1) { // overlying data sets
                var succDataSet = TissueStack.dataSetStore.getDataSetById(selectedNodes[i+1].data.key);
                if (succDataSet)
                    for (var p in dataSet.planes)
                        dataSet.planes[p].overlay_canvas = 
                            succDataSet.planes[p];
            }
        }
    }
};		
