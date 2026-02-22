<?php

require_once __DIR__ . "/default_model.php";

class LoginModel {

    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function checkExistence($username, $email) {
        $query = "SELECT id FROM users WHERE username = ? AND email = ? LIMIT 1";
        
        $stmt = $this->connection->prepare($query);
        
        $stmt->bind_param("ss", $username, $email);
        $stmt->execute();
        
        $result = $stmt->get_result();
        
        if ($row = $result->fetch_assoc()) {
            return $row['id'];
        }

        return false;
    }
}