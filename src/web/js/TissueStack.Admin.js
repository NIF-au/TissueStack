//Temporary function for admin interface and filelistname
TissueStack.Admin = function (session) {
	this.createSession(session);
	this.checkCookie(session);
	this.adminInterface();
	this.showAllList();
	this.submitNewFile();
	this.addToDataSet();
};

TissueStack.Admin.prototype = {
	session: null,
	createSession : function (session) {
		$('#login_btn').click(function(){
		 	var password = $('#password').val();
		 	if(!password ==""){		
				$.ajax({
					async: false,
					url :"backend/security/new_session/json?password="+ password,
					dataType : "json",
					cache : false,
					timeout : 30000,
					success: function(data, textStatus, jqXHR) {
						if (!data.response && !data.error) {
							alert("Did not receive any session, neither lose session ....");
							return;
						}	
						if (data.error) {
							var message = "Session Error: " + (data.error.message ? data.error.message : " no more session available. Please login again.");
							alert(message);
							return;
						}	
						if (data.response.noResults) {
							alert("Please login with right password!");
							return;
						}
						session= data.response;
						TissueStack.Admin.prototype.session = session.id;
						var value = $('#login_btn').val(100);
						TissueStack.Admin.prototype.checkCookie(session, value);
					},
					error: function(jqXHR, textStatus, errorThrown) {
						alert("Error connecting to backend: " + textStatus + " " + errorThrown);
					}
				});
			}
			$('#password').val("");
			$("div#panel").animate({
				height: "0px"
			}, "fast");
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
		var session_name=TissueStack.Admin.prototype.getCookie("session");
		
		if ($(value).val() != null){
			session_name = null;
		}
		
		if (session_name!=null && session_name!="")
		  {
		  	TissueStack.Admin.prototype.session = session_name;
		  	return;
		  }
		else 
		  {
		  session_name = session;
		  if (session_name!=null && session_name!="")
		    {
		   		TissueStack.Admin.prototype.setCookie("session",session_name.id,1);
		   		return;
		    }
		  }
	},
	adminInterface : function () {
		$("div.panel_button").click(function(){
			$("div#panel").animate({
				height: "500px"
			})
			.animate({
				height: "400px"
			}, "fast");
			$("div.panel_button").toggle();
		});	
		
	  	$("div#hide_button").click(function(){
			$("div#panel").animate({
				height: "0px"
			}, "fast");
	   	});	
	},
	showAllList : function (){  
	     $(".file_radio_list").show(function(){
	      TissueStack.Utils.sendAjaxRequest("/backend/admin/upload_directory/json", "GET", true,function(result) {
	        $.each(result, function(i, field){
	        	var listOfFileName = "";
	        	for (i in field){
		        	content = '<input type="checkbox" name='+'"check_'+[i]+'" id="'+ 'check_'+ [i] + '" value="check_'+[i]+'" />'+
		        			  '<label for="'+'check_'+ [i] +'">'+ field[i] + '</label>';
		        	listOfFileName += content; 
	           	 }
              $('.file_radio_list').fadeIn(1500, function() {  
              	 $('.file_radio_list').append(listOfFileName)
              	 	.trigger( "create" ); 
              	 $('.file_radio_list').controlgroup();
              });
	        });
	      });
	    });
	},
	submitNewFile : function () {
		 $("#uploadForm").submit(function(){
			$(this).ajaxSubmit({ 	
				url :"/backend/admin/upload/json?session=" + TissueStack.Admin.prototype.session,
				dataType : "json",
				success: function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						TissueStack.Admin.prototype.replaceErrorMessage("No File Submission!");
						return false;
					}
					
					if (data.error) {
						var message = "Error: " + (data.error.message ? data.error.message : " No File Submission!");
						TissueStack.Admin.prototype.replaceErrorMessage(message);				
						return false;
					}
					
					if (data.response.noResults) {
						TissueStack.Admin.prototype.replaceErrorMessage("No Results!");
						return false;
					}
					TissueStack.Admin.prototype.replaceErrorMessage("File Has Been Successfully Uploaded!");
					$('.error_message').css("background", "#4ea8ea"); 
					$('.file_radio_list').fadeOut(500, function() { 
						$('.file_radio_list').html("");
					 	TissueStack.Admin.prototype.showAllList();
					});
				},
				error: function(jqXHR, textStatus, errorThrown) {
					alert("Error connecting to backend: " + textStatus + " " + errorThrown);
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

		$('#file_uploaded_message').html("<div class='error_message'></div>");  		
		$('.error_message').html(message + "<br/>")  
		.append().hide().fadeIn(1500, function() {  
		  $('.error_message').append("");  
		})
		.fadeOut(5000); 	
	},
	addToDataSet : function () {
		//USED FOR ADD DATASET TO DATA TREE IN MAIN VIEW
		$("#bt_add_dataset").click(function(){
			TissueStack.Utils.sendAjaxRequest("/backend/admin/upload_directory/json", "GET", true,function(result) {
			  	$.each(result, function(i, field){
				  	for (i in field){
	 					if ($('#check_'+[i]).is(':checked')) {
	 						var msgDescription = $('#txtDesc').val();
	 						if (msgDescription == ""){
	 							TissueStack.Admin.prototype.replaceErrorMessage("Please Add Description Before Adding New Data Set!");
	 							return;
	 						}
	 						$("#bt_add_dataset").ajaxSubmit({ 	
	 							url :"/backend/admin/update_dataset/json?filename=" + field[i] + "&description=" + msgDescription,
	 							dataType : "json",
	 							success: function(data, textStatus, jqXHR) {
	 								if (!data.response && !data.error) {
	 									TissueStack.Admin.prototype.replaceErrorMessage("No Data Set Updated!");
	 									return false;
	 								}
	 								if (data.error) {
	 									var message = "Error: " + (data.error.message ? data.error.message : " No Data Set Updated!");
	 									TissueStack.Admin.prototype.replaceErrorMessage(message);				
	 									return false;
	 								}
	 								if (data.response.noResults) {
	 									TissueStack.Admin.prototype.replaceErrorMessage("No Results!");
	 									return false;
	 								}
	 								TissueStack.Admin.prototype.replaceErrorMessage("DataSet update successfully. Please go back to main canvas!");
	 								$('.error_message').css("background", "#32CD32");
	 							},
	 							error: function(jqXHR, textStatus, errorThrown) {
	 								alert("Error connecting to backend: " + textStatus + " " + errorThrown);
	 								return false;
	 							}
	 						});
	 						return false;
				   		}
			    	};
			  	});
			});
		});
	}
};