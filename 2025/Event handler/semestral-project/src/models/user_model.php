<?php

require_once __DIR__ . "/db_init.php";

class UserModel {

    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function getData($id) {
        $query = "
        SELECT * FROM users WHERE id = ?
        ";
        
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $id);
        $stmt->execute();
        
        $result = $stmt->get_result();
        return $result->fetch_assoc();
    }

    public function changeSettings($data, $id) {
        $query = "UPDATE users 
          SET username = ?, 
              email = ?
          WHERE  id = ?";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("ssi", 
            $data['name'],
            $data['email'],
            $id
        );
        $stmt->execute();
        $_SESSION['user_name'] = $data['name'];
        $_SESSION['email'] = $data['email'];
    }

    public function deleteAccount($id) {
        $query = "DELETE FROM users WHERE id = ?";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $id);
        $stmt->execute();
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