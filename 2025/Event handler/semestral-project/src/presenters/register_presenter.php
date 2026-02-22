<?php

require_once __DIR__ . '/../views/register_view.php';
require_once __DIR__ . '/../models/user_model.php';

class RegisterPresenter {
    public $model;
    public $view;

    public function __construct() {
        $this->model = new UserModel();
        $this->view = new RegisterView();
    }

    public function render($error) {
        $this->view->render($error);
        $this->model->connection->close();
    }

    public function registerUser($postData) {
        if (isset($postData['username']) && isset($postData['email'])) {
            $result = $this->model->insertNewUser($postData['username'], $postData['email']);
            if (is_int($result)) {
                session_destroy();
                session_start();
                $_SESSION['user_name'] = $postData['username'];
                $_SESSION['email'] = $postData['email'];
                $_SESSION['id'] = $result;
                $this->view->render(1);
            } else {
                $this->view->render($result);
            }
        }
    }
}
