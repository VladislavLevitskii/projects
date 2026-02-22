<?php

require_once __DIR__ . "/default_model.php";

class RegisterModel {

    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function insertNewUser($username, $email) {
        $query = "INSERT INTO users (username, email) VALUES (?, ?)";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("ss", $username, $email);
        
        try {
            $stmt->execute();
            return $stmt->insert_id; 
        } catch (mysqli_sql_exception $e) {
            if ($e->getCode() === 1062) {
                return "User or email already exists."; 
            }
            return "DB error: " . $e->getMessage();
        }
    }

}