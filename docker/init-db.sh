#!/bin/bash
set -e

psql -U postgres -a -f create_tissuestack_db.sql
psql -U tissuestack -d tissuestack -a -f create_tissuestack_tables.sql
psql -U tissuestack -d tissuestack -a -f create_tissuestack_config.sql
# psql -U tissuestack -d tissuestack -a -f patches.sql
