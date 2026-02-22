<?php

require_once __DIR__ . '/../views/settings_view.php';
require_once __DIR__ . '/../models/user_model.php';

class SettingsPresenter {

    public $model;
    public $view;

    public function __construct() {
        $this->model = new UserModel();
        $this->view = new SettingsView();
    }

    public function render($result = 0) {
        if (!isset($_SESSION['id'])) {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("You are not registered");
            exit;
        } else {
            $data = $this->model->getData($_SESSION['id']);
            $this->view->render($data, $result);
        }
        
        $this->model->connection->close();
    }

    private function databaseError() {
        http_response_code(500);
        require_once __DIR__ . "/error_presenter.php";
        $presenter = new ErrorPresenter();
        $presenter->render("Something wrong with the database, repeat later");
        exit;
    }

    public function changeSettings($data) {
        try {
            $this->model->changeSettings($data, $_SESSION['id']);
            $this->render(1);
        } catch (mysqli_sql_exception $e) {
            $this->databaseError();
        }
    }

    public function deleteAccount($id) {
        try {
            $this->model->deleteAccount($id);
            session_destroy();
            $this->render(2);
        } catch (mysqli_sql_exception $e) {
            $this->databaseError();
        }
    }
}
