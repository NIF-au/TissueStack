# docker compose

## Create .env file

Copy default.env into .env and update parameters
```
cd docker
cp default.env .env
# Modify .env here
```

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
docker compose exec tissuestack-db /bin/bash
cd /sql
bash init-db.sh
```

Restart docker containers 

## Initialise tissuestack data
Copy src/web into ${TISSUESTACK_DATA_DIR}
