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
INSERT INTO configuration VALUES('version', '0.4', 'version');
INSERT INTO configuration VALUES('tiles', 'TRUE', 'supports tiled datasets (TRUE/FALSE)');
INSERT INTO configuration VALUES('images', 'FALSE', 'supports querying image files directly (TRUE/FALSE)');
INSERT INTO configuration VALUES('tile_directory', 'tiles/', 'tile directory (relative to the application''s apache web server)');
INSERT INTO configuration VALUES('default_drawing_interval', '150', 'default drawing interval');
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
  CONSTRAINT dataset_pk PRIMARY KEY (id),
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
  name CHAR(1) NOT NULL,
  max_x INTEGER NOT NULL,
  max_y INTEGER NOT NULL,
  max_sclices INTEGER NOT NULL,
  zoom_levels TEXT NOT NULL,
  one_to_one_zoom_level INTEGER NOT NULL,
  transformation_matrix TEXT,
  CONSTRAINT dataset_planes_pk PRIMARY KEY (id),
  CONSTRAINT dataset_planes_fk FOREIGN KEY (dataset_id)
      REFERENCES dataset (id),
  CONSTRAINT dataset_planes_unique UNIQUE (dataset_id, name)
);
ALTER TABLE dataset_planes OWNER TO tissuestack;

-- INSERT SOME TEST DATA
INSERT INTO dataset VALUES (1, '/opt/data/00-normal-model-nonsym.mnc', 'This is our prototype mouse for development');
INSERT INTO dataset_planes VALUES (1, 1, 'x', 1311, 499, 678, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -327.15 ],[0   , 0.5, 0   , -124.2   ],[0   , 0   , 0.5, -169.2   ],[0   , 0   , 0   ,  1 ]]');
INSERT INTO dataset_planes VALUES (2, 1, 'y', 679, 499, 1310, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -169.2   ],[0   , 0.5, 0   , -124.2   ],[0   , 0   , 0.5, -327.15 ],[0   , 0   , 0   ,  1  ]]');
INSERT INTO dataset_planes VALUES (3, 1, 'z', 679, 1311, 498, '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75 ]', 3, '[[0.5, 0   , 0   , -169.2   ],[0   , 0.5, 0   , -327.15 ],[0   , 0   , 0.5, -124.2  ],[0   , 0   , 0   ,  1 ]]');


