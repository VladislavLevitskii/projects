<?php

require_once __DIR__ . '/../views/event_edit_view.php';
require_once __DIR__ . '/../models/event_edit_model.php';

class EventEditPresenter {

    public $model;
    public $view;

    public function __construct() {
        $this->model = new EventEditModel();
        $this->view = new EventEditView();
    }

    public function render($id, $edited = 0) {
        $data = $this->model->getData($id);
        if (!isset($_SESSION['id']) || $data === NULL || $data['organizer_id'] !== $_SESSION['id']) {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("You are not registered to edit events");
            exit;
        } else {
            $this->view->render($data, $edited);
        }
        
        $this->model->connection->close();
    }

    public function changeEvent($data, $image) {
        try {
            $this->model->changeEvent($data, $image);
            $this->render($data['id'], 1);
        } catch (mysqli_sql_exception $e) {
            http_response_code(500);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Something wrong with the database, repeat later");
            exit;
        }
    }
}
