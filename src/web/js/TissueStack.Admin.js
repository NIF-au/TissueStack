//Temporary function for admin interface and filelistname
TissueStack.Admin = function () {
	TissueStack.Admin.prototype.adminInterface();
	TissueStack.Admin.prototype.showAllList();
	TissueStack.Admin.prototype.submitNewFile();
	TissueStack.Admin.prototype.createSession();
	TissueStack.Admin.prototype.checkCookie();
};

TissueStack.Admin.prototype = {
	createSession : function () {
		$('#login_btn').click(function(){
		 	var password = $('#password').val();
		 	if(password =="@minTischYu"){
				var xmlhttp = new XMLHttpRequest();
				xmlhttp.open("GET", "/backend/admin/new_session?" + password, true);
				
				xmlhttp.onreadystatechange = function(){
				    if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
				        alert("Done! Session created.");
				    }
				};
			}
			/*
			$.ajax({
	            type : "POST",	
	            url : "/backend/admin/new_session",
	            data : "Username=" + username + "&Password=" + password,
	            dataType : 'json',
	            cache : false,
	            success : function(data) {
	                if(data.error) {
	                    $('.login div.error').show().html(data.error);
	
	                } else {
	                    $('.login div.success').show().html(data.success);
	                }	
	            },
	            error : function(jqXHR, textStatus, errorThrown) {
	                alert("error " + textStatus + ": " + errorThrown);
	            },
	            beforeSend : function() {
	                $(".load").html("Loading...");
	            }
	        });
	        */
	        username= $('#username').val();
	    	TissueStack.Admin.prototype.checkCookie(username);
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
	checkCookie: function (f_name){
		var f_name=TissueStack.Admin.prototype.getCookie("username");
		if (f_name!=null && f_name!="")
		  {
		 	 alert("Welcome Back: " + f_name);
		  }
		else 
		  {
		  f_name= $('#username').val();
		  if (f_name!=null && f_name!="")
		    {
		    	TissueStack.Admin.prototype.setCookie("username",username,365);
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
	      $.getJSON("/backend/admin/filename_list/json",function(result){
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
			$('#uploadForm').ajaxForm(function() { 
			    $('#uploadForm').html("<div id='message'></div>");  
			    $('#message').html("<h2>File Uploaded!</h2>")  
			    .append("<p>I love eating mnc files. Please give me more ^^</p>")  
			    .hide()  
			    .fadeIn(1500, function() {  
			      $('#message').append("");  
			    }); 
			});			
			$('.file_radio_list').hide()
			.html("");
			TissueStack.Admin.prototype.showAllList();
		});
	},  
};


//TODO: create session and cookie listener	