<?php

require_once __DIR__ . "/default_model.php";

class ListEventsModel {

    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function getData() {
        $query = "
        SELECT events.*, users.username AS name_organizer
        FROM events
        JOIN users ON events.organizer_id = users.id
        ORDER BY start_date DESC
        ";
        $stmt = $this->connection->prepare($query);
        $stmt->execute();
        $result = $stmt->get_result();
        return $result;
    }
}