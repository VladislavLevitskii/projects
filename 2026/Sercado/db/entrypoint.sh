#!/bin/bash
/opt/mssql/bin/sqlservr & 

echo "Waiting for SQL Server to start..."
sleep 20 

/opt/mssql-tools18/bin/sqlcmd -S localhost -U sa -P "$MSSQL_SA_PASSWORD" -C -i /usr/src/app/init.sql

wait