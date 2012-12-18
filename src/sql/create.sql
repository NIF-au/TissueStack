-- USE OUR OWN USER
CREATE ROLE tissuestack LOGIN PASSWORD 'tissuestack';

-- CREATE DB INSTANCE 
CREATE DATABASE tissuestack OWNER tissuestack;

-- !!!!! SWITCH TO tissuestack NOW before executing the following lines !!!!!

-- GENERAL INSTANCE CONFIGURATION
CREATE TABLE configuration
(
  name VARCHAR(25) NOT NULL,
  "value" TEXT NOT NULL,
  description TEXT,
  CONSTRAINT configuration_pk PRIMARY KEY (name),
  CONSTRAINT configuration_unique UNIQUE (name, "value")
);
ALTER TABLE configuration OWNER TO tissuestack;

-- GENERAL INSTANCE CONFIGURATION - some default values
INSERT INTO configuration VALUES('version', '0.9', 'version');
INSERT INTO configuration VALUES('image_service_proxy_path', 'image_service', 'image service proxy path (relative to the application''s web root directory)');
INSERT INTO configuration VALUES('tile_directory', 'tiles', 'tile directory (relative to the application''s web root directory)');
INSERT INTO configuration VALUES('server_tile_directory', '/opt/tissuestack/tiles', 'server side tile directory for pre-tiling');
INSERT INTO configuration VALUES('upload_directory', '/opt/tissuestack/upload', 'upload directory (absolute system path on server)');
INSERT INTO configuration VALUES('lookup_tables_directory', '/opt/tissuestack/lookup', 'directory that houses the lookup table files (absolute system path on server)');
INSERT INTO configuration VALUES('colormaps_directory', '/opt/tissuestack/colormaps', 'directory that houses the color map files (absolute system path on server)');
INSERT INTO configuration VALUES('tasks_queue_file', '/opt/tissuestack/tasks/general', 'task queue file (absolute system path on server)');
INSERT INTO configuration VALUES('data_directory', '/opt/tissuestack/data', 'data directory (absolute system path on server)');
INSERT INTO configuration VALUES('ands_dataset_xml', '/opt/tissuestack/ands/datasets.xml', 'ands data set xml');
INSERT INTO configuration VALUES('max_upload_size', '1073741824', 'the maximum number of bytes allowed to upload in one go. A gig by default');
INSERT INTO configuration VALUES('default_drawing_interval', '250', 'default drawing interval');
INSERT INTO configuration VALUES('default_zoom_levels', '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75]', 'default zoom levels');
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

-- DATASET LIST
CREATE TABLE dataset
(
  id bigserial NOT NULL,
  filename VARCHAR(250) NOT NULL,
  description TEXT,
  lookup_id bigint REFERENCES dataset_values_lookup (id),
  CONSTRAINT dataset_pk PRIMARY KEY (id),
  CONSTRAINT dataset_lookup_fk FOREIGN KEY (lookup_id) REFERENCES dataset (id),
  CONSTRAINT dataset_filename_unique UNIQUE (filename)
);
ALTER TABLE dataset OWNER TO tissuestack;

CREATE INDEX idx_dataset_description
  ON dataset
  USING btree
  (description);
  
-- DATASET PLANE INFORMATION
CREATE TABLE dataset_planes
(
  id bigserial NOT NULL,
  dataset_id bigint NOT NULL,
  is_tiled CHAR(1) NOT NULL DEFAULT 'T',
  name CHAR(1) NOT NULL,
  max_x INTEGER NOT NULL,
  max_y INTEGER NOT NULL,
  max_slices INTEGER NOT NULL,
  zoom_levels TEXT NOT NULL,
  one_to_one_zoom_level INTEGER NOT NULL,
  transformation_matrix TEXT,
  resolution_mm NUMERIC(18,10),
  value_range_min NUMERIC(18,5) DEFAULT 0,
  value_range_max NUMERIC(18,5) DEFAULT 255,
  CONSTRAINT dataset_planes_pk PRIMARY KEY (id),
  CONSTRAINT dataset_planes_fk FOREIGN KEY (dataset_id)
      REFERENCES dataset (id) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT dataset_planes_unique UNIQUE (dataset_id, name)
);
ALTER TABLE dataset_planes OWNER TO tissuestack;

-- GENERAL DATASET OVERLAY INFO (SHARED FOR VARIOUS MANIFESTATIONS OF OVERLAY - see below)
CREATE TABLE dataset_overlays
(
  id bigserial NOT NULL,
  dataset_id bigint NOT NULL,
  dataset_planes_id bigint NOT NULL,
  slice integer NOT NULL,
  name VARCHAR(150),
  type VARCHAR(10) NOT NULL DEFAULT 'CANVAS'::character varying,
  CONSTRAINT dataset_overlays_pk PRIMARY KEY (id ),
  CONSTRAINT dataset_overlays_fk1 FOREIGN KEY (dataset_id)
      REFERENCES dataset (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT dataset_overlays_fk2 FOREIGN KEY (dataset_planes_id)
      REFERENCES dataset_planes (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT dataset_overlays_unique UNIQUE (dataset_id , dataset_planes_id , slice )
);
ALTER TABLE dataset_overlays OWNER TO tissuestack;

-- CUSTOM CANVAS OVERLAY (stores info in internal format that the frontend can turn into canvas drawings)
 CREATE TABLE dataset_canvas_overlay
(
  id bigint NOT NULL,
  content text,
  CONSTRAINT dataset_canvas_overlay_pk PRIMARY KEY (id ),
  CONSTRAINT dataset_canvas_overlay_fk FOREIGN KEY (id)
      REFERENCES dataset_overlays (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
);
ALTER TABLE dataset_canvas_overlay OWNER TO tissuestack;

-- SVG OVERLAY (entire SVG is stored)
CREATE TABLE dataset_svg_overlay
(
  id bigint NOT NULL,
  content text,
  CONSTRAINT dataset_svg_overlay_pk PRIMARY KEY (id ),
  CONSTRAINT dataset_svg_overlay_fk FOREIGN KEY (id)
      REFERENCES dataset_overlays (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
);
ALTER TABLE dataset_svg_overlay OWNER TO tissuestack;

-- SEARCH INDICES
CREATE INDEX dataset_overlays_idx1 ON dataset_overlays USING btree (dataset_id);
CREATE INDEX dataset_overlays_idx2 ON dataset_overlays USING btree (dataset_id, type);
CREATE INDEX dataset_overlays_idx3 ON dataset_overlays USING btree (dataset_id, dataset_planes_id, slice);

-- DATASET VALUES LOOKUP TABLE 
CREATE TABLE dataset_values_lookup
(
  id bigserial NOT NULL,
  filename VARCHAR(250) NOT NULL,
  content TEXT,
  CONSTRAINT dataset_values_lookup_pk PRIMARY KEY (id),
  CONSTRAINT dataset_values_lookup_unique UNIQUE (filename)
);
ALTER TABLE dataset_values_lookup OWNER TO tissuestack;

CREATE INDEX idx_dataset_values_lookup_filename
  ON dataset_values_lookup
  USING btree
  (filename);

-- INSERT SOME TEST DATA
--INSERT INTO dataset VALUES (1, '/opt/data/00-normal-model-nonsym-tiled.mnc', 'Tiled Version');
--INSERT INTO dataset_planes VALUES (1, 1, 'Y', 'x', 1311, 499, 678, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -327.15 ],[0   , 0.5, 0   , -124.2   ],[0   , 0   , 0.5, -169.2   ],[0   , 0   , 0   ,  1 ]]');
--INSERT INTO dataset_planes VALUES (2, 1, 'Y', 'y', 679, 499, 1310, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -169.2   ],[0   , 0.5, 0   , -124.2   ],[0   , 0   , 0.5, -327.15 ],[0   , 0   , 0   ,  1  ]]');
--INSERT INTO dataset_planes VALUES (3, 1, 'Y', 'z', 679, 1311, 498, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -169.2   ],[0   , 0.5, 0   , -327.15 ],[0   , 0   , 0.5, -124.2  ],[0   , 0   , 0   ,  1 ]]');
--INSERT INTO dataset VALUES (2, '/opt/data/00-normal-model-nonsym.mnc', 'Image Service Version');
--INSERT INTO dataset_planes VALUES (4, 2, 'N', 'x', 1311, 499, 678, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -327.15 ],[0   , 0.5, 0   , -124.2   ],[0   , 0   , 0.5, -169.2   ],[0   , 0   , 0   ,  1 ]]');
--INSERT INTO dataset_planes VALUES (5, 2, 'N', 'y', 679, 499, 1310, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -169.2   ],[0   , 0.5, 0   , -124.2   ],[0   , 0   , 0.5, -327.15 ],[0   , 0   , 0   ,  1  ]]');
--INSERT INTO dataset_planes VALUES (6, 2, 'N', 'z', 679, 1311, 498, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -169.2   ],[0   , 0.5, 0   , -327.15 ],[0   , 0   , 0.5, -124.2  ],[0   , 0   , 0   ,  1 ]]');

-- SESSION 
CREATE TABLE session
(
  id VARCHAR(100) NOT NULL,
  expiry bigint NOT NULL,
  CONSTRAINT session_pk PRIMARY KEY (id)
);
ALTER TABLE session OWNER TO tissuestack;
