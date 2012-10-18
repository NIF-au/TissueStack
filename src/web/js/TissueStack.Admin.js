TissueStack.Admin = function () {
	this.registerCreateSessionHandler();
	this.checkCookie();
	this.registerLoginHandler();
	this.registerFileUpload();
	this.registerAddToDataSetHandler();
	this.refreshQueryList();
	this.displayUploadDirectory();
	this.registerConvertHandler();
	this.registerPreTileHandler();
};

TissueStack.Admin.prototype = {
	session: null,
	queue_handle : null,
	processType: "",
	progress_task_id: "",
	pre_tile_task_add: "",
	refreshQueryList : function () {
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
		$.cookie(c_name,c_value, 1);
	},
	checkCookie: function (session, value){
		var session_name=this.getCookie("session");
		
		if ($(value).val() != null){
			session_name = null;
		}
		
		if (session_name!=null && session_name !="")
		  {
		  	this.session = session_name;		  	
		  	
		  	if($.cookie("CVT") != null){
		  		var CVTcookie = document.cookie.split("; ");
		  		for (i = 0 ; i < CVTcookie.length ; i++)
		  		  {
		  		  	if(CVTcookie[i].substr(0,CVTcookie[i].indexOf("=")) == "CVT"){
		  		  		var convert_cookie = CVTcookie[i].substr(CVTcookie[i].indexOf("=")+1);
		  		  	}
		  		  }
		  		  var cv_task, cv_file, cv_type, convert_table = convert_cookie.split(":");
		  		  
		  		  if(convert_table.length <=3)
		  		  {
		  		  	cv_task = convert_table[0].substr(convert_table[0].indexOf("=")+ 1);
		  		  	cv_file = convert_table[1].substr(convert_table[1].indexOf("=")+ 1);
		  		  	cv_type = convert_table[2].substr(convert_table[2].indexOf("=")+ 1);
		  		  }
		  		this.createTaskView(cv_task, cv_file, cv_type, null);
		  		this.taskPasueHandler(cv_task); 
		  		this.taskResumeHandler(cv_task);
		  	}
		  	if($.cookie("PTL") != null){
		  		var PTLcookie = document.cookie.split("; "); //"; " space required after ";". Don't remove it. 
		  		for (i = 0 ; i < PTLcookie.length ; i++)
		  		  {
		  		  	if(PTLcookie[i].substr(0,PTLcookie[i].indexOf("=")) == "PTL"){
		  		  		var pretile_cookie = PTLcookie[i].substr(PTLcookie[i].indexOf("=")+1);
		  		  	}
		  		  }
		  		  var pt_task, pt_name, pt_file, pt_type, pretile_table = pretile_cookie.split(":");
		  		  
		  		  for( i = 0; i < pretile_table.length ; i ++)
		  		  {	
		  		  	pt_name = pretile_table[i].substr(0,pretile_table[i].indexOf("="));
		  		  	if(pt_name == "pt_file"){
		  		  		pt_file = pretile_table[i].substr(pretile_table[i].indexOf("=")+ 1);
		  		  	}else if ( pt_name == "pt_type") {
		  		  		pt_type = pretile_table[i].substr(pretile_table[i].indexOf("=")+ 1);
		  		  	}
		  		  }
		  		  
		  		  for( i = 0; i < pretile_table.length ; i ++)
		  		  {	
		  		    pt_name = pretile_table[i].substr(0,pretile_table[i].indexOf("="));
		  		  	if( pt_name != "pt_file" && pt_name != "pt_type")
		  		  	{
		  		  		pt_task = pretile_table[i].substr(pretile_table[i].indexOf("=")+ 1);
		  		  		this.createTaskView(pt_task, pt_file, pt_type, i);
		  		  		this.taskPasueHandler(pt_task); 
		  		  		this.taskResumeHandler(pt_task);
		  		  	}
		  		  }
		  	}
		  	return;
		  }
		else 
		  {
		  session_name = session;
		  if (session_name!=null && session_name!="")
		    {
		   		this.setCookie("session",session_name.id,1);
		   		return;
		    }
		  }
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
	registerFileUpload : function () {
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
	registerAddToDataSetHandler : function () {
		var _this = this;
		$("#bt_process").click(function(){
			if($('input[name=radio_task]:checked').val() == "rad_addDataSet") {
					// TODO: This is only here for easier testing
					// once finished => move the following line into the success handler (called AFTER successful ds addition)
					_this.registerDataSetWithAnds();
						
				  	$.each($('.uploaded_file'), function(i, uploaded_file) {
	 					if (uploaded_file.checked) {
	 						var msgDescription = $('#txtDesc').val();
	 						if (msgDescription == ""){
	 							_this.replaceErrorMessage("Please Add Description Before Adding New Data Set!");
	 							return false;
	 						}
	 						// send backend request
	 				 		TissueStack.Utils.sendAjaxRequest(
	 							"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/add_dataset/json?session=" + _this.session + "&filename=" + uploaded_file.value + "&description=" + msgDescription,
	 							'GET', true,
	 							function(data, textStatus, jqXHR) {
	 								if (!data.response && !data.error) {
	 									_this.replaceErrorMessage("No Data Set Updated!");
	 									return false;
	 								}
	 								if (data.error) {
	 									var message = "Error: " + (data.error.message ? data.error.message : " No Data Set Updated!");
	 									_this.replaceErrorMessage(message);				
	 									return false;
	 								}
	 								if (data.response.noResults) {
	 									_this.replaceErrorMessage("No Results!");
	 									return false;
	 								}
	
	 								var dataSet = data.response;
									var addedDataSet = TissueStack.dataSetStore.addDataSetToStore(dataSet, "localhost");
	 								if (addedDataSet) {
	 									if(TissueStack.desktop)	TissueStack.dataSetNavigation.addDataSetToDynaTree(addedDataSet);
		 								if (TissueStack.tablet) TissueStack.dataSetNavigation.addDataSetToTabletTree(addedDataSet);
		 								_this.displayUploadDirectory();
		 								_this.replaceErrorMessage("Data Set Has Been Added Successfully!");
		 								$('.error_message').css("background", "#32CD32");
		 								return false;
	 								}
	 							},
	 							function(jqXHR, textStatus, errorThrown) {
	 								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
	 								return false;
	 							}
	 						);
	 				 		return false;
				   		}
	 				// we only come here if no file was selected	
	 				_this.replaceErrorMessage("Please check a file that you want to add!");
			  	});
			}
		});
	},
	registerConvertHandler : function () {
		var _this = this;
		$("#bt_process").click(function(){
			if($('input[name=radio_task]:checked').val() == "Convert") {
			   TissueStack.Utils.sendAjaxRequest(
					"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/convert/json?" +
					"session=" + _this.session +
					"&file=/opt/tissuestack/upload/" +
					$('input[name=radio_listFile]:checked').val(),
					'GET', true,
					function(data, textStatus, jqXHR) {
						if (!data.response && !data.error) {
							_this.replaceErrorMessage("No Data Set Updated!");
							return false;
						}
						if (data.error) {
							var message = "Error: " + (data.error.message ? data.error.message : " No Data Set Updated!");
							_this.replaceErrorMessage(message);				
							return false;
						}
						if (data.response.noResults) {
							_this.replaceErrorMessage("No Results!");
							return false;
						}
							var checked_listFile_Name = $('input[name=radio_listFile]:checked').val();
							var checked_task_Name = $('input[name=radio_task]:checked').val();
							
							_this.progress_task_id = data.response;
							$(".file_radio_list input:radio[id^="+ "'check_" + checked_listFile_Name + "']:first").attr('disabled', true).checkboxradio("refresh");
							_this.createTaskView(_this.progress_task_id, checked_listFile_Name, checked_task_Name, null);
							
							//pass new cookie so that users won't lost task table after refresh!
							document.cookie = "CVT=" + "cv_tk=" + _this.progress_task_id 
													 + ":cv_file=" + checked_listFile_Name 
													 + ":cv_type=" + checked_task_Name;
							
							_this.taskPasueHandler(_this.progress_task_id); 
							_this.taskResumeHandler(_this.progress_task_id);
							
							return false;
					},
					function(jqXHR, textStatus, errorThrown) {
						_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
						return false;
					}
				);
			}
		});
	},
	registerPreTileHandler : function () {
		var _this = this;
		var task_zoom_level = eval(TissueStack.configuration.default_zoom_levels.value).length;
		$("#bt_process").click(function(){
			if($('input[name=radio_task]:checked').val() == "PreTile") {
			   for(var i = 0; i < task_zoom_level ; i++){
				   (function (i) {
					   TissueStack.Utils.sendAjaxRequest(
							"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/tile/json?"
								+ "session=" + _this.session
								+ "&file=" + TissueStack.configuration['upload_directory'].value + "/" 
								+ $('input[name=radio_listFile]:checked').val()
								+ "&tile_dir=" + TissueStack.configuration['server_tile_directory'].value
								+ "&dimensions=0,0,0,0,0,0" 
								+ "&zoom=" + i 
								+ "&preview=true",
								//+ "&store_data_set=true",
							'GET', true,
							function(data, textStatus, jqXHR) {
								if (!data.response && !data.error) {
									_this.replaceErrorMessage("No Data Set To Be Tiled!");
									return false;
								} 
								if (data.error) {
									var message = "Error: " + (data.error.message ? data.error.message : " No Data Set To Be Tiled!");
									_this.replaceErrorMessage(message);				
									return false;
								}
								if (data.response.noResults) {
									_this.replaceErrorMessage("No Results!");
									return false;
								}
									var checked_listFile_Name = $('input[name=radio_listFile]:checked').val();
									var checked_task_Name = $('input[name=radio_task]:checked').val();
									
									_this.progress_task_id = data.response;
									$(".file_radio_list input:radio[id^="+ "'check_" + checked_listFile_Name + "']:first").attr('disabled', true).checkboxradio("refresh");
									_this.createTaskView(_this.progress_task_id, checked_listFile_Name, checked_task_Name, i);
									
									//pass new cookie so that users won't lost task table after refresh!
									_this.pre_tile_task_add += "pt_tk_" + i + "=" + _this.progress_task_id + ":"; 
									
									document.cookie = "PTL=" + _this.pre_tile_task_add
															 + "pt_file=" + checked_listFile_Name 
														 	 + ":pt_type=" + checked_task_Name;
														 	 
									_this.taskPasueHandler(_this.progress_task_id); 
									_this.taskResumeHandler(_this.progress_task_id);
																										
									return false;
							},
							function(jqXHR, textStatus, errorThrown) {
								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
								return false;
							}
						);
					})(i);
				}
			}
		});
	},
	createTaskView: function (process_task, process_file, process_type, zoom_level) {
		var _this = this;
		var nrCols = 5; //important! can't change!
		var maxRows = 0;
		var nrRows = maxRows+1;
		
		var tab = $('#task_table');
		var tbody = ('#task_table tbody');
		
		var row, cell;
		for(var i = 0; i < nrRows; i++){
			row=document.createElement('tr');
			for(var j = 0; j < nrCols; j++){
				var processBar = "" ;
				cell=document.createElement('td');
				
				if(j == 0){ // ID
					processBar = process_task;
				}

				if(j == 1){ // File Name
					processBar = process_file;
				}

				if(j == 2){ // Type
					processBar = process_type;
				}
				
				if(j == 3){ // Action
					processBar = '<div data-role="controlgroup" data-type="horizontal">'
							   + '<a id=' + 'constart_' + process_task 
							   + ' data-role="button" data-theme="c" data-icon="arrow-r" data-iconpos="notext" class="ui-disable ui-disabled">Start</a>'
							   + '<a id=' + 'conresume_' + process_task 
							   + ' data-role="button" data-theme="c" data-icon="refresh" data-iconpos="notext" class="ui-disable ui-disabled">Resume</a>'
							   + '<a id=' + 'conpause_' + process_task 
							   + ' data-role="button" data-theme="c" data-icon="info" data-iconpos="notext">Pasue</a>'
							   + '<a id=' + 'constop_' + process_task 
							   + ' data-role="button" data-theme="c" data-icon="delete" data-iconpos="notext" class="ui-disable ui-disabled">Cancel</a>'
							   + '</div>';
				}
				
				if(j == 4){ // Progress
					_this.queue_handle = setInterval(function () {
						TissueStack.Utils.sendAjaxRequest(
							"/" + TissueStack.configuration['restful_service_proxy_path'].value + "/admin/progress/json?task_id="+ process_task,
							'GET', true,
							function(data, textStatus, jqXHR) {
								if (!data.response && !data.error) {
									_this.replaceErrorMessage("No Data Set Updated!");
									_this.stopQueue();
									return false;
								}
								if (data.error) {
									var message = "Error: " + (data.error.message ? data.error.message : " No DataSet Selected!");
									_this.replaceErrorMessage(message);
									_this.stopQueue();				
									return false;
								}
								if (data.response.noResults) {
									_this.replaceErrorMessage("No Results!");
									_this.stopQueue();
									return false;
								}
	
								var processTask = data.response;
									if(zoom_level == null){
										content = '<progress id="bar" value="'+ processTask.progress +'" max="100"></progress>'
												+ '<span id="progess_in_process" style="text-align: left">'+ ' ' + processTask.progress.toFixed(2) +'%</span >'; 
									}
									else{
										content = '<progress id="bar" value="'+ processTask.progress +'" max="100"></progress>'
												+ '<span id="progess_in_process" style="text-align: left">'+ ' ' + processTask.progress.toFixed(2) +'%</span >'
												+ '<span id="progess_in_zoom_level" style="text-align: left">  ( ZOOM: '+ ' ' + zoom_level +' ) </span >'; 
									}
									processBar = content;
									$(cell).html(processBar);
									$(row).append(cell);
									
									//_this.taskPasueHandler(process_task); 
									//_this.taskResumeHandler(process_task);
									
									if(processTask.progress == "100"){
										_this.displayUploadDirectory();
										_this.stopQueue();
									}
									return false;
							},
							function(jqXHR, textStatus, errorThrown) {
								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
								_this.stopQueue();
								return false;
							}
						);  
					}, 2500);
				}
				$(cell).append(processBar);
				$(row).append(cell);
			}
			$(tbody).append(row);
		}	
		tab.trigger("create");
	},
	stopQueue : function() {
		clearInterval(this.queue_handle);
		this.queue_handle = null;
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
	taskCancelHandler : function () {
	
	},
};