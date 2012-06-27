//Temporary function for admin interface and filelistname
TissueStack.AdminInterface = function () {
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
TissueStack.FileManagement = function () {    
    $(".file_radio_list").show(function(){
      $.getJSON("/backend/admin/filename_list/json?",function(result){
        $.each(result, function(i, field){
        	var listOfFileName = "";
        	for (i in field){
	        	//content = '<div style="text-align: left">'+'File Name: '+ field[i] + '</div><br/>';
	        	content = '<input type="checkbox" name='+'"radio_'+[i]+'" id="'+ 'radio_choice_'+ [i] + '" value="choice-'+[i]+'" />'+
	        			  '<label for="'+'radio_choice_'+ [i] +'">'+ field[i] + '</label>';
	        	listOfFileName += content; 
           	 }
             $('.file_radio_list').append(listOfFileName);
        });
      });
    });
}
//TODO: create session and cookie listener	