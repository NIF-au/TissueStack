TissueStack.ComponentFactory = {
    extractHostNameFromUrl : function(url) {
		if (typeof(url) !== "string") {
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
    createDataSet : function(div, dataSet, dataSetOrdinal, server, includeCrossHair) {
        if (!TissueStack.ComponentFactory.checkStandardInput(div, dataSet, dataSetOrdinal, true))
            return;

        if (typeof(server) === 'string' && server !== 'localhost')
            server = TissueStack.ComponentFactory.extractHostNameFromUrl(server);
        
        var maximizePngPath = 'images/maximize.png';
        if (server != null)
            maximizePngPath = "http://" + server + "/" + maximizePngPath;
        
        var dataSetPrefix = "dataset_" + dataSetOrdinal;
        
        var html = '<div id="' + dataSetPrefix + '" class="dataset">';
		
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
        
        return dataSetPrefix;
    },
    addScaleToDataSet : function(div, dataSet, dataSetOrdinal) {
        if (!TissueStack.ComponentFactory.checkStandardInput(div, dataSet, dataSetOrdinal)) {
            alert("ComponentFactory::addScaleToDataSet => Failed to add scale bar!");
            return;
        }
        
        var dataSetPrefix = "dataset_" + dataSetOrdinal;

        var html = '<div class="scalecontrol_main"><div id="' + dataSetPrefix + '_scale_middle" class="scalecontrol_middle">'
            + '<div class="scalecontrol_image" style="left: 0px; top: -424px; width: 89px;"></div></div>'
            + '<div id="' + dataSetPrefix + '_scale_left" class="scalecontrol_left">'
			+ '<div class="scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
			+ '<div id="' + dataSetPrefix + '_scale_center_left" class="scalecontrol_center_left">'
	    	+ '<div class="scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + dataSetPrefix + '_scale_center_right" class="scalecontrol_center_right">'
	    	+ '<div class="scalecontrol_image" style="left: 0px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + dataSetPrefix + '_scale_up" class="scalecontrol_up">'
    		+ '<div class="scalecontrol_image" style="left: -4px; top: -398px; width: 59px;"></div></div>'
	    	+ '<div id="' + dataSetPrefix + '_scale_text_up" class="scalecontrol_text_up"></div></div>';
   
        $("#" + div).prepend(html);
	}
};