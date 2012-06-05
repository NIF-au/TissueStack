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
	buildDynaTree : function() {
		//NOTE: key has to include local and remote id
		//ALSO: in the first version, only allow 1 data set to be viewed at once !! 
		
		//CREATE DATA SET TREE 
		var treeData = [
		    {title: "CROSS", key: "cross_tree_id", select: true, tooltip: "cross overlay" },
		    {title: "CANVAS TILES", key:"canvas_tree_id", select: true },
		    {title: "REGIONS", isFolder: true, key: "id3", expand: true,
		      children: [
		        {title: "REGIONS X", key: "regions_tree_id", 
		          children: [
		            {title: "Sub-item 3.1.1", key: "regions_tree_x1.1" },
		            {title: "Sub-item 3.1.2", key: "regions_tree_x1.2" }
		          ]
		        },
		        {title: "REGIONS Y",
		          children: [
		            {title: "Sub-item 3.2.1", key: "regions_tree_y1.1" },
		            {title: "Sub-item 3.2.2", key: "regions_tree_y1.2" }
		          ]
		        },
		        {title: "REGIONS Z",
		          children: [
		            {title: "Sub-item 3.2.1", key: "regions_tree_y1.1" },
		            {title: "Sub-item 3.2.2", key: "regions_tree_y1.2" }
		          ]
		        } 
		      ]
		    },
		    {title: "REMOTE SERVER", key: "remote_server_id", expand: true,
		      children: [
		        {title: "FISH TILE",
		          children: [
		            {title: "FISH TILE 1", key: "remote_server_fish1" },
		            {title: "FISH TILE 2", key: "remote_server_fish2" }
		          ]
		        },
		        {title: "MOUSE TILE", expand: true,
		          children: [
		            {title: "MOUSE TILE 1", key: "remote_server_mouse1", select: true },
		            {title: "MOUSE TILE 2", key: "remote_server_mouse2" }
		          ]
		        },
		        {title: "Sub-item 4.3", hideCheckbox: true },
		        {title: "Sub-item 4.4", hideCheckbox: true }
		      ]
		    }
		  ];
		 // TRIGGER DATA SET FUNCTION 
		 $("#treedataset").dynatree({
		       checkbox: true,
		       selectMode: 2,
		       children: treeData,

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
		       },
		  });
		//FUTURE USE USING JSON TO LOAD DATA SET TREE	  
		//	  $("#treedataset").dynatree({
		//	        title: "JSON LOADING FOR TISSUESTACK TREE EVENT",
		//	        fx: { height: "toggle", duration: 200 },
		//	        autoFocus: false, // Set focus to first child, when expanding or lazy-loading.
		//            initAjax: {
		//                url: "/TissueStack/minc/test",
		//                data: { mode: "test" } // data from data set
		//                },
		//	        onActivate: function(node) {
		//	        },
		//	  
		//	        onLazyRead: function(node){
		//                node.appendAjax({
		//                  url: "/TissueStack/minc/test",
		//                  data: {key: node.data.key,
		//                   mode: "test"
		//                 }
		//             });
		//	        }
		//	    });
			//END
	},
	buildTabletMenu : function() {
		// TODO: implement
	}
};
