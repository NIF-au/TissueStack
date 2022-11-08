# docker compose

## Create .env file

Copy default.env into .env and update parameters

## docker compose build

```
docker compose down
docker compose build
docker compose up -d
```

## Initialise database (new database only)

Wait for `container for service "tissuestack-db" is unhealthy`

Shell into tissuestack-db container
```
cd /sql
bash init-db.sh
```

Restart docker containers 

## Initialise tissuestack data
Copy src/web into ${TISSUESTACK_DATA_DIR}
