<?php

require_once __DIR__ . "/default_model.php";

class NewEventModel {

    public $connection;

    public function __construct() {
        $this->connection = connect_and_create();
    }

    public function createEvent($data, $image) {
        $imageName = null;
        $hash = time();
        if (isset($image['hero_image_path']) && $image['hero_image_path']['error'] === 0) {
            $imageName = $this->uploadImage($image['hero_image_path'], $hash);
        }
        $this->insertEvent($data, $imageName);
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

    private function insertEvent($data, $imageName) {
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
}