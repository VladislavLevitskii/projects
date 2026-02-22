<?php

require_once __DIR__ . "/default_model.php";

class EventEditModel {

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

    public function changeEvent($data, $image) {
        $imageName = null;
        $hash = time();
        if (isset($image['hero_image_path']) && $image['hero_image_path']['error'] === 0) {
            $imageName = $this->uploadImage($image['hero_image_path'], $hash);
        }
        $this->deleteImage($data['id']);
        $this->updateEvent($data, $imageName);
    }

    private function uploadImage($file, $hash) {
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

    private function updateEvent($data, $imageName) {
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

    private function deleteImage($id) {
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
}