<?php

require_once __DIR__ . '/../views/event_register_view.php';
require_once __DIR__ . '/../models/event_register_model.php';

class EventRegisterPresenter {
    public $model;
    public $view;

    public function __construct() {
        $this->model = new EventRegisterModel();
        $this->view = new EventRegisterView();
    }

    public function render($id) {
        $data = $this->model->getData($id);
        if ($data !== NULL && isset($_SESSION['id']) && $data['name_organizer'] !== $_SESSION['user_name']) {
            $this->view->render($data);
        } else {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("event doesnt exist");
            exit;
        }
        
        $this->model->connection->close();
    }

    public function registerEvent($postData, $id) {
        $result = $this->model->registerEvent($postData, $id, $_SESSION['id']);
        if ($result !== true) {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render($result);
            exit;
        }
    }
}