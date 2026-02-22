<?php

require_once __DIR__ . "/default_model.php";

class EventModel {

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

    public function deleteEvent($id) {
        $query = "DELETE FROM events WHERE id = ?";
        
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $id);
        
        return $stmt->execute();
    }
}