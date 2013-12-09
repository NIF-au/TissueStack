-- update version 
UPDATE configuration SET value='1.3' WHERE name='version';
-- remove wrong constraints
ALTER TABLE dataset DROP CONSTRAINT dataset_lookup_id_fkey;
ALTER TABLE dataset DROP CONSTRAINT dataset_lookup_fk;
-- add proper constraints
ALTER TABLE dataset ADD CONSTRAINT dataset_lookup_fk FOREIGN KEY (lookup_id) REFERENCES dataset_values_lookup (id);
-- be more generous with uploads
UPDATE configuration SET value='107374182400',description='the maximum number of bytes allowed to upload in one go: 100 gig by default' WHERE name='max_upload_size';