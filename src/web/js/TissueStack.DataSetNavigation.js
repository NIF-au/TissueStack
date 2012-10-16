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
		if (typeof(index) != "number") {
			return;
		}
		if (TissueStack.desktop && (index < 0 || index > 1)) {
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
		if (TissueStack.desktop && typeof(index) != "number") {
			return;
		}
		if (TissueStack.desktop && (index < 0 || index > 1)) {
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
		if (TissueStack.desktop && typeof(index) != "number") {
			return;
		}
		if (TissueStack.desktop && (index < 0 || index > 1)) {
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
			dataSet.planes[plane].queue.latestDrawRequestTimestamp = -1;
			dataSet.planes[plane].queue.stopQueue();
			dataSet.planes[plane].events.unbindAllEvents();
			if (dataSet.planes[plane].contrast) dataSet.planes[plane].contrast.unregisterListeners();
		}
		dataSet.planes = {};
		
		// restore plane and canvas order to default
		var coll = $("#" + dataset + "_main_view_canvas");
		coll.removeAttr("class");
		coll.addClass("canvasview canvas_y");
		coll = $("#" + dataset + "_main_view_canvas canvas");
		var selectors = [
		                 ["#" + dataset + "_main_view_canvas canvas", "canvas_y_plane"],
		                 ["#" + dataset + "_left_side_view_canvas canvas", "canvas_x_plane"],
		                 ["#" + dataset + "_right_side_view_canvas canvas", "canvas_z_plane"]
		                ];
			for (var i=0;i<selectors.length;i++) {
				coll = $(selectors[i][0]);
				coll.each(function(index) {
				    var id = $(this).attr("id");
				    $(this).attr("id", id.replace(/canvas_[x|y|z]_plane/, selectors[i][1]));
				});
		}

		// restore some settings 
		try {
			$("#colormap_choice input").removeAttr("checked").checkboxradio("refresh");
			$("#colormap_grey").attr("checked", "checked").checkboxradio("refresh");
		} catch (e) {
			// we don't care, stupid jquery mobile ...
			$("#colormap_grey").attr("checked", "checked");
		}
		
		//reset contrast box
		$("#dataset_1_url_box, #dataset_1_contrast_box").hide("fast");
		$("#dataset_2_url_box, #dataset_2_contrast_box").hide("fast");
		
		
		if(TissueStack.desktop || TissueStack.tablet){
			// restore slider states
			var old_classes = $("#" + dataset + "_canvas_main_slider").attr("class");
			old_classes = old_classes.replace(/canvas_[x|y|z]/, "canvas_y");
			coll = $("#" + dataset + "_canvas_main_slider");
			coll.removeAttr("class");
			coll.addClass(old_classes);

			// restore maximizing states
			$("#" + dataset + "_left_side_view_maximize").attr("class", "maximize_view_icon canvas_x");
			$("#" + dataset + "_right_side_view_maximize").attr("class", "maximize_view_icon canvas_z");
			
			// finally hide everything
		   $('#' + dataset + '_center_point_in_canvas').closest('.ui-btn').hide();
		   $("#" + dataset + ", #" + dataset + "_right_panel").addClass("hidden");
		   $("#" + dataset + "_left_side_view_canvas").addClass("hidden");
		   $("#" + dataset + "_right_side_view_canvas").addClass("hidden");
		}
	},
	showDataSet : function(index) {
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
		$('#dataset_' + index + '_center_point_in_canvas').closest('.ui-btn').show();
		
		var dataSet = 
			TissueStack.dataSetStore.getDataSetById(
					TissueStack.dataSetNavigation.selectedDataSets["dataset_" + index]);
		if (dataSet && dataSet.data.length > 1) {
			$("#dataset_" + index  + "_right_panel").removeClass("hidden");
		}
	},
	buildDynaTree : function() {
		var treeData = [];

		if (TissueStack.dataSetStore.getSize() != 0) {
			var counter = 0;
			for (var dataSetKey in TissueStack.dataSetStore.datasets) {
				var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];
				treeData[counter] =	{
						title: dataSet.local_id + '@' +dataSet.host,
						key: dataSet.id,
						tooltip: (dataSet.description ? dataSet.description : ""),
						select: false,
						isFolder: true,
						expand: true
					};
				// add children
				// TODO: later we add the overlays, for now, the only children are the base layers
				treeData[counter].children = {
						title: dataSet.filename,
						isBaseLayer : true,
						key: dataSet.id + "_base_layer",
						tooltip: (dataSet.description ? dataSet.description : ""),
						select: false,
						expand: false
				};
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
		    			   node.childList[x].select(flag);
		    		   }
		    	   } else if (node.data.isBaseLayer) {
		    		   // likewise if a base layer was selected/deselected, that results in the parent data set not being de/selected
		    		   node.parent.select(flag);
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
	    		   // brief check
	    		   if (selectedNodes.length > 2) {
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
	    		   }

	    		   if (selectedNodes.length == 0) {
	    			   TissueStack.Utils.adjustScreenContentToActualScreenSize(0);
	    			   return;
	    		   }


	    		   // display/hide data sets left / not left
	    		   for (var n=0;n<selectedNodes.length;n++) {
		    		   _this.addToOrReplaceSelectedDataSets(selectedNodes[n].data.key, n);
	    		   }

	    			// show everything again
	    		   for (var n=0;n<selectedNodes.length;n++) {
		    			_this.showDataSet(n + 1);
	    		   }

	    			// re-initialize data set handed in
	    			TissueStack.InitUserInterface();
	    			TissueStack.BindDataSetDependentEvents();
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
	addDataSetToDynaTree : function(dataSet) {
		var tree = this.getDynaTreeObject();
		if (!tree) {
			return;
		}
		
		var root = tree.getRoot();
		var newNode = 
			root.addChild({
				title: dataSet.local_id + '@' +dataSet.host,
				key: dataSet.id,
				tooltip: (dataSet.description ? dataSet.description : ""),
				select: false,
				isFolder: true,
				expand: false
		});
		// add children
		// TODO: later we add the overlays, for now, the only children are the base layers
		newNode.addChild({
				title: dataSet.filename,
				isBaseLayer : true,
				key: dataSet.id + "_base_layer",
				tooltip: (dataSet.description ? dataSet.description : ""),
				select: false,
				expand: false
		});
	},
	addDataSetToTabletTree : function (dataSet) {
		
		$('#tablet_tree').empty();
		$('#tablet_tree').unbind("expand");
		var htmlString ="";

		for (var dataSetKey in TissueStack.dataSetStore.datasets) {
			var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];
			  htmlString += '<div data-role="collapsible"'+' id="tabletTreeDiv-'+ dataSet.local_id + dataSet.host + '"' 
			  			 + 'data-transition="slide"'+'>' + '<h3>'+ dataSet.local_id + ' in ' + dataSet.host +'</h3>'
  			  			 + '<p>'+ dataSet.description +'<br>'+ 'Location: '+ dataSet.filename +'</p>'
  			  			 + '<fieldset data-role="controlgroup" data-mini="true">'
  			  			 + '<input type="radio" name="radio-' + dataSet.local_id + '"'+' id="radio-'+ dataSet.local_id +'"'+' value="on" />'
  			  			 + '<label for="radio-'+ dataSet.local_id +'"'+'>Overlay ON</label>'
  			  			 + '<input type="radio" name="radio-' + dataSet.local_id + '"'+' id="radio-off-'+ dataSet.local_id +'"'
  			  			 + 'value="off" />' + '<label for="radio-off-'+ dataSet.local_id + '"'+'>Overlay OFF</label>'
  			  			 + '</fieldset></div>';		
		}		
		$('#tablet_tree').append(htmlString)
		.trigger("create").controlgroup('refresh', true);

		this.getSelectedTabletTree(dataSet);
	},
	getSelectedTabletTree : function (dataSet) {
		for (var dataSetKey in TissueStack.dataSetStore.datasets) {
			var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];
			(function(dataSet,_this) {
				$("#tabletTreeDiv-" + dataSet.local_id + dataSet.host).trigger("collapse");
				$("#tabletTreeDiv-" + dataSet.local_id + dataSet.host).bind("expand", function() {
					if(TissueStack.phone){
						TissueStack.Utils.adjustScreenContentToActualScreenSize(0);
					}
					_this.addToOrReplaceSelectedDataSets(dataSet.id, 0);
					_this.showDataSet(1);
					TissueStack.InitUserInterface();
					TissueStack.BindDataSetDependentEvents();
					//redirect to x plane after expanded
					if (TissueStack.phone) window.location = document.location.href.split('#dataset')[0] + '#tissueX';
					return;
				});
			})(dataSet, this);
		}
	},
	loadScrollMenu : function () {
		//add touch transition for phone menu and dataset menu so that it can scroll over and see contents.
		var LeftMenu, DataSetMenu, TableMenu;
		
		LeftMenu = new iScroll('menutransition', { useTransition:true });
		if(TissueStack.phone) DataSetMenu = new iScroll('phonetransition', { useTransition:true });
		if(TissueStack.tablet) TableMenu = new iScroll('tablettransition', { useTransition:true }); //For tablet version (Haven't apply yet)

		document.addEventListener('touchmove', function (e) { e.preventDefault(); }, false);
	}, 
	syncDataSetCoordinates : function(canvas) {
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
		var other_ds_id = canvas.dataset_id === 'dataset_1' ? 'dataset_2' : 'dataset_1';
		
		// now fetch the other data set and see if we have a matching canvas or more accurately a matching plane with world coordinates
		var other_ds = TissueStack.dataSetStore.getDataSetById(TissueStack.dataSetNavigation.selectedDataSets[other_ds_id]);
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

		other_plane.redrawWithCenterAndCrossAtGivenPixelCoordinates(pixel_coords_for_other_plane, false);
		other_plane.queue.drawLowResolutionPreview(sync_time);
		other_plane.queue.drawRequestAfterLowResolutionPreview(null, sync_time);
		other_plane.queue.tidyUp();

		if (other_plane.is_main_view) {
			var slider = $("#" + (other_plane.dataset_id == "" ? "" : other_plane.dataset_id + "_") + "canvas_main_slider");
			if (slider) {
				slider.val(pixel_coords_for_other_plane.z);
				slider.blur();
			}
		}
		
		canvas.queue.last_sync_timestamp = -1; // reset 
	}
};		
