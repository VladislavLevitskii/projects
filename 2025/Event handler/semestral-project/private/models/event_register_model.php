<?php

require_once __DIR__ . "/default_model.php";

class EventRegisterModel {

    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function getData($id) {
        $query = "
        SELECT events.*, users.username AS name_organizer
        FROM events
        JOIN users ON events.organizer_id = users.id
        WHERE events.id = ?
        LIMIT 1
        ";
        
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $id);
        $stmt->execute();
        
        $result = $stmt->get_result();
        return $result->fetch_assoc();
    }

    public function registerEvent($postData, $id_event, $id_user) {
        try {
            $workshops = isset($postData['workshops']) ? $postData['workshops'] : [];
            $data = implode(" ", $workshops);

            $query = "INSERT INTO registrations (event_id, user_id, user_workshops) VALUES (?, ?, ?)";
            $stmt = $this->connection->prepare($query);
            $stmt->bind_param("iis", $id_event, $id_user, $data);
            
            return $stmt->execute();
        } catch (mysqli_sql_exception $e) {
            if ($e->getCode() == 1062) {
                return "You are already registered for this event";
            }
            throw $e;
        }
    }
}