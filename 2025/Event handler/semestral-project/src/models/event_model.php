<?php

require_once __DIR__ . "/db_init.php";

class EventModel {

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

    public function getDataForLandingPage() {
        $query = "
        SELECT events.*, users.username AS name_organizer
        FROM events
        JOIN users ON events.organizer_id = users.id
        WHERE start_date >= NOW()
        ORDER BY start_date DESC
        LIMIT 3;
        ";
        $stmt = $this->connection->prepare($query);
        $stmt->execute();
        $result = $stmt->get_result();
        return $result;
    }

    public function deleteEvent($id) {
        $query = "DELETE FROM events WHERE id = ?";
        
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $id);
        
        return $stmt->execute();
    }

    public function getAllData() {
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

    public function createEvent($data, $image) {
        $imageName = null;
        $hash = time();
        if (isset($image['hero_image_path']) && $image['hero_image_path']['error'] === 0) {
            $imageName = $this->uploadImage($image['hero_image_path'], $hash);
        }
        $this->insertEvent($data, $imageName);
    }

    public function uploadImage($file, $hash) {
        if ($file['error'] !== UPLOAD_ERR_OK) {
            return null;
        }

        $targetDir = __DIR__ . '/../../public/data/';

        $ext = strtolower(pathinfo($file['name'], PATHINFO_EXTENSION));
        $allowed = ['jpg', 'jpeg', 'png', 'webp'];
        if (!in_array($ext, $allowed)) {
            return "Tento typ souboru není povolen.";
        }
        $newFileName = $_SESSION['id'] . $hash . '.' . $ext;

        if (move_uploaded_file($file['tmp_name'], $targetDir . $newFileName)) {
            return $newFileName;
        }

        return null;
    }

    public function deleteImage($id) {
        $query = "
        SELECT hero_image_path 
        FROM events
        WHERE id = ?";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $result = $stmt->get_result();
        
        if ($row = $result->fetch_assoc()) {
            $filePath = __DIR__ . "/../../public/data/" . $row['hero_image_path'];
            if (file_exists($filePath) && $row['hero_image_path'] !== NULL) {
                unlink($filePath);
            }
        }
    }

    public function insertEvent($data, $imageName) {
        $query = "INSERT INTO events (name, description, start_date, end_date, hero_image_path, workshops, organizer_id) VALUES (?, ?, ?, ?, ?, ?, ?)";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("ssssssi", 
            $data['name'],
            $data['description'], 
            $data['start_date'], 
            $data['end_date'],
            $imageName,
            $data['workshops'], 
            $_SESSION['id']);
        $stmt->execute();
    }

    public function changeEvent($data, $image) {
        $imageName = null;
        $hash = time();
        if (isset($image['hero_image_path']) && $image['hero_image_path']['error'] === 0) {
            $imageName = $this->uploadImage($image['hero_image_path'], $hash);
        }
        $this->deleteImage($data['id']);
        $this->updateEvent($data, $imageName);
    }

    public function updateEvent($data, $imageName) {
        $query = "UPDATE events 
          SET name = ?, 
              description = ?, 
              start_date = ?, 
              end_date = ?, 
              hero_image_path = ?,
              workshops = ? 
          WHERE organizer_id = ? AND id = ?";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("ssssssii", 
            $data['name'], 
            $data['description'], 
            $data['start_date'], 
            $data['end_date'], 
            $imageName, 
            $data['workshops'], 
            $_SESSION['id'], 
            $data['id']
        );
        $stmt->execute();
    }

    public function createReferalLink($referalName, $userId, $idEvent) {
        $query = "INSERT INTO referral_links (event_id, creator_id, identifier) VALUES (?, ?, ?)";
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("iis", $idEvent, $userId, $referalName);
        $stmt->execute();
    }

    public function getIDReferralLink($referralName, $eventID) {
        $query = "
        SELECT id
        FROM referral_links
        WHERE identifier = ? AND event_id = ?
        ";
        
        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("si", $referralName, $eventID);
        $stmt->execute();
        
        $result = $stmt->get_result();
        $row = $result->fetch_assoc();
    
        if ($row) {
            return $row['id'];
        } else {
            return null;
        }
    }

    public function getReferralStats($eventId) {
        $query = "
            SELECT rl.identifier, COUNT(r.user_id) as usage_count
            FROM referral_links rl
            JOIN registrations r ON rl.id = r.referral_link_id
            WHERE rl.event_id = ?
            GROUP BY rl.id, rl.identifier
            ORDER BY usage_count DESC
        ";

        $stmt = $this->connection->prepare($query);
        $stmt->bind_param("i", $eventId);
        $stmt->execute();
        
        $result = $stmt->get_result();
        return $result->fetch_all(MYSQLI_ASSOC);
}

}