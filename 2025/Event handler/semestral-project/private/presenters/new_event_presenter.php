<?php

require_once __DIR__ . '/../views/new_event_view.php';
require_once __DIR__ . '/../models/new_event_model.php';

class NewEventPresenter {
    
    public $model;
    public $view;
    
    public function __construct() {
        $this->model = new NewEventModel();
        $this->view = new NewEventView();
    }
    
    public function render() {
        $this->view->render();
        $this->model->connection->close();
    }
    
    private function check_validation($data) {
        $errors = [];

        if (strlen($data['name']) < 1 || strlen($data['name']) > 64) {
            $errors[] = "name must be 1-64 characters";
        }

        if (strlen($data['description']) > 1024) {
            $errors[] = "description is too long (max 1024)";
        }

        if (empty($data['start_date'])) {
            $errors[] = "start date is required";
        }

        if (empty($data['end_date'])) {
            $errors[] = "end date is required";
        }

        try {
            if (!empty($data['start_date']) && !empty($data['end_date'])) {
                $start = new DateTime($data['start_date']);
                $end = new DateTime($data['end_date']);
                $today = new DateTime('today');

                if ($start <= $today) {
                    $errors[] = "start date must be in the future";
                }
                if ($end < $start) {
                    $errors[] = "end date cannot be before start date";
                }
            }
        } catch (Exception $e) {
            $errors[] = "invalid date format";
        }

        return $errors;
    }

    public function createEvent($data, $image) {
        $validationErrors = $this->check_validation($data);
        if (!empty($validationErrors)) {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            
            $presenter = new ErrorPresenter();
            $errorString = implode(", ", $validationErrors);
            
            $presenter->render($errorString);
            exit;
        }
        try {
            $this->model->createEvent($data, $image);
            header("Location: " . BASE_URL);
            exit;
        } catch (mysqli_sql_exception $e) {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render();
            exit;
        }
    }

}