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
TissueStack.Admin = function () {
	this.checkCookie();
	this.initTaskView();
	// register all the event handlers for login, file upload and task actions
	this.registerLoginHandler();
	this.registerCreateSessionHandler();
	this.registerFileUploadHandler();
	this.registerQueryListRefreshHandler();
	this.registerTaskHandler();
	// make sure the upload directory displays the up-to-date contents
	this.displayUploadDirectory();
	this.cloneTableHeader();
};

TissueStack.Admin.prototype = {
	session: null,
	queue_handles : {},
	processType: "",
	progress_task_id: "",
	pre_tile_task_add: "",
	detect_cookie_type: "",
	registerQueryListRefreshHandler : function () {
		var _this = this;
		//top tabs check for refreshing upload directory list
		$('#tab_admin_data').click(function(){
			_this.displayUploadDirectory();
		});
		$('#radio_task').change(function(){
			_this.displayUploadDirectory();
		});
		//refresh jquery mobile list view in colormap menu phone version
		$('#setting_menu_control').click(function(){
			$('#phone_color_map').fadeOut(50).fadeIn(50, function() {
				$('#phone_color_map').trigger( "create" );
			});
		});
	},
	registerCreateSessionHandler : function () {
	 	var _this = this;
	 	
	 	// make it possible to use ENTER to submit
	 	$("#password").keyup(function(event){
	 	    if(event.keyCode == 13){
	 	        $("#login_btn").click();
	 	    }
	 	});
	 	// make it possible to use ENTER to submit
	 	$("#new_password").keyup(function(event){
	 	    if(event.keyCode == 13){
	 	        $("#passwd_change_btn").click();
	 	    }
	 	});

	 	// login handler
		$('#login_btn').click(function(){
		 	var password = $('#password').val();
		 	
		 	if (!password) {
		 		_this.replaceErrorMessage("A non-empty password needs to be given!");
		 		return;
		 	}
		 	password = $.trim(password);
		 	if (password == "") {
		 		_this.replaceErrorMessage("A non-empty password needs to be given!");
		 		return;
		 	}
		 	
	 		TissueStack.Utils.sendAjaxRequest(
 				"/" + TissueStack.configuration['server_proxy_path'].value +
 				"/?service=services&sub_service=security&action=new_session&password="+ password, 'GET', true,
 				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("Did not receive any session, neither lose session ....");
						return;
					}	
					if (data.error) {
						var message = "Session Error: " + (data.error.description ? data.error.description : " no more session available. Please login again.");
						_this.replaceErrorMessage(message);
						return;
					}	
					if (data.response.noResults) {
						_this.replaceErrorMessage("Wrong password!");
						return;
					}
					var session= data.response;
					_this.session = session.id;
					var value = $('#login_btn').val(100);
					_this.checkCookie(session, value);
					
					if(TissueStack.phone){
						$('#phone_login').append("").fadeOut(500);		
						$('#phone_addDataSet').show().fadeIn(500);
						return;  
					}
					$("#close").click();
					_this.replaceErrorMessage("Login successfully!");
					$('.error_message').css("background", "#32CD32");
					$('#password').val("");
				},
				function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
				}
			);
		});
		
	 	// passwd change handler
		$('#passwd_change_btn').click(function(){
		 	var old_password = $('#old_password').val();
		 	var new_password = $('#new_password').val();
		 	
		 	if (!old_password || !new_password) {
		 		_this.replaceErrorMessage("Both, old and new password need to be given!");
		 		return;
		 	}
		 	old_password = $.trim(old_password);
		 	new_password = $.trim(new_password);
		 	if (old_password == "" || new_password == "") {
		 		_this.replaceErrorMessage("Both, old and new password need to be given!");
		 		return;
		 	}
		 	
	 		TissueStack.Utils.sendAjaxRequest(
	 				"/" + TissueStack.configuration['server_proxy_path'].value +
	 				"/?service=services&sub_service=security&action=passwd&old_passwd="+ old_password
	 				+ "&new_passwd=" + new_password, 'GET', true,
 				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("Serious Backend Error!");
						return;
					}	
					if (data.error) {
						var message = "Failed to change Password: " + (data.error.description ? data.error.description : " Try again...");
						_this.replaceErrorMessage(message);
						return;
					}	
					var session= data.response;
					_this.session = session.id;
					var value = $('#login_btn').val(100);
					_this.checkCookie(session, value);
					
					if(TissueStack.phone){
						$('#phone_login').append("").fadeOut(500);		
						$('#phone_addDataSet').show().fadeIn(500);
						return;  
					}
					_this.replaceErrorMessage("Password changed successfully!");
					$('.error_message').css("background", "#32CD32");
					$('#old_password').val("");
					$('#new_password').val("");
				},
				function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
				}
			);
		});
	},
	getCookie: function(c_name)	{
		var i,x,y,ARRcookies=document.cookie.split(";");
		for (i=0;i<ARRcookies.length;i++)
		  {
		  x=ARRcookies[i].substr(0,ARRcookies[i].indexOf("="));
		  y=ARRcookies[i].substr(ARRcookies[i].indexOf("=")+1);
		  x=x.replace(/^\s+|\s+$/g,"");
		  if (x==c_name)
		    {
		   	 return unescape(y);
		    }
		  }
	},	
	setCookie: function(c_name,value,exdays){
		var exdate=new Date();
		exdate.setDate(exdate.getDate() + exdays);
		var c_value = escape(value) + ((exdays==null) ? "" : "; expires="+exdate.toUTCString());		
		document.cookie=c_name + "=" + c_value;
		//$.cookie(c_name,c_value, 1);
	},
	checkCookie: function (session, value){
		var session_name=this.getCookie("session");
		
		if ($(value).val() != null){
			session_name = null;
		}
		
		if (session_name!=null && session_name !="")
		  	this.session = session_name;		  	
		else { 
		  session_name = session;
		  if (session_name!=null && session_name!="")
		    {
		   		this.setCookie("session",session_name.id,1);
		   		return;
		    }
		  }
	},
	initTaskView : function() {
	  	if($.cookie("tasks") == null) return;
	  	
	  	// read cookie and build local dom representation
	  	TissueStack.Tasks.readFromCookie();
	  	
	  	if (TissueStack.tasks == null) return;
	  	// loop through all tasks
	  	for (var t in TissueStack.tasks) 
	  		this.addTask(TissueStack.tasks[t]);
	},
	registerLoginHandler : function () {
	   	$("#open").click(function(){
	   		$("div#panel").slideDown("fast");	
	   	});		
   		$("#close").click(function(){
   			$("div#panel").slideUp("fast");	
   		});		
   		$("#toggle a").click(function () {
   			$("#toggle a").toggle();
   		});	
	},
	displayUploadDirectory : function (){
		//var _this = this;  
	     $(".file_radio_list").show(function(){
	    	 var url = "/" + TissueStack.configuration['server_proxy_path'].value + 
	    	 	"/?service=services&sub_service=admin&action=";
	    	 var action = $('input[name=radio_task]:checked').val();
	    	 if (typeof(action) == 'string' && action === 'PreTile')
	    		 url += "data_set_raw_files";
	    	 else url += "upload_directory";

	    	 // complete url setting filter params
	    	//url += ("/json");
	    	//if (typeof(action) == 'string' && action === 'AddDataSet') url += "?display_raw_only=true";
	    	//if (typeof(action) == 'string' && action === 'Convert') url += "?display_conversion_formats_only=true";
	    	 if (typeof(action) == 'string' && action === 'AddDataSet') url += "&display_raw_only=true";
	    	 if (typeof(action) == 'string' && action === 'Convert') url += "&display_conversion_formats_only=true";
	    	$("#directory_name").html((typeof(action) == 'string' && action === 'PreTile') ? "Data Sets" : "Upload Directory");
	    	
	      	TissueStack.Utils.sendAjaxRequest(url, "GET", true,function(result) {
	        	var listOfFileName = "";
	      		if (result && result.response && result.response.length > 0) {
			        $.each(result.response, function(i, uploadedFile){
			        	content = '<input type="radio" name="radio_listFile" class="uploaded_file" id="check_' + uploadedFile + '" value="' + uploadedFile + '" />'
			        			+ '<label for="'+'check_'+ uploadedFile +'">'+ uploadedFile + '</label>';
			        	listOfFileName += content; 
		              });
	      		}
	            $('.file_radio_list').fadeIn(1500, function() {  
	             	 $('.file_radio_list').html(listOfFileName)
	             	 	.trigger( "create" );
	             	 	//_this.identifyFileType(); 
	             	 $('.file_radio_list').controlgroup('refresh', true);
		        });
	      	});	    
	     });
	},
	registerFileUploadHandler : function () {
		var _this = this;

		// upload progress elements
		var bar = $('.bar');
		var percent = $('.percent');
		
		 $("#uploadForm").submit(function(){
			// error display helper
			var errorHandling2 = function(message) {
			    bar.width('0%');
			    percent.html('0%');
				_this.replaceErrorMessage(message);
			};

			// extract file name and start upload monitor
			var filename = $.trim($('#filename_1').val());
			var progressUpdater = null;
			if (filename == '') {
				errorHandling2("Error: Please select a file for upload!");
				return false;
			}

			// check session validity beforehand
			if (!_this.checkSessionValidity(_this.session)) {
				errorHandling2("Error: Invalid Session! Please Log In.");
				return false;
			}

			var slashPos = filename.lastIndexOf('\\');
			if (slashPos < 0)
				slashPos = filename.lastIndexOf('/');
			if (slashPos >= 0) {
				// reset to zero percent 
		        bar.width('0%');
		        percent.html('0%');
		        // extract file name without fake path
				filename = filename.substring(slashPos+1);

				// we query periodically and abort after 5 failed communications
				var failedQueryAttempts = 5;
				var error_handling = function() {
					if (failedQueryAttempts <= 0) {
						if (progressUpdater) clearInterval(progressUpdater);
						percent.html('Failed to upload File!');
						_this.replaceErrorMessage("Failed to upload file. It may be connectivity...");
					}
					failedQueryAttempts--;
				};
				
				// the periodic request
				progressUpdater = setInterval(function () {
					TissueStack.Utils.sendAjaxRequest(
							"/" + TissueStack.configuration['server_proxy_path'].value + "/?service=services&sub_service=admin&action=upload_progress&&file=" + filename,
							'GET', true,
							function(data, textStatus, jqXHR) {
								if ((!data.response && !data.error)
										|| data.error || data.response.noResults) {
									error_handling();
									return;
								}

								try {
									var progress = data.response.progress;
						
									// file may not have been there due to periodic writing
									if (progress == -1)
										return;
									
									//  we are finished: cancel progress monitor
									if (progress >= 100 ) {
								    	bar.width('100%');
								    	percent.html('100%');
										if (progressUpdater) clearInterval(progressUpdater);
										_this.displayUploadDirectory();
										_this.replaceErrorMessage("File Has Been Successfully Uploaded!");
										$('.error_message').css("background", "#32CD32");
										
										return;
									}
									
									// update percent
							    	var percentVal = progress + '%';
							    	bar.width(percentVal);
							    	percent.html(percentVal);
							    	// reset failed counter
									failedQueryAttempts = 5;
								} catch (anything) {
									error_handling();
								}
							},
							function(jqXHR, textStatus, errorThrown) {
								error_handling();
								return;
							}
						);  
					}, 1500);
			}
			
			// the actual submit for the file upload
			$(this).ajaxSubmit({ 	
				url : "/" + TissueStack.configuration['server_proxy_path'].value +
					"/?service=services&sub_service=admin&action=upload&session=" + _this.session,
				dataType : "json",
				success: function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						errorHandling2("No File Submission!");
						return false;
					}
					
					if (data.error) {
						errorHandling2("Error: " + (data.error.description ? data.error.description : " No File Submission!"));
						return false;
					}
					
					if (data.response.noResults) {
						errorHandling2("No Results!");
						return false;
					}
				},
				error: function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					return false;
				}
			});
			return false;
		});
	},
	replaceErrorMessage : function (message) {
		var excludes = message;
		
		if(excludes.search("IllegalArgumentException") != -1){
			message = excludes.replace("java.lang.IllegalArgumentException:", "");
		}		
		if(excludes.search("RuntimeException") != -1){
			message = excludes.replace("java.lang.RuntimeException:", "");
		}	

		if(TissueStack.desktop || TissueStack.tablet){
			$('#file_uploaded_message').html("<div class='error_message'></div>");  		
			$('.error_message').html(message + "<br/>")  
				.append().hide().fadeIn(1500, function() {  
					$('.error_message').append("");  
			}).fadeOut(5000);
		}
		if(TissueStack.phone) alert(message);	
	},
	registerDataSetWithAnds : function(id) {
		if (typeof(id) != 'number') return;
		
		var reply = confirm("Do you want to register your data set?");
		if (reply) {
			var popup_handle = window.open('/ands_dataset_registration.html', 'Registration', 'height=550,width=400,location=no');
			var closePopup = function() {
				popup_handle.close();
			};
			popup_handle.closeMe = closePopup;

			// pre-fill a few values after creation
			var hostPart = document.location.href.split('?')[0];
			var potentialHash = hostPart.indexOf('#');
			if (potentialHash > 0) hostPart = hostPart.substring(0,potentialHash);
			setTimeout(function() {
				$("#dataset_id", popup_handle.document).attr("value",id);
				$("#dataset_location", popup_handle.document).attr("value", hostPart + "?ds=" + id);
			},100);
		}
	},
	registerTaskHandler : function() {
		var _this = this;
		$("#bt_process").click(function() {
			var action = $('input[name=radio_task]:checked').val();
			var actonType = null;
			var actionStatus = TissueStack.Tasks.ReverseStatusLookupTable["Queued"];
			var fileSelected = false;
			
			var task_zoom_level = eval(TissueStack.configuration.default_zoom_levels.value).length;
			
			var successHandler = null;
			
			// the common code part for all actions
		  	$.each($('.uploaded_file'), function(i, uploaded_file) {
		  		if (uploaded_file.checked) { // only if a file was selected
		  			fileSelected = true;
					if (!action) { // check whether action was selected
						_this.replaceErrorMessage("Please choose an action!");
						return;
					}		
					
					// quick preliminary for file type: data set additions and tiling are only allowed for raw,
					// conversions should be nifti or minc
					if ((uploaded_file.value.lastIndexOf(".nii") > 0 || uploaded_file.value.lastIndexOf(".mnc") > 0)
							&& action != "Convert") {
						_this.replaceErrorMessage("Formats .nii and .mnc need to be converted to .raw first!");				
						return;
					} else if (uploaded_file.value.lastIndexOf(".raw") > 0
							&& (action != "PreTile" && action != "AddDataSet") ) {
						_this.replaceErrorMessage("Conversion needs a .nii or .mnc file as input!");				
						return;
					}
						
			  		for(var i = 0; i < task_zoom_level ; i++) {
				  		// don't be confused by the outer loop, it will only apply for pre-tile
			  			if (action != "PreTile" && i > 0) break;
			  			
						// the url to contact (will be completed by whatever action we want to carry out
						var url = "/" + TissueStack.configuration['server_proxy_path'].value +
							"/?service=";
	
						// set the individual bits for the selected action 
						if(action == "AddDataSet") {
							// we have an additional description existence check here
	 						var msgDescription = $('#txtDesc').val();
	 						if (msgDescription == ""){
	 							_this.replaceErrorMessage("Please Add Description Before Adding New Data Set!");
	 							return;
	 						}
							
							//url += ("add_dataset/json?session=" + _this.session + "&filename=" + uploaded_file.value + "&description=" + msgDescription);
	 						url += 
	 							("services&sub_service=admin&action=add_dataset&session=" + 
	 								_this.session + "&filename=" + uploaded_file.value + "&description=" + msgDescription);
							successHandler = _this.addDataSetSuccessHandler;
						} else if(action == "Convert") {
							url += ("conversion&session=" + _this.session + "&file=" + TissueStack.configuration['server_proxy_path'].value + "/" + uploaded_file.value);
							actonType = TissueStack.Tasks.ReverseTypeLookupTable["Conversion"];
							successHandler = _this.conversionAndPreTileSuccessHandler;
						} else if(action == "PreTile") {
							url += ("tiling&session=" + _this.session	+ "&file=" + TissueStack.configuration['data_directory'].value + "/" 
									+ uploaded_file.value
									+ "&tile_dir=" + TissueStack.configuration['server_tile_directory'].value
									+ "&dimensions=x,y,z" + "&zoom=" + i);
							
							actonType = TissueStack.Tasks.ReverseTypeLookupTable["Tiling"];
							successHandler = _this.conversionAndPreTileSuccessHandler;
						}
	
				  		// send ajax request
				 		TissueStack.Utils.sendAjaxRequest(url, 'GET', true,
							function(data, textStatus, jqXHR) {
								if (!data.response && !data.error) {
									_this.replaceErrorMessage("No Response Data Returned!");
									return;
								}
								if (data.error) {
									var message = "Error: " + (data.error.description ? data.error.description : " Action wasn't performed!");
									_this.replaceErrorMessage(message);				
									return;
								}
								if (data.response.noResults) {
									_this.replaceErrorMessage("No Results!");
									return;
								}
		
								if (successHandler)
									successHandler(_this, data.response, uploaded_file.value, actonType, actionStatus, i);
							},
							function(jqXHR, textStatus, errorThrown) {
								if (jqXHR.status == 408) {
									_this.replaceErrorMessage("Task has become obsolete!");
									return
								}
								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
								return;
							}
						);

			  		}
		  		}
		  	});
		  	if (!fileSelected) _this.replaceErrorMessage("Please select a file first!");
		});
	},
	addDataSetSuccessHandler : function(_this, response) {
		var dataSet = response;
		var addedDataSet = TissueStack.dataSetStore.addDataSetToStore(dataSet, "localhost");
		if (addedDataSet) {
			if(TissueStack.desktop)	TissueStack.dataSetNavigation.addDataSetToDynaTree(addedDataSet);
			if (TissueStack.tablet) TissueStack.dataSetNavigation.addDataSetToTabletTree(addedDataSet);
			_this.displayUploadDirectory();
			_this.replaceErrorMessage("Data Set Has Been Added Successfully!");
			$('.error_message').css("background", "#32CD32");
			
			//_this.registerDataSetWithAnds(dataSet.id);
		}
	},
	conversionAndPreTileSuccessHandler : function(_this, response, file, type, status, zoom) {
		$(".file_radio_list input:radio[id^="+ "'check_" + file + "']:first").attr('disabled', true).checkboxradio("refresh");
		var task = {id: parseInt(response), file: file, type: type, status: status};
		if (typeof(zoom) == 'number') task.zoom = zoom;
		
		if (!_this.addTask(task)) {
			_this.replaceErrorMessage("Failed To Add Action To The Queue!");
		} else {
			var handle = 
				setInterval(function() {if (TissueStack.Tasks.writeToCookie()) clearInterval(handle);}, 500);
		}
	},
	addTask: function (task) {
		// add task to dom representation first, if the function calls works (does not return null) go ahead and create the html table
		if (!TissueStack.Tasks.addOrUpdateTask(task)) return false;
		
		var _this = this;
		var nrCols = 6; 
		
		var tab = $('#task_table');
		var tbody = ('#task_table tbody');
		
		var row = document.createElement('tr');
		//row.height = "10px";
		row.style ="height:20px";	
		row.id = "task_" + task.id;			
		for(var j = 0; j < nrCols; j++){
			var processBar = "" ;
			var cell = document.createElement('td');
			
			switch(j) {
				case 0: // Task id
					processBar = task.id;
					break;

				case 1: // File
					processBar = task.file;
					break;
					
				case 2: // Type
					processBar = TissueStack.Tasks.getTypeAsString(task.type);
					break;
				
				case 3: // Status
					cell.id = "task_status_" + task.id;
					processBar = TissueStack.Tasks.getStatusAsString(task.status);
					break;
				
				case 4: // Progress
					//no process bar in tablet version (iPad limitation : non display of process bar)
					if (TissueStack.tablet){
						processBar = '<div class="progress_bar_div">' + '<span class="progress_bar_text_tablet" id="progress_text_' + task.id + '">0%</span ></div>';
					}
					else{	
						processBar = 
							'<div class="progress_bar_div"><progress class="progress_bar" id="progress_bar_' + task.id + '" value="0" max="100"></progress>'
							+ '<span class="progress_bar_text" id="progress_text_' + task.id + '">0%</span ></div>';
					}
				break;
				
				case 5: // Cancel Column
					processBar	= '<a id="cancel_' + task.id  + '" data-role="button" data-theme="c" data-icon="delete" data-iconpos="notext">Cancel</a>';
					break;
			}
			$(cell).append(processBar);	
			$(row).append(cell);
		}

		// add row to table
		$(tbody).append(row);
		// refresh
		tab.trigger("create");
		// register cancel event and launch task progress checkers
		_this.startProgressChecker(task.id);
		_this.registerTaskCancelHandler(task.id, task.file);
		
		return true;
	},
	stopTaskProgressCheck : function(id) {
		clearInterval(this.queue_handles[id]);
		this.queue_handles[id] = null;
	},
	registerTaskCancelHandler : function (id, file) {
		if (typeof(id) != 'number' && typeof(file) != 'string') return;
		
		var markTaskAsCanceledAndStopChecking = function(_this, id) {
			 // set status to canceled
			TissueStack.tasks[id].status = 3;
			// reflect new status in UI
			$('#task_status_' + id).html(TissueStack.Tasks.getStatusAsString(TissueStack.tasks[id].status));
			// disable cancel
			$('#cancel_' + id).addClass('ui-disabled');
			_this.displayUploadDirectory();
			// write back to cookie
			if (TissueStack.Tasks.writeToCookie())
				_this.stopTaskProgressCheck(id);
		};
		
		var _this = this;
		$('#cancel_' + id).click(function() {
		   TissueStack.Utils.sendAjaxRequest(
				"/" + TissueStack.configuration['server_proxy_path'].value +
					"/?service=services&sub_service=admin&action=cancel" +
				"&session=" + _this.session + "&task_id=" + id,
				'GET', true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("No Task ID Applied");
						return;
					}
					if (data.error) {
						var message = "Error: " + (data.error.description ? data.error.description : " No Task ID Applied");
						// no task id message
						if (message.indexOf(id) > 0)
							markTaskAsCanceledAndStopChecking(_this, id);
						else
							_this.replaceErrorMessage(message);				
						return;
					}
					if (data.response.noResults) {
						_this.replaceErrorMessage("No Results!");
						return;
					}
					
					if (data.response.filename && data.response.filename.indexOf(file) >= 0) 
						markTaskAsCanceledAndStopChecking(_this, id);
				},
				function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					return;
				}
			);
		});
	},
	startProgressChecker : function(id) {
		if (typeof(id) != 'number') return;
		
		var _this = this;
		
		this.queue_handles[id] = setInterval(function () {
			TissueStack.Utils.sendAjaxRequest(
				"/" + TissueStack.configuration['server_proxy_path'].value +
					"/?service=services&sub_service=admin&action=progress&task_id=" + id,
				'GET', true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("No Data Set Updated!");
						_this.stopTaskProgressCheck(id);
						$('#task_status_' + id).html("Back End Error. Retry later (Refresh Page)!");
						return;
					}
					if (data.error) {
						var message = "Error: " + (data.error.description ? data.error.description : " No DataSet Selected!");
						_this.replaceErrorMessage(message);
						_this.stopTaskProgressCheck(id);
						$('#task_status_' + id).html("Back End Error. Retry later (Refresh Page)!");
						return;
					}
					if (data.response.noResults) {
						_this.replaceErrorMessage("No Results!");
						_this.stopTaskProgressCheck(id);
						$('#task_status_' + id).html("Back End Error. Retry later (Refresh Page)!");
						return;
					}

					var processTask = data.response;
		
					//  set to running from initial 'Queued' state (if we are not canceled or finished that is)
					if (processTask.progress >= 0 && TissueStack.tasks[id].status != 2 && TissueStack.tasks[id].status != 3) {
						TissueStack.tasks[id].status = 1; 
						$('#task_status_' + id).html(TissueStack.Tasks.getStatusAsString(TissueStack.tasks[id].status));
					}
					
					if (processTask.progress >= 0 && processTask.progress < 100) {
						$("#" + "progress_text_" + id).html(processTask.progress.toFixed(2) + "%");
						$("#" + "progress_bar_" + id).val(processTask.progress);
					} else if (processTask.progress >= 100) {
						$("#" + "progress_text_" + id).html("100%");
						$("#" + "progress_bar_" + id).val(100);
						 // set status to finished
						TissueStack.tasks[id].status = 2;
						// reflect new status in UI
						$('#task_status_' + id).html(TissueStack.Tasks.getStatusAsString(TissueStack.tasks[id].status));
						// disable cancel
						$('#cancel_' + id).addClass('ui-disabled');
						_this.displayUploadDirectory();
						// write back to cookie
						if (TissueStack.Tasks.writeToCookie())
							_this.stopTaskProgressCheck(id);
					}
				},
				function(jqXHR, textStatus, errorThrown) {
					if (typeof(textStatus) == 'string' && (textStatus == 'parsererror' || textStatus == 'error'))
						return;
					
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					_this.stopTaskProgressCheck(id);
					$('#task_status_' + id).html("Back End Error. Retry later (Refresh Page)!");
					return;
				}
			);  
		}, 2500);
	},
	togglePreTilingFlag: function(id, flag) {
		__this = this;
		if (!__this.session || !id) return;
		
		TissueStack.Utils.sendAjaxRequest(
			"/" + TissueStack.configuration['server_proxy_path'].value +
				"/?service=services&sub_service=admin&action=toggle_tiling" +
			"&session=" + __this.session + "&id=" + id + "&flag=" + flag,
			'GET', true,
			function(data, textStatus, jqXHR) {
				// truly we don't care
			},
			function(jqXHR, textStatus, errorThrown) {
				// truly we don't care either
			}
		);
	},
	updateTableHeaders: function () {
        $("#task_list").each(function() {
            var originalHeaderRow = $(".original_table_header", this);
            var floatingHeaderRow = $(".new_table_header", this);
            var scrollTop = $("#task_list").scrollTop();
            
            floatingHeaderRow.css("top", scrollTop + "px");
			if(scrollTop !=0)
				floatingHeaderRow.css("visibility", "visible");
            // Copy cell widths from original header
            $("th", floatingHeaderRow).each(function(index) {
                var cellWidth = $("th", originalHeaderRow).eq(index).css('width');
                $(this).css('width', cellWidth);
            });

            // Copy row width from whole table
            floatingHeaderRow.css("width", $(this).css("width"));
        });
	},
	cloneTableHeader: function () {
        $("#task_table").each(function() {
            $(this).wrap("<div class='task_list' style='position:relative'></div>");

            var originalHeaderRow = $("tr:first", this)
            originalHeaderRow.before(originalHeaderRow.clone());
            var clonedHeaderRow = $("tr:first", this)

            clonedHeaderRow.addClass("new_table_header");
            clonedHeaderRow.css("position", "absolute");
            clonedHeaderRow.css("top", "0px");
            clonedHeaderRow.css("z-index", "2000");
            clonedHeaderRow.css("left", $(this).css("margin-left"));
			clonedHeaderRow.css("visibility", "hidden");
            originalHeaderRow.addClass("original_table_header");
        });
        this.updateTableHeaders();
        
        $("#task_list").scroll(this.updateTableHeaders);
        $("#task_list").resize(this.updateTableHeaders);
	},
	checkSessionValidity : function (session_token) {
		var isValid = false;

		TissueStack.Utils.sendAjaxRequest(
	    	"/" + TissueStack.configuration['server_proxy_path'].value + 
	    	 	"/?service=services&sub_service=security&action=check_session&session=" + session_token,
			'GET', false,
			function(data, textStatus, jqXHR) {
				if ((!data.response && !data.error) || data.error) {
					return; // NOK case
				}

				// this is the OK case!
				if (data.response.noResults) isValid = true;
			},
			function(jqXHR, textStatus, errorThrown) {
				return;
			}
		);  
	
		return isValid;
	}
};