-- update version 
UPDATE configuration SET value='1.8' WHERE name='version';
-- remove wrong constraints
ALTER TABLE dataset DROP CONSTRAINT dataset_lookup_id_fkey;
ALTER TABLE dataset DROP CONSTRAINT dataset_lookup_fk;
-- add new tables 
-- atlas info
CREATE TABLE atlas_info
(
	id bigserial NOT NULL,
	atlas_prefix VARCHAR(50) NOT NULL,
	atlas_description VARCHAR(250) NOT NULL,
	atlas_query_url VARCHAR(250),
	CONSTRAINT atlas_info_pk PRIMARY KEY (id)
);
ALTER TABLE  atlas_info OWNER TO tissuestack;
-- lookup mapping
CREATE TABLE dataset_lookup_mapping
(
	dataset_id bigint NOT NULL,
	associated_dataset_id bigint NOT NULL,
	CONSTRAINT dataset_lookup_mapping_pk PRIMARY KEY (dataset_id, associated_dataset_id),
	CONSTRAINT dataset_lookup_mapping_fk1 FOREIGN KEY (dataset_id)
		REFERENCES dataset (id) ON DELETE CASCADE ON UPDATE CASCADE,
	CONSTRAINT dataset_lookup_mapping_fk2 FOREIGN KEY (associated_dataset_id)
		REFERENCES dataset (id) ON DELETE CASCADE ON UPDATE CASCADE
);
ALTER TABLE  dataset_lookup_mapping OWNER TO tissuestack;
-- add new columns 
ALTER TABLE dataset_values_lookup ADD COLUMN atlas_association bigint;
-- add proper constraints
ALTER TABLE dataset ADD CONSTRAINT dataset_lookup_fk FOREIGN KEY (lookup_id) REFERENCES dataset_values_lookup (id);
ALTER TABLE dataset_values_lookup ADD CONSTRAINT dataset_values_lookup_fk FOREIGN KEY (atlas_association) REFERENCES atlas_info (id);
-- be more generous with uploads
UPDATE configuration SET value='107374182400',description='the maximum number of bytes allowed to upload in one go: 100 gig by default' WHERE name='max_upload_size';
-- constraint to disallow self references 
ALTER TABLE dataset_lookup_mapping ADD CONSTRAINT dataset_lookup_mapping_no_self_references CHECK(dataset_id <> associated_dataset_id);
-- this will enable raw file querying with less data base lookup since most info is already on the server side in memory
-- these parameters while on a dimension level are really the same for all planes in practice,
-- hence redundant and belonging to the data set as a whole
ALTER TABLE dataset ADD COLUMN is_tiled CHAR(1) NOT NULL DEFAULT 'F';
ALTER TABLE dataset ADD COLUMN zoom_levels TEXT NOT NULL DEFAULT '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2.00, 2.25, 2.5]';
ALTER TABLE dataset ADD COLUMN one_to_one_zoom_level INTEGER NOT NULL DEFAULT 3;
ALTER TABLE dataset ADD COLUMN resolution_mm NUMERIC(18,10) DEFAULT 0;
UPDATE configuration SET value='server',description='server proxy path (relative to the application''s web root directory)' WHERE name='server_proxy_path';
UPDATE configuration SET value='[0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5]',description='default zoom levels' WHERE name='default_zoom_levels';
ALTER TABLE dataset ADD COLUMN value_range_min numeric(18,5) NOT NULL DEFAULT 0;
ALTER TABLE dataset ADD COLUMN value_range_max numeric(18,5) NOT NULL DEFAULT 255;
ALTER TABLE dataset_planes ADD COLUMN step NUMERIC(18,10) NOT NULL DEFAULT 1;
-- copy resolutions if there 
--UPDATE dataset SET resolution_mm = res
--FROM (SELECT dataset_id, min(resolution_mm) AS res FROM dataset_planes group by dataset_id) AS sub_planes
--WHERE dataset.id = sub_planes.dataset_id
--ALTER TABLE dataset_planes DROP COLUMN resolution_mm;