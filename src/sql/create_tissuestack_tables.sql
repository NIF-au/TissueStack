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
