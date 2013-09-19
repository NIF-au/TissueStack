-- remove wrong constraints
ALTER TABLE dataset DROP CONSTRAINT dataset_lookup_id_fkey;
ALTER TABLE dataset DROP CONSTRAINT dataset_lookup_fk;
-- add proper constraints
ALTER TABLE dataset ADD CONSTRAINT dataset_lookup_fk FOREIGN KEY (lookup_id) REFERENCES dataset_values_lookup (id);