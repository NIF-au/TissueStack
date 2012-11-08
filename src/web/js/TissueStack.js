if (typeof(TissueStack) == 'undefined') {
        TissueStack = {};
};

TissueStack.configuration = {
		restful_service_proxy_path : 
			{ 	value: "backend", 
				description: "restful java service proxy path (relative to the application''s web root directory)"
			}
};
TissueStack.debug = true;
TissueStack.color_maps = null;
TissueStack.sync_datasets = false;
TissueStack.overlay_datasets = false;
TissueStack.planes_swapped = false;
TissueStack.indexed_color_maps = {
	"grey" : null,
	"hot" : null,
	"spectral" : null
};
TissueStack.tasks = {};
TissueStack.cookie_lock = false;
