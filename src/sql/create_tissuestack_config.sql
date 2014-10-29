-- !!!!! SWITCH TO tissuestack NOW before executing the following lines !!!!!

-- GENERAL INSTANCE CONFIGURATION - some default values
INSERT INTO configuration VALUES('version', '1.4', 'version');
INSERT INTO configuration VALUES('session_timeout_minutes', '15', 'The session timeout given in minutes');
INSERT INTO configuration VALUES('admin_passwd', '101ee9fe7aceaa8bea949e75a529d796da02e08bced78c6c4dde60768183fa14', 'Admin Password');
INSERT INTO configuration VALUES('server_proxy_path', 'server', 'server proxy path (relative to the application''s web root directory)');
INSERT INTO configuration VALUES('tile_directory', 'tiles', 'tile directory (relative to the application''s web root directory)');
INSERT INTO configuration VALUES('server_tile_directory', '/opt/tissuestack/tiles', 'server side tile directory for pre-tiling');
INSERT INTO configuration VALUES('upload_directory', '/opt/tissuestack/upload', 'upload directory (absolute system path on server)');
INSERT INTO configuration VALUES('lookup_tables_directory', '/opt/tissuestack/lookup', 'directory that houses the lookup table files (absolute system path on server)');
INSERT INTO configuration VALUES('colormaps_directory', '/opt/tissuestack/colormaps', 'directory that houses the color map files (absolute system path on server)');
INSERT INTO configuration VALUES('tasks_queue_file', '/opt/tissuestack/tasks/general', 'task queue file (absolute system path on server)');
INSERT INTO configuration VALUES('data_directory', '/opt/tissuestack/data', 'data directory (absolute system path on server)');
INSERT INTO configuration VALUES('ands_dataset_xml', '/opt/tissuestack/ands/datasets.xml', 'ands data set xml');
INSERT INTO configuration VALUES('max_upload_size', '10000000000', 'the maximum number of bytes allowed to upload in one go');
INSERT INTO configuration VALUES('default_drawing_interval', '250', 'default drawing interval');
INSERT INTO configuration VALUES('default_zoom_levels', '[0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5]', 'default zoom levels');
INSERT INTO configuration VALUES('color_maps', 
	'{"grey" : [[0, 0, 0, 0],[1, 1, 1, 1]],
	"hot" 		: [[0, 0, 0, 0],
	      		   [0.25, 0.5, 0, 0],
   				   [0.5, 1, 0.5, 0],
				   [0.75, 1, 1, 0.5],
				   [1, 1, 1, 1]],
	"spectral"	: [[0, 0, 0, 0],
	      		   [0.05, 0.46667, 0, 0.05333],
	      		   [0.1, 0.5333, 0, 0.6],
	      		   [0.15, 0, 0, 0.6667],
	      		   [0.2, 0, 0, 0.8667],
	      		   [0.25, 0, 0.4667, 0.8667],
	      		   [0.3, 0, 0.6, 0.8667],
	      		   [0.35, 0, 0.6667, 0.6667],
				   [0.4, 0, 0.6667, 0.5333],
				   [0.45, 0, 0.6, 0],
				   [0.5, 0, 0.7333, 0],
				   [0.55, 0, 0.8667, 0],
				   [0.6, 0, 1, 0],
				   [0.65, 0.7333, 1, 0],
				   [0.7, 0.9333, 0.9333, 0],
				   [0.75, 1, 0.8, 0],
				   [0.8, 1, 0.6, 0],
				   [0.85, 1, 0, 0],
				   [0.9, 0.8667, 0, 0],
				   [0.95, 0.8, 0, 0],
				   [1, 0.8, 0.8, 0.8]]
		}', 'default color mapping: grey, hot and spectral');
