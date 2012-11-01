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
		
		$("#radio_task input:radio[id^='bt_convert']:first").attr('disabled', true).checkboxradio("refresh");
		$("#radio_task input:radio[id^='bt_preTile']:first").attr('disabled', true).checkboxradio("refresh");
		$("#radio_task input:radio[id^='bt_AddDataSet']:first").attr('disabled', true).checkboxradio("refresh");
		
		//top tabs check for refreshing upload directory list
		$('#tab_admin_data, #tab_admin_setting').click(function(){
			_this.displayUploadDirectory();
		});
	},
	registerCreateSessionHandler : function () {
	 	var _this = this;

		$('#login_btn').click(function(){
		 	var password = $('#password').val();
		 	if(password != "") {
		 		TissueStack.Utils.sendAjaxRequest(
	 				"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/security/new_session/json?password="+ password, 'GET', true,
	 				function(data, textStatus, jqXHR) {
						if (!data.response && !data.error) {
							_this.replaceErrorMessage("Did not receive any session, neither lose session ....");
							return;
						}	
						if (data.error) {
							var message = "Session Error: " + (data.error.message ? data.error.message : " no more session available. Please login again.");
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
						$("div#panel").slideUp("slow");
						$('#name_tap').html("Hello " + $('#username').val());
						_this.replaceErrorMessage("Login successfully!");
						$('.error_message').css("background", "#32CD32");
					},
					function(jqXHR, textStatus, errorThrown) {
						_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					}
				);
			}
			$('#password').val("");
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
		var _this = this;  
	     $(".file_radio_list").show(function(){
	      	"/" + TissueStack.Utils.sendAjaxRequest(TissueStack.configuration['restful_service_proxy_path'].value + "/admin/upload_directory/json", "GET", true,function(result) {
		    	if (!result || !result.response || result.response.length == 0) {
		    		return;
		    	}
		    	// display results
	        	var listOfFileName = "";	    	
		        $.each(result.response, function(i, uploadedFile){
		        	content = '<input type="radio" name="radio_listFile" class="uploaded_file" id="check_' + uploadedFile + '" value="' + uploadedFile + '" />'
		        			+ '<label for="'+'check_'+ uploadedFile +'">'+ uploadedFile + '</label>';
		        	listOfFileName += content; 
	              });
	            $('.file_radio_list').fadeIn(1500, function() {  
	             	 $('.file_radio_list').html(listOfFileName)
	             	 	.trigger( "create" );
	             	 	_this.identifyFileType(); 
	             	 $('.file_radio_list').controlgroup('refresh', true);
		        });
	      	});	    
	     });
	},
	identifyFileType: function () {
		$("input[name=radio_listFile]").change(function() {
			if($(this).val().split('.').pop() == "raw") {
				$('#radio_task').fadeOut(250, function() {  
				 	 $("#radio_task input:radio[id^='bt_convert']:first").attr('disabled', true).checkboxradio("refresh");
				 	 $("#radio_task input:radio[id^='bt_AddDataSet']:first").attr('disabled', false).checkboxradio("refresh");
				 	 $("#radio_task input:radio[id^='bt_preTile']:first").attr('disabled', false).checkboxradio("refresh");
				 	 $('#radio_task').fadeIn();
				});
			}
			if($(this).val().split('.').pop() == "mnc" || $(this).val().split('.').pop() == "nii" ) {
				$('#radio_task').fadeOut(250, function() {  
				 	 $("#radio_task input:radio[id^='bt_convert']:first").attr('disabled', false).checkboxradio("refresh");
				 	 $("#radio_task input:radio[id^='bt_AddDataSet']:first").attr('disabled', true).checkboxradio("refresh");
				 	 $("#radio_task input:radio[id^='bt_preTile']:first").attr('disabled', true).checkboxradio("refresh");
				 	 $('#radio_task').fadeIn();
				});
			}
		});
	},
	registerFileUploadHandler : function () {
		var _this = this;
		_this.uploadProgress();
		 $("#uploadForm").submit(function(){
			$(this).ajaxSubmit({ 	
				url : "/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/upload/json?session=" + _this.session,
				dataType : "json",
				success: function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("No File Submission!");
						return false;
					}
					
					if (data.error) {
						var message = "Error: " + (data.error.message ? data.error.message : " No File Submission!");
						_this.replaceErrorMessage(message);
						return false;
					}
					
					if (data.response.noResults) {
						_this.replaceErrorMessage("No Results!");
						return false;
					}
					_this.displayUploadDirectory();
					_this.replaceErrorMessage("File Has Been Successfully Uploaded!");
					$('.error_message').css("background", "#32CD32");
				},
				error: function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					return false;
				}
			});
			return false;
		});
	},
	uploadProgress : function () {
		var _this = this;
		var bar = $('.bar');
		var percent = $('.percent');
		   
		$('#uploadForm').ajaxForm({
		    beforeSend: function() {
		        var percentVal = '0%';
		        bar.width(percentVal);
		        percent.html(percentVal);
		    },
		    uploadProgress: function(event, position, total, percentComplete) {
		    	var check_Success = 1;
		    	$.each($('.uploaded_file'), function(i, uploaded_file) {
		    		var filePath = $('#filename_1').val();
		    		if(filePath.match(/fakepath/)){
		    			filePath = filePath.replace(/C:\\fakepath\\/i, '');	    		
		    		}
		    		if (_this.session == null || filePath == uploaded_file.value || filePath == '') {
		    			check_Success = null;
		    		}	
		    	});
		    	if(check_Success !=null){
			    	var percentVal = percentComplete + '%';
			    	bar.width(percentVal);
			    	percent.html(percentVal);
		    	}
		    },
		    complete: function (xhr) {
		    }
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
	registerDataSetWithAnds : function() {
		var reply = confirm("Do you want to register your data set?");
		if (reply) {
			var popup_handle = window.open('/ands_dataset_registration.html', 'Registration', 'height=350,width=250,location=no');
			var closePopup = function() {
				popup_handle.close();
			};
			popup_handle.closeMe = closePopup;
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
			
			// the url to contact (will be completed by whatever action we want to carry out
			var url = "/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/";
			var successHandler = null;
			
			// the common code part for all actions
		  	$.each($('.uploaded_file'), function(i, uploaded_file) {
		  		if (uploaded_file.checked) { // only if a file was selected
		  			fileSelected = true;
					if (!action) { // check whether action was selected
						_this.replaceErrorMessage("Please choose an action!");
						return;
					}			
						
			  		// don't be confused by the outer loop, it will only apply for pre-tile
			  		for(var i = 0; i < task_zoom_level ; i++) {
			  			if (action != "PreTile" && i > 0) break;

						// set the individual bits for the selected action 
						if(action == "rad_addDataSet") {
							// we have an additional description existence check here
	 						var msgDescription = $('#txtDesc').val();
	 						if (msgDescription == ""){
	 							_this.replaceErrorMessage("Please Add Description Before Adding New Data Set!");
	 							return;
	 						}
							
							url += ("add_dataset/json?session=" + _this.session + "&filename=" + uploaded_file.value + "&description=" + msgDescription);
							successHandler = _this.addDataSetSuccessHandler;
						} else if(action == "Convert") {
							url += ("convert/json?session=" + _this.session + "&file=/opt/tissuestack/upload/" + uploaded_file.value);
							actonType = TissueStack.Tasks.ReverseTypeLookupTable["Conversion"];
							successHandler = _this.conversionAndPreTileSuccessHandler;
						} else if(action == "PreTile") {
							url += ("tile/json?session=" + _this.session	+ "&file=" + TissueStack.configuration['upload_directory'].value + "/" 
									+ uploaded_file.value
									+ "&tile_dir=" + TissueStack.configuration['server_tile_directory'].value
									+ "&dimensions=0,0,0,0,0,0"	+ "&zoom=" + i + "&preview=true"); //+ "&store_data_set=true"
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
									var message = "Error: " + (data.error.message ? data.error.message : " Action wasn't performed!");
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
								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
								return;
							}
						);
				 		
				 		return;
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
			
			_this.registerDataSetWithAnds();
		}
	},
	conversionAndPreTileSuccessHandler : function(_this, response, file, type, status, zoom) {
		$(".file_radio_list input:radio[id^="+ "'check_" + file + "']:first").attr('disabled', true).checkboxradio("refresh");
		var task = {id: parseInt(response), file: file, type: type, status: status};
		if (typeof(zoom) == 'number') task.zoom = zoom;
		
		if (_this.addTask(task)) {
			_this.replaceErrorMessage("Action Has Been Added To The Queue!");
			$('.error_message').css("background", "#32CD32");
		} else {
			_this.replaceErrorMessage("Failed To Add Action To The Queue!");
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
		row.height = 20;
		row.id = "task_" + task.id;
		//$('tr').attr({"height" : 20});			
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
					processBar = TissueStack.Tasks.getStatusAsString(task.status);
					break;
				
				case 4: // Progress
					processBar = 
						'<div class="progress_bar_div" id="progress_bar_' + task.id + '"><progress class="progress_bar" id="progress_bar' + task.id + '" value="0" max="100"></progress>'
						+ '<span class="progress_bar_text" id="progress_text_' + task.id + '">0%</span ></div>';
					_this.queue_handles[task.id] = setInterval(function () {
						TissueStack.Utils.sendAjaxRequest(
							"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/progress/json?task_id="+ task.id,
							'GET', true,
							function(data, textStatus, jqXHR) {
								if (!data.response && !data.error) {
									_this.replaceErrorMessage("No Data Set Updated!");
									_this.stopTaskProgressCheck(task.id);
									return false;
								}
								if (data.error) {
									var message = "Error: " + (data.error.message ? data.error.message : " No DataSet Selected!");
									_this.replaceErrorMessage(message);
									_this.stopTaskProgressCheck(task.id);				
									return false;
								}
								if (data.response.noResults) {
									_this.replaceErrorMessage("No Results!");
									_this.stopTaskProgressCheck(task.id);
									return false;
								}
	
								var processTask = data.response;
					
								$("#" + "progress_bar_" + task.id).val(processTask.progress);
								$("#" + "progress_text_" + task.id).val(processTask.progress);
								
								if(processTask.progress == "100"){
									_this.displayUploadDirectory();
									_this.stopTaskProgressCheck(task.id);
								}
							},
							function(jqXHR, textStatus, errorThrown) {
								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
								_this.stopTaskProgressCheck(task.id);
								return false;
							}
						);  
					}, 25000);
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
		//$("#task_table").css({"height" : nrRows * 20 + 15});
		// refresh
		tab.trigger("create");
		
		return true;
	},
	removeTask : function(id) {
		if (typeof(id) != 'number') return;
		$("#" + id).remove();
		// $('#task_table').tri
	},
	stopTaskProgressCheck : function(id) {
		clearInterval(this.queue_handles[id]);
		this.queue_handles[id] = null;
	},
	registerTaskActionHandler : function() {
		var _this = this;
	},
	taskPasueHandler : function (process_task) {
		var _this = this;
		$('#conpause_' + process_task).click(function(){
		   TissueStack.Utils.sendAjaxRequest(
				"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/pause/json?" +
				"session=" + _this.session +
				"&task_id=" + process_task,
				'GET', true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("No Task ID Applied");
						return false;
					}
					if (data.error) {
						var message = "Error: " + (data.error.message ? data.error.message : " No Task ID Applied");
						_this.replaceErrorMessage(message);				
						return false;
					}
					if (data.response.noResults) {
						_this.replaceErrorMessage("No Results!");
						return false;
					}
						//Do something here !!
						$('#conresume_' + process_task).removeClass('ui-disabled');
						$('#conpause_' + process_task).addClass('ui-disabled');
						return false;
				},
				function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					return false;
				}
			);
		});
	},
	taskResumeHandler : function (process_task) {
		var _this = this;
		$('#conresume_' + process_task).click(function(){
		   TissueStack.Utils.sendAjaxRequest(
				"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/resume/json?" +
				"session=" + _this.session +
				"&task_id=" + process_task,
				'GET', true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("No Task ID Applied");
						return false;
					}
					if (data.error) {
						var message = "Error: " + (data.error.message ? data.error.message : " No Task ID Applied");
						_this.replaceErrorMessage(message);				
						return false;
					}
					if (data.response.noResults) {
						_this.replaceErrorMessage("No Results!");
						return false;
					}
						//Do something here !!
						$('#conpause_' + process_task).removeClass('ui-disabled');
						$('#conresume_' + process_task).addClass('ui-disabled');
						return false;
				},
				function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					return false;
				}
			);
		});
	},
	taskCancelHandler : function (process_task) {
		var _this = this;
		$('#conresume_' + process_task).click(function(){
		   TissueStack.Utils.sendAjaxRequest(
				"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/cancel/json?" +
				"session=" + _this.session +
				"&task_id=" + process_task,
				'GET', true,
				function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						_this.replaceErrorMessage("No Task ID Applied");
						return false;
					}
					if (data.error) {
						var message = "Error: " + (data.error.message ? data.error.message : " No Task ID Applied");
						_this.replaceErrorMessage(message);				
						return false;
					}
					if (data.response.noResults) {
						_this.replaceErrorMessage("No Results!");
						return false;
					}
						//Do something here !!
						$('#conpause_' + process_task).addClass('ui-disabled');
						$('#conresume_' + process_task).addClass('ui-disabled');
						$('#concancel_' + process_task).addClass('ui-disabled');
						$('#constart_' + process_task).addClass('ui-disabled');
						
						return false;
				},
				function(jqXHR, textStatus, errorThrown) {
					_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
					return false;
				}
			);
		});
	},
};