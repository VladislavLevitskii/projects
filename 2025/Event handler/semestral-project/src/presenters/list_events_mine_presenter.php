<?php

require_once __DIR__ . '/../views/list_events_mine_view.php';
require_once __DIR__ . '/../models/registration_model.php';

class ListEventsMinePresenter {
    public $model;
    public $view;

    public function __construct() {
        $this->model = new RegistrationModel();
        $this->view = new ListEventsMineView();
    }

    public function render() {
        if (isset($_SESSION['id'])) {
            try {
                $data = $this->model->getRegistrationsOfUser($_SESSION['id']);
                $this->view->render($data);
            } catch (mysqli_sql_exception $e) {
                http_response_code(500);
                require_once __DIR__ . "/error_presenter.php";
                $presenter = new ErrorPresenter();
                $presenter->render("Something wrong with the database, repeat later");
                exit;
            }
        } else {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("You are not registered");
        }
        
        $this->model->connection->close();
    }

    public function updateRegistration($data) {
        try {
            $this->model->updateRegistration($data, $_SESSION['id']);
            $this->render();
        } catch (mysqli_sql_exception $e) {
            http_response_code(500);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Something wrong with the database, repeat later");
            exit;
        }
    }

}
