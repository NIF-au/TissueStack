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

-- ATLAS INFO
CREATE TABLE atlas_info
(
	id bigserial NOT NULL,
	atlas_prefix VARCHAR(50) NOT NULL,
	atlas_description VARCHAR(250) NOT NULL,
	atlas_query_url VARCHAR(250),
	CONSTRAINT atlas_info_pk PRIMARY KEY (id)
);
ALTER TABLE  atlas_info OWNER TO tissuestack;

-- DATASET VALUES LOOKUP TABLE 
CREATE TABLE dataset_values_lookup
(
  id bigserial NOT NULL,
  filename VARCHAR(250) NOT NULL,
  content TEXT,
  atlas_association bigint,
  CONSTRAINT dataset_values_lookup_pk PRIMARY KEY (id),
  CONSTRAINT dataset_values_lookup_fk FOREIGN KEY (atlas_association) REFERENCES atlas_info (id),
  CONSTRAINT dataset_values_lookup_unique UNIQUE (filename)
);
ALTER TABLE dataset_values_lookup OWNER TO tissuestack;

CREATE INDEX idx_dataset_values_lookup_filename
  ON dataset_values_lookup
  USING btree
  (filename);

-- DATASET LIST
CREATE TABLE dataset
(
  id bigserial NOT NULL,
  filename VARCHAR(250) NOT NULL,
  description TEXT,
  lookup_id bigint,
  is_tiled CHAR(1) NOT NULL DEFAULT 'F',
  zoom_levels TEXT NOT NULL DEFAULT '[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2.00, 2.25, 2.5]',
  one_to_one_zoom_level INTEGER NOT NULL DEFAULT 3,
  value_range_min NUMERIC(18,5) NOT NULL DEFAULT 0,
  value_range_max NUMERIC(18,5) NOT NULL DEFAULT 255,
  resolution_mm NUMERIC(18,10) DEFAULT 0,
  CONSTRAINT dataset_pk PRIMARY KEY (id),
  CONSTRAINT dataset_lookup_fk FOREIGN KEY (lookup_id) REFERENCES dataset_values_lookup (id),
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
  max_slices INTEGER NOT NULL,
  step NUMERIC(18,10) NOT NULL DEFAULT 1,
  transformation_matrix TEXT,
  CONSTRAINT dataset_planes_pk PRIMARY KEY (id),
  CONSTRAINT dataset_planes_fk FOREIGN KEY (dataset_id)
      REFERENCES dataset (id) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT dataset_planes_unique UNIQUE (dataset_id, name)
);
ALTER TABLE dataset_planes OWNER TO tissuestack;

-- DATA SET LOOKUP MAPPING
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
  CONSTRAINT dataset_overlays_unique UNIQUE (dataset_id , dataset_planes_id , slice, type )
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

-- DATASET OVERLAY (we link another data set plane via its unique data set and plane id)
CREATE TABLE dataset_other_overlay
(
  id bigint NOT NULL,
  linked_dataset_id bigint,
  linked_dataset_planes_id bigint,
  CONSTRAINT dataset_other_overlay_pk PRIMARY KEY (id ),
  CONSTRAINT dataset_other_overlay_fk1 FOREIGN KEY (id)
      REFERENCES dataset_overlays (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT dataset_other_overlay_fk2 FOREIGN KEY (linked_dataset_id)
    REFERENCES dataset (id) MATCH SIMPLE,
      CONSTRAINT dataset_other_overlay_fk3 FOREIGN KEY (linked_dataset_planes_id)
    REFERENCES dataset_planes (id) MATCH SIMPLE
);
ALTER TABLE dataset_other_overlay OWNER TO tissuestack;

-- SEARCH INDICES
CREATE INDEX dataset_overlays_idx1 ON dataset_overlays USING btree (dataset_id);
CREATE INDEX dataset_overlays_idx2 ON dataset_overlays USING btree (id, type);
CREATE INDEX dataset_overlays_idx3 ON dataset_overlays USING btree (dataset_id, dataset_planes_id, type);

-- SESSION 
CREATE TABLE session
(
  id VARCHAR(100) NOT NULL,
  expiry bigint NOT NULL,
  CONSTRAINT session_pk PRIMARY KEY (id)
);
ALTER TABLE session OWNER TO tissuestack;
