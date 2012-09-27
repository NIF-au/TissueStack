if (typeof(TissueStack) == 'undefined') {
        TissueStack = {};
};

TissueStack.configuration = {};
TissueStack.debug = true;
TissueStack.tile_directory = "tiles/";
TissueStack.color_maps = null;
TissueStack.sync_datasets = false;
TissueStack.indexed_color_maps = {
	"grey" : null,
	"hot" : null,
	"spectral" : null
};
