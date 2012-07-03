//Temporary function for admin interface and filelistname
TissueStack.Admin = function (session) {
	this.createSession(session);
	this.checkCookie(session);
	this.adminInterface();
	this.showAllList();
	this.submitNewFile();
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
						TissueStack.Admin.prototype.checkCookie(session);
					},
					error: function(jqXHR, textStatus, errorThrown) {
						alert("Error connecting to backend: " + textStatus + " " + errorThrown);
					}
				});
			}
			TissueStack.Admin.prototype.clearText();
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
	checkCookie: function (session){
		var session_name=TissueStack.Admin.prototype.getCookie("session");
		if (session_name!=null && session_name!="")
		  {
		  	TissueStack.Admin.prototype.session = session_name;
		  	$('#uploadForm').attr('action', '/backend/admin/upload/json?session='+ session_name);
		  }
		else 
		  {
		  session_name = session;
		  if (session_name!=null && session_name!="")
		    {
		    TissueStack.Admin.prototype.setCookie("session",session_name.id,1);
		    $('#uploadForm').attr('action', '/backend/admin/upload/json?session='+ session_name.id);
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
	      $.getJSON("/backend/admin/upload_directory/json",function(result){
	        $.each(result, function(i, field){
	        	var listOfFileName = "";
	        	for (i in field){
		        	content = '<input type="checkbox" name='+'"radio_'+[i]+'" id="'+ 'radio_choice_'+ [i] + '" value="choice-'+[i]+'" />'+
		        			  '<label for="'+'radio_choice_'+ [i] +'">'+ field[i] + '</label>';
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
	    $(".submit_new_file").click(function(){
			//TODO: fix it later for the exists file validation
			$.getJSON("/backend/admin/upload_directory/json",function(result){
			  $.each(result, function(i, field){
			  	for (i in field){
					if(field[i] == $('#filename_1').val()){
						var first_msg = "<p>Already in the upload destination!</p>";
						var second_msg = "<h2>File " + $('#filename_1').val() + " exists</h2>";
						TissueStack.Admin.prototype.fileUploadMessage(first_msg, second_msg);
						return;
					}
			      }
			  });
			});
			
			if(TissueStack.Admin.prototype.session == null || $('#filename_1').val()=="" || (TissueStack.Admin.prototype.session == null && $('#filename_1').val()!="")){
				var first_msg = (TissueStack.Admin.prototype.session == null? "Please login again first":"Please select file to upload");
				var second_msg = (TissueStack.Admin.prototype.session == null?"<h2>No more session available</h2>":"<h2>No file was selected</h2>");
				TissueStack.Admin.prototype.fileUploadMessage(first_msg, second_msg);
				return;
			}
			
			if (!TissueStack.Admin.prototype.session == "" && $('#filename_1').val()!=""){    	
				var first_msg = "<p>I love eating mnc files. Please give me more ^^</p>";
				var second_msg = "<h2>File Uploaded!</h2>";
				TissueStack.Admin.prototype.fileUploadMessage(first_msg, second_msg);
				return;
			 }
		});
	},
	fileUploadMessage : function (first_msg, second_msg) {
		$('#uploadForm').ajaxForm(function() { 
		    $('#uploadForm').html("<div id='message'></div>");  
		    $('#message').html(second_msg)  
		    .append(first_msg)  
		    .hide()  
		    .fadeIn(1500, function() {  
		      $('#message').append("");  
		    }); 
		});
		$('.file_radio_list').fadeOut(500, function() { 
			$('.file_radio_list').html("");
		 	TissueStack.Admin.prototype.showAllList();
		});
	},
	clearText : function () {
		$('#username').val("");
		$('#password').val("");
	},
	
	//TODO: using ajax to get server message instead of checking by session and filename <-temp not working in this way but will fix later
	submitNewFileTemp : function () {
		$('#uploadForm').live('submit',function(){
			$.ajax({
				async: false,
				url :"/backend/admin/upload/json?session="+ TissueStack.Admin.prototype.session,
				dataType : "json",
				contentType: 'multipart/form-data',
				processData: false,
				cache : false,
				timeout : 30000,
				success: function(data, textStatus, jqXHR) {
					if (!data.response && !data.error) {
						alert("message1");
						return;
					}
					
					if (data.error) {
						var message = "Error: " + (data.error.message ? data.error.message : " message2.");
						alert(message);
						return;
					}
					
					if (data.response.noResults) {
						alert("message3!");
						return;
					}
				},
				error: function(jqXHR, textStatus, errorThrown) {
					alert("Error connecting to backend: " + textStatus + " " + errorThrown);
				}
			});
			TissueStack.Admin.prototype.clearText();
			$("div#panel").animate({
				height: "0px"
			}, "fast");
		});
	}  
};