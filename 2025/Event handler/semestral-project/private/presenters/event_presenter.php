<?php

require_once __DIR__ . '/../views/event_view.php';
require_once __DIR__ . '/../models/event_model.php';

class EventPresenter {

    public $model;
    public $view;

    public function __construct() {
        $this->model = new EventModel();
        $this->view = new EventView();
    }

    public function render($id) {

        try {
            $data = $this->model->getData($id);
        } catch (mysqli_sql_exception $e) {
            http_response_code(500);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Something wrong with the database, repeat later");
            exit;
        }
        if ($data !== NULL) {
            $this->view = new EventView();
            $this->view->render($data);
        } else {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Event doesn't exist");
            exit;
        }
        
        $this->model->connection->close();
    }

    public function deleteEvent($id) {
        $this->model->deleteEvent($id);
    }
}
