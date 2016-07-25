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
TissueStack = {
	requiredLibs : [
	                	"/js/libs/sylvester/sylvester.js",
	                	"/js/libs/jquery/jquery.contextMenu-custom.js",
                        "/js/TissueStack.js",
	                	"/js/TissueStack.MouseWheel.js",
	                	"/js/TissueStack.Utils.js",
	                	"/js/TissueStack.Extent.js",
	                	"/js/TissueStack.Queue.js",
	                	"/js/TissueStack.Canvas.js",
	                	"/js/TissueStack.Events.js",
	                	"/js/TissueStack.DataSetStore.js",
                        "/js/TissueStack.ComponentFactory.js"
	                ],
    embedded : true
};

TissueStack.Embedded = function (div, server, data_set_id, include_cross_hair, use_image_service, initOpts) {
	// evaluate input and conduct peliminary checks

	if (typeof(div) != 'string' || $.trim(div) == '') {
		alert("Empty div ids are invalid!");
		return;
	}

	this.div = $("#" + div);
	if (this.div.length == 0) {
		alert("Given div id does not exist!");
		return;
	}

	if (typeof(server) != 'string' || $.trim(server) == '') {
		this.writeErrorMessageIntoDiv("Empty server name was given!");
		return;
	}
    this.server = server;

	if (typeof(data_set_id) != 'number' || data_set_id <= 0) {
		this.writeErrorMessageIntoDiv("Data Set id has to be greater than 0!");
		return;
	}
	this.data_set_id = data_set_id;

	if (typeof(include_cross_hair) != 'boolean' || include_cross_hair == true) {
		this.include_cross_hair = true;
	} else {
		this.include_cross_hair = false;
	}

	if (typeof(use_image_service) == 'boolean' && use_image_service == true) {
		this.use_image_service = true;
	} else {
		this.use_image_service = false;
	}

	if (typeof(initOpts) == 'object') {
		this.initOpts = initOpts;
	} else {
		this.initOpts = null;
	}

	var afterLoadActions = function(_this) {
        if (!TissueStack.Utils.supportsCanvas()) {
            alert("Your browser does not support the HTML5 feature 'Canvas'!\n\n" +
                    "This means that this site will be of very limited use for you.\n\n" +
                    "We recommend upgrading your browser: Latest versions of Chrome, Firefox, Safari and Opera support the canvas element," +
                    " so does IE from version 9 on.");
        }
    	//_this.domain = TissueStack.Utils.extractHostNameFromUrl(_this.server);

        // load server configuration values needed
        _this.loadDataBaseConfiguration();
        // load given data set configuration
        _this.loadDataSetConfigurationFromServer();
        // this is for when the window dimensions & the div dimensions may change dynamically
        _this.registerWindowResizeEvent();
	};

	// include the java script and css that is needed for the rest of the functionality
	this.includeJavaScriptAndCssNeeded(afterLoadActions);
};

TissueStack.Embedded.prototype = {
	librariesLoaded : 0,
	getDiv : function() {
		return this.div;
	},
	writeErrorMessageIntoDiv : function(message) {
		this.getDiv().html(message);
	},
	supportsCanvas : function() {
	  var elem = document.createElement('canvas');
	  return !!(elem.getContext && elem.getContext('2d'));
	},
	includeJavaScriptAndCssNeeded : function(afterLoadActions) {
		// if header does not exist => create it
		var head = $("head");
		if (head.length == 0) {
			head = $("html").append("<head></head>");
		}

		// add all the css that's necessary
		head.append(this.createElement("link", "http://" + this.server + "/css/default.css"));

		_this = this;

		var checkLoadOfLibraries = function() {
			_this.librariesLoaded++;
			if (_this.librariesLoaded == TissueStack.requiredLibs.length) {
				afterLoadActions(_this);
			}
		};

		var error = function() {
			alert("Failed to load required library!");
		};

		for (var i=0;i<TissueStack.requiredLibs.length;i++) {
			(function () {
				$.ajax({
					url : "http://" + _this.server + TissueStack.requiredLibs[i],
					async: false,
					dataType : "script",
					cache : false,
					timeout : 30000,
					success : checkLoadOfLibraries,
					error: error
				});
			})();
		}
	},
	createElement : function(tag, value) {
		if (typeof(tag) != "string" || typeof(value) != "string" || tag.length == 0 || value.length == 0) {
			return null;
		}

		var newEl = document.createElement(tag);
		if (tag == "script") {
			newEl.src = value;
		} else if (tag == "link") {
			newEl.rel = "stylesheet";
			newEl.href = value;
		} else {
			newEl.value = value;
		}

		return newEl;
	},
	loadDataSetConfigurationFromServer : function() {
		var _this = this;
		var url = "http://" + _this.server + "/" +
			TissueStack.configuration['server_proxy_path'].value + "/?service=services&sub_service=data&action=query&id="
			+ _this.data_set_id + "&include_planes=true";

		// create a new data store
        if (!TissueStack.dataSetStore)
            TissueStack.dataSetStore = new TissueStack.DataSetStore(null, true);

		TissueStack.Utils.sendAjaxRequest(
			url, 'GET',	true,
			function(data, textStatus, jqXHR) {
				if (!data.response && !data.error) {
					_this.writeErrorMessageIntoDiv("Did not receive anyting from backend, neither success nor error ....");
					return;
				}

				if (data.error) {
					var message = "Application Error: " + (data.error.description ? data.error.description : " no more info available. check logs.");
					_this.writeErrorMessageIntoDiv(message);
					return;
				}

				if (data.response.noResults) {
					_this.writeErrorMessageIntoDiv("No data set found in configuration database for given id");
					return;
				}

				// add to data store
				var dataSet = data.response[0];
				dataSet = TissueStack.dataSetStore.addDataSetToStore(dataSet, _this.server);
				if (!dataSet.data || dataSet.data.length == 0) {
					this.writeErrorMessageIntoDiv("Data set '" + dataSet.id + "' does not have any planes associated with it!");
					return;
				}

                // look for the next dataset_x that's available ...
                var nextAvailableOrdinal = 1;
                while (nextAvailableOrdinal < 100) {
                    var testDiv = $("#dataset_" + nextAvailableOrdinal);
                    if (!testDiv || testDiv.length == 0)
                        break;
                    nextAvailableOrdinal++;
                }

				// create the HTML necessary for display and initialize the canvas objects
                if (_this.initOpts)
                   _this.getDiv().hide();
                var ds_div = TissueStack.ComponentFactory.createDataSetWidget(
                    _this.getDiv().attr("id"), dataSet, nextAvailableOrdinal, _this.server, _this.include_cross_hair, true, _this.use_image_service);
                TissueStack.ComponentFactory.addProgressBar(ds_div, dataSet);
                //TissueStack.ComponentFactory.createColorMapSwitcher("dataset_" + nextAvailableOrdinal);
                //TissueStack.ComponentFactory.initColorMapSwitcher("dataset_" + nextAvailableOrdinal, dataSet);
				if (_this.initOpts) {
                    TissueStack.ComponentFactory.applyUserParameters(_this.initOpts, dataSet);
                    _this.getDiv().show();
                } else
                    TissueStack.ComponentFactory.redrawDataSet(dataSet);
			},
			function(jqXHR, textStatus, errorThrown) {
				_this.writeErrorMessageIntoDiv("Error connecting to backend: " + textStatus + " " + errorThrown);
			}
		);
	},
	loadDataBaseConfiguration : function() {
		// we do this one synchronously
		TissueStack.Utils.sendAjaxRequest(
			"http://" + this.server + "/" + TissueStack.configuration['server_proxy_path'].value +
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

				TissueStack.Utils.loadColorMaps();
			},
			function(jqXHR, textStatus, errorThrown) {
				alert("Error connecting to backend: " + textStatus + " " + errorThrown);
			}
		);
	},
	registerWindowResizeEvent : function() {
		var _this = this;
		// handle dynamic window & parent div resizing
		$(window).resize(function() {
			// set new canvas dimensions
			var dataSet = TissueStack.dataSetStore.getDataSetByIndex(0);
			for (var plane in dataSet.planes) {
				dataSet.planes[plane].resizeCanvas();
			}
		});
	},
};
