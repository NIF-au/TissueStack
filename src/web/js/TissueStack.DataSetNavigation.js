TissueStack.DataSetNavigation = function() {
	if (TissueStack.phone) {
		return;
	} else if (TissueStack.desktop) {
		this.buildDynaTree();
	} else if (TissueStack.tablet) {
		this.buildTabletMenu();
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
		if (TissueStack.tablet) {
			index = 0;
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
			dataSet.planes[plane].events.unbindAllEvents();
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

		/*
		$("#" + dataset + "_canvas_main_slider").slider("destroy");
		$("#" + dataset + "_canvas_main_slider").children().remove();
		$("#" + dataset + "_right_panel").html('<input id="' + dataset + '_canvas_main_slider" class="canvasslider canvas_y" type="range" min="0" max="1000" value="500" step="1" orientation="vertical"  />');
		$("#" + dataset + "_canvas_main_slider").slider();
		*/
		
		var old_classes = $("#" + dataset + "_canvas_main_slider").attr("class");
		old_classes = old_classes.replace(/canvas_[?]/, "canvas_y");
		coll = $("#" + dataset + "_canvas_main_slider");
		coll.removeAttr("class");
		coll.addClass(old_classes);
		$("#" + dataset + "_left_side_view_maximize").attr("class", "maximize_view_icon canvas_x");
		$("#" + dataset + "_right_side_view_maximize").attr("class", "maximize_view_icon canvas_z");
		
		// finally hide everything
	   $('#' + dataset + '_center_point_in_canvas').closest('.ui-btn').hide();
	   $("#" + dataset + ", #" + dataset + "_right_panel").addClass("hidden");
	},
	showDataSet : function(index) {
		if (typeof(index) != "number") {
			return;
		}
		if (TissueStack.desktop && (index < 0 || index > 2)) {
			return;
		}
		if (TissueStack.tablet || TissueStack.phone) {
			index = 0;
		}

		$("#canvas_point_x,#canvas_point_y,#canvas_point_z").removeAttr("disabled");
		$("#dataset_" + index + ", #dataset_" + index + "_right_panel").removeClass("hidden");
		$('#dataset_' + index + '_center_point_in_canvas').closest('.ui-btn').show();
	},
	buildDynaTree : function() {
		var treeData = [];

		if (TissueStack.dataSetStore.getSize() == 0) {
			treeData[0] = {title: "No Data Sets Found", tooltip: "No Data Sets Found"};
		} else {
			var counter = 0;
			for (var dataSetKey in TissueStack.dataSetStore.datasets) {
				var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];
				treeData[counter] =	{
						title: dataSet.local_id + '@' +dataSet.host,
						key: dataSet.id,
						tooltip: (dataSet.description ? dataSet.description : ""),
						select: counter == 0 ? true : false,
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
						select: counter == 0 ? true : false,
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
	    		   
	    			// re-initialize data set handed in
	    			TissueStack.InitUserInterface();
	    			TissueStack.BindDataSetDependentEvents();
	    			
	    			// show everything again
	    		   for (var n=0;n<selectedNodes.length;n++) {
		    			_this.showDataSet(n + 1);
	    		   }
		       }
/*
		       onActivate: function(node) {

			   },
		       onDeactivate: function(node) {

	           },
		       
		       onSelect: function(select, node) {
		         var selNodes = node.tree.getSelectedNodes();
		         var selKeys = $.map(selNodes, function(node){
		              return "[" + node.data.key + "]: '" + node.data.title + "'";
		         });
		         
		         //CONTROL CROSS SHOW/HIDE
		         var selected = node.isSelected();
		         if(node.data.key == "cross_tree_id" && selected == false){
		         	$("#canvas_y_plane_cross_overlay").fadeOut(50);
		         }else if(node.data.key == "cross_tree_id" && selected == true){
		         	$("#canvas_y_plane_cross_overlay").fadeIn(50);
		         }
		         //END
		         	         
		       },
		       onClick: function(node, event) {
		         if( node.getEventTargetType(event) == "title" )
		           node.toggleSelect();
		       },
		       onKeydown: function(node, event) {
		         if( event.which == 32 ) {
		           node.toggleSelect();
		           return false;
		         }
		       },
		       onDblClick: function(node, event) {
		         node.toggleSelect();
		       }
		       */
		  });
	},
	buildTabletMenu : function() {
		var treeData = [];
			if (TissueStack.dataSetStore.getSize() == 0) {
				treeData[0] = {title: "No Data Sets Found", tooltip: "No Data Sets Found"};
			} else {
				var htmlString ="";	
				for (var dataSetKey in TissueStack.dataSetStore.datasets) {
					var dataSet = TissueStack.dataSetStore.datasets[dataSetKey];
						htmlString += '<div data-role="collapsible" data-collapsed="false">' + '<h3>'+ dataSet.local_id + ' in ' + dataSet.host +'</h3>'+
										'<p>'+ dataSet.description +'<br>'+ 'Location: '+ dataSet.filename +'</p>'+
										'<fieldset data-role="controlgroup" data-mini="true">'+
										'<input type="radio" name="radio-' + dataSet.local_id + '"'+' id="radio-'+ dataSet.local_id +'"'+' value="on" checked="checked"/>'+
										'<label for="radio-'+ dataSet.local_id +'"'+'>ON</label>'+
										'<input type="radio" name="radio-' + dataSet.local_id + '"'+' id="radio-off-'+ dataSet.local_id +'"'+' value="off" />'+
										'<label for="radio-off-'+ dataSet.local_id + '"'+'>OFF</label>'+
										'</fieldset></div>';
																												
				}
				$('#tablet_tree').append(htmlString);
				$("#tablet_tree").trigger("create");
			}	
			
			//Notes: your key is the dataSet.id
			// replace
			//this.addToOrReplaceSelectedDataSets(newkey, 0);
			// 3. reinitialize ui and events 
			//TissueStack.InitUserInterface();
			//TissueStack.BindDataSetDependentEvents();
			// show everything again
		   //this.showDataSet(1);
	},
	getDynaTreeObject :function() {
		if (!$("#treedataset") || !$("#treedataset").dynatree) {
			return null;
		}
		
		return $("#treedataset").dynatree("getTree");
	}, getSelectedDynaTreeNodes : function(stopAtParent) {
		if (typeof(stopAtParent) == 'undefined') {
			stopAtParent = true;
		}
		var dynaTree = this.getDynaTreeObject();
		if (!dynaTree) {
			return null;
		}
		
		return dynaTree.getSelectedNodes(stopAtParent);
	}
};		
