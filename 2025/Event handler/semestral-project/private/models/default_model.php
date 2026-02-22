<?php

function connect_and_create() {
    
    $connection = mysqli_connect(DB_HOST, DB_USER, DB_PASS, DB_NAME);

    if ($connection->connect_error) {
        die("Connection error: " . $connection->connect_error);
    }

    $queryUsers = "
    CREATE TABLE IF NOT EXISTS users (
        id INT AUTO_INCREMENT PRIMARY KEY,
        username VARCHAR(50) NOT NULL UNIQUE,
        email VARCHAR(255) NOT NULL UNIQUE
    );";
    $connection->query($queryUsers);

    $queryEvents = "
    CREATE TABLE IF NOT EXISTS events (
        id INT AUTO_INCREMENT PRIMARY KEY,
        name VARCHAR(64) NOT NULL,
        description VARCHAR(1024),
        start_date DATETIME NOT NULL,
        end_date DATETIME NOT NULL,
        hero_image_path VARCHAR(255) UNIQUE,
        workshops TEXT,
        
        organizer_id INT NOT NULL,
        
        FOREIGN KEY (organizer_id) REFERENCES users(id) ON DELETE CASCADE
    );";
    $connection->query($queryEvents);

    $queryRegistrations = "
    CREATE TABLE IF NOT EXISTS registrations (
        event_id INT NOT NULL,
        user_id INT NOT NULL,
        user_workshops VARCHAR(1024),
        PRIMARY KEY (event_id, user_id),
        
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
        FOREIGN KEY (event_id) REFERENCES events(id) ON DELETE CASCADE
    );";

    $connection->query($queryRegistrations);

    return $connection;
}

?>