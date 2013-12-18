-- update version 
UPDATE configuration SET value='1.3' WHERE name='version';
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