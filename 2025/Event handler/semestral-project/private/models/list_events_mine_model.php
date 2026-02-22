<?php

require_once __DIR__ . "/default_model.php";

class ListEventsMineModel {

    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function getData($id) {
        $query = "
        SELECT * 
        FROM registrations r
        JOIN events e ON r.event_id = e.id
        WHERE r.user_id = ?
        ";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $result = $stmt->get_result();
        return $result->fetch_all(MYSQLI_ASSOC);
    }

    public function updateRegistration($data, $id_user) {
        if (empty($data['workshops'])) {
            $workshops = "";
        } else {
            $workshops = implode(" ", $data['workshops']);
        }
        $event_id = $data['id'];
        $query = "UPDATE registrations 
          SET user_workshops = ?
          WHERE event_id = ? AND user_id = ?";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("sii", 
            $workshops,
            $event_id,
            $id_user
        );
        $stmt->execute();
    }
}