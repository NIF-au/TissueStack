TissueStack.Admin = function () {
	this.registerCreateSessionHandler();
	this.checkCookie();
	this.registerLoginHandler();
	this.registerFileUpload();
	this.registerAddToDataSetHandler();
	this.displayUploadDirectory();
	this.registerConvertHandler();
	//this.registerPreTileHandler();
};

TissueStack.Admin.prototype = {
	session: null,
	queue_handle : null,
	processType: "",
	registerCreateSessionHandler : function () {
	 	var _this = this;

		$('#login_btn').click(function(){
		 	var password = $('#password').val();
		 	if(password != "") {
		 		TissueStack.Utils.sendAjaxRequest(
	 				"backend/security/new_session/json?password="+ password, 'GET', true,
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
		var c_value=escape(value) + ((exdays==null) ? "" : "; expires="+exdate.toUTCString());
		document.cookie=c_name + "=" + c_value;
	},
	checkCookie: function (session, value){
		var session_name=this.getCookie("session");
		
		if ($(value).val() != null){
			session_name = null;
		}
		
		if (session_name!=null && session_name !="")
		  {
		  	this.session = session_name;
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
		_this = this;  
	     $(".file_radio_list").show(function(){
	      TissueStack.Utils.sendAjaxRequest("/backend/admin/upload_directory/json", "GET", true,function(result) {
	    	if (!result || !result.response || result.response.length == 0) {
	    		return;
	    	}
	    	// display results
        	var listOfFileName = "";	    	
	        $.each(result.response, function(i, uploadedFile){
	        	content = '<input type="radio" name="radio_listFile" class="uploaded_file" id="check_' + i + '" value="' + uploadedFile + '" />'
	        			+ '<label for="'+'check_'+ i +'">'+ uploadedFile + '</label>';
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
				 	 $("#radio_task input:radio[id^='bt_preTile']:first").attr('disabled', true).checkboxradio("refresh"); //temp for development
				 	 $('#radio_task').fadeIn();
				});
			}
			if($(this).val().split('.').pop() == "mnc" || $(this).val().split('.').pop() == "ini") {
				$('#radio_task').fadeOut(250, function() {  
				 	 $("#radio_task input:radio[id^='bt_convert']:first").attr('disabled', false).checkboxradio("refresh");
				 	 $("#radio_task input:radio[id^='bt_preTile']:first").attr('disabled', true).checkboxradio("refresh"); //temp for development
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
				url :"/backend/admin/upload/json?session=" + _this.session,
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
	registerAddToDataSetHandler : function () {
		var _this = this;
		$("input[name=radio_task]").change(function() {
			if($('input[name=radio_task]:checked').val() == "rad_addDataSet") {
				$("#bt_process").click(function(){
					  	$.each($('.uploaded_file'), function(i, uploaded_file) {
		 					if (uploaded_file.checked) {
		 						var msgDescription = $('#txtDesc').val();
		 						if (msgDescription == ""){
		 							_this.replaceErrorMessage("Please Add Description Before Adding New Data Set!");
		 							return false;
		 						}
		 						// send backend request
		 				 		TissueStack.Utils.sendAjaxRequest(
		 							"/backend/admin/add_dataset/json?session=" + _this.session + "&filename=" + uploaded_file.value + "&description=" + msgDescription,
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
		 				 		
		 				 		// we only process the first that has been checked
		 				 		return false;
					   		}
		 				// we only come here if no file was selected	
		 				_this.replaceErrorMessage("Please check a file that you want to add!");
				  	});
				});
			}
		});
	},
	registerConvertHandler : function () {
		_this = this;
			$("input[name=radio_task]").change(function() {
				if($(this).val() == "CONVERT") {
					// send backend request
					$("#bt_process").click(function(){
					   TissueStack.Utils.sendAjaxRequest(
							"/backend/admin/convert/json?file=/opt/upload/" + $('input[name=radio_listFile]:checked').val(),
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
									_this.createTaskView(data.response);
									return false;
							},
							function(jqXHR, textStatus, errorThrown) {
								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
								return false;
							}
						);
					});
				}
			});
		
	},
	/*registerPreTileHandler : function () {
		_this = this;
		$("input[name=radio_task]").change(function() {
			if($(this).val() == "CONVERT") {
				
			}
		});
	},*/
	createTaskView: function (process_task) {
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
					//processBar = i;
				}

				if(j == 1){ // file name
					processBar = $('input[name=radio_listFile]:checked').val();
				}

				if(j == 2){ // give ID as well
					processBar = $('input[name=radio_task]:checked').val();
				}
				
				if(j == 3){ // give ID as well
					processBar = '<div data-role="controlgroup" data-type="horizontal">'
							   + '<a id=' + 'constart_' + i + ' data-role="button" data-theme="c" data-icon="arrow-r" data-iconpos="notext">Start</a>'
							   + '<a id=' + 'conrefresh_' + i + ' data-role="button" data-theme="c" data-icon="refresh" data-iconpos="notext">Resume</a>'
							   + '<a id=' + 'constop_' + i + ' data-role="button" data-theme="c" data-icon="delete" data-iconpos="notext">Cancel</a>'
							   + '</div>';
				}
				
				if(j == 4){ // give ID as well
					_this.queue_handle = setInterval(function () {
						TissueStack.Utils.sendAjaxRequest(
							"/backend/admin/progress/json?task_id="+ process_task,
							'GET', true,
							function(data, textStatus, jqXHR) {
								if (!data.response && !data.error) {
									_this.replaceErrorMessage("No Data Set Updated!");
									return false;
								}
								if (data.error) {
									var message = "Error: " + (data.error.message ? data.error.message : " No DataSet Selected!");
									_this.replaceErrorMessage(message);				
									return false;
								}
								if (data.response.noResults) {
									_this.replaceErrorMessage("No Results!");
									return false;
								}
	
								var processTask = data.response;
									content = '<progress id="bar" value="'+ processTask.progress +'" max="100"></progress>'
											+ '<span style="text-align: left">'+ ' ' + processTask.progress.toFixed(2) +'%</span >';  
									processBar = content;
									$(cell).html(processBar);
									$(row).append(cell);
									if(processTask.progress == "100"){
										_this.displayUploadDirectory();
										clearInterval(_this.queue_handle);
									}
									return false;
							},
							function(jqXHR, textStatus, errorThrown) {
								_this.replaceErrorMessage("Error connecting to backend: " + textStatus + " " + errorThrown);
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
};