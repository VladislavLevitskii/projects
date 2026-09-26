IF NOT EXISTS (SELECT * FROM sys.databases WHERE name = 'WebCrawlerDB')
BEGIN
    CREATE DATABASE WebCrawlerDB
END
GO