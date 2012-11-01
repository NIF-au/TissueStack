TissueStack.Tasks = {
	TypeLookupTable   : ["Tiling", "Conversion"],
	ReverseTypeLookupTable   : {"Tiling" : 1, "Conversion" : 2},
	StatusLookupTable : ["Running", "Paused", "Canceled", "Queued"],
	ReverseStatusLookupTable : {"Running" : 1, "Paused": 2, "Canceled": 3, "Queued": 4},	
	getStatusAsString : function(status_as_number) {
		if (typeof(status_as_number) !== 'number' || status_as_number < 1
				|| status_as_number > TissueStack.Tasks.StatusLookupTable.length) return;
		
		return TissueStack.Tasks.StatusLookupTable[status_as_number-1];
	},
	getStatusFromString : function(status_as_string) {
		if (typeof(status_as_string) !== 'string') return;
		
		return TissueStack.Tasks.ReverseStatusLookupTable[status_as_string];
	},
	getTypeAsString : function(type_as_number) {
		if (typeof(type_as_number) !== 'number'  || type_as_number < 1
				|| type_as_number > TissueStack.Tasks.TypeLookupTable.length) return;
		
		return TissueStack.Tasks.TypeLookupTable[type_as_number-1];
	},
	getTypeFromString : function(type_as_string) {
		if (typeof(type_as_string) !== 'string') return;
		
		return TissueStack.Tasks.ReverseTypeLookupTable[type_as_string];
	},
	addOrUpdateTask : function(task) {
		if (typeof(task) !== 'object') return;
		
		if (typeof(task.id) !== "number" || typeof(task.file) !== "string"
			|| typeof(task.type) !== "number" || typeof(task.status) !== "number") return;
		
		if (typeof(TissueStack.tasks) != 'object') TissueStack.tasks = {};
		
		TissueStack.tasks[task.id] = task;
		
		return task;
	}, removeTask : function(id) {
		if (typeof(id) != 'number' || typeof(TissueStack.tasks[id]) != 'object') return;
		
		delete TissueStack.tasks[id];
	}, writeToCookie : function() {
		if (typeof(TissueStack.tasks) != 'object') return true;
		
		var content = "";
		// loop through all tasks
		for (var t in TissueStack.tasks) {
			if (TissueStack.tasks[t].status == TissueStack.Tasks.StatusLookupTable["Canceled"]) continue; // don't persist
			
			content += (TissueStack.tasks[t].id + "=");
			for (var p in TissueStack.tasks[t]) {
				if (p == "id") continue;
				content += (p + ":" + TissueStack.tasks[t][p] + "|");
			}
			content += ",";
		}

		if (content == "") return true;

		// check lock
		if (typeof(TissueStack.cookie_lock) != 'boolean' && TissueStack.cookie_lock) return false;
		// set cookie lock
		TissueStack.cookie_lock = true;
		
		var exdate=new Date();
		exdate.setDate(exdate.getDate() + 365);	
		
		$.cookie("tasks", content, { expires: exdate });
		TissueStack.cookie_lock = false;
	}, readFromCookie : function() {
		var content = $.cookie("tasks");
		if (typeof(content) != 'string') return;
		
		var tasks = content.split(',');
		if (!tasks || tasks.length == 0) return;

		if (typeof(TissueStack.tasks) != 'object' || !TissueStack.tasks) TissueStack.tasks = {};
		
		for (var x=0;x<tasks.length;x++) {
			// check for id
			var posEquals = tasks[x].indexOf("=");
			if (posEquals <=0) return;
			
			// extract id
			var id = parseInt(tasks[x].substring(0,posEquals));
			var taskProperties = tasks[x].substring(posEquals + 1); 
			
			// create object
			var taskObject = {};
			taskObject.id = id;
			
			var props = taskProperties.split('|');
			for (var y=0;y<props.length;y++) {
				var keyValuePair = props[y].split(':');
				if (!keyValuePair || keyValuePair.length < 2 || keyValuePair.length > 2)
					TissueStack.Tasks.addOrUpdateTask(taskObject); // add task
				else {
					// convert numbers if they are numbers, otherwise leave them alone
					var value = keyValuePair[1];
					try {
					  value = parseInt(value);
					  if (isNaN(value)) value = keyValuePair[1];
					} catch(noNumber) {
						value = keyValuePair[1];
					}
					taskObject[keyValuePair[0]] = value; // add property
				}
			} 
		}
	}
};

