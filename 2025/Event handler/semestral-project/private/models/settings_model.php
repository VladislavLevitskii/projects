<?php

require_once __DIR__ . "/default_model.php";

class SettingsModel {

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

}