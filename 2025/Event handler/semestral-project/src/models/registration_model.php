<?php

require_once __DIR__ . "/db_init.php";

class RegistrationModel {
    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function getDataByID($id) {
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

    public function getRegistrationsOfUser($id) {
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

    public function registerEvent($postData, $id_event, $id_user, $referralCode) {
        try {
            $workshops = isset($postData['workshops']) ? $postData['workshops'] : [];
            $data = implode(" ", $workshops);

            $query = "INSERT INTO registrations (event_id, user_id, user_workshops, referral_link_id) VALUES (?, ?, ?, ?)";
            $stmt = $this->connection->prepare($query);
            $stmt->bind_param("iisi", $id_event, $id_user, $data, $referralCode);
            return $stmt->execute();
        } catch (mysqli_sql_exception $e) {
            if ($e->getCode() == 1062) { // duplicity
                return "You are already registered for this event";
            }
            throw $e;
        }
    }
}