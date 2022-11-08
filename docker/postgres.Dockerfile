FROM postgres:8.4.22

COPY ./src/sql /sql
COPY ./docker/init-db.sh /sql/init-db.sh
