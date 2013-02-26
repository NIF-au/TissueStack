-- CREATE OUR OWN USER
CREATE ROLE tissuestack LOGIN PASSWORD 'tissuestack';

-- CREATE DB INSTANCE 
CREATE DATABASE tissuestack OWNER tissuestack;
