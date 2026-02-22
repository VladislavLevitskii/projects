<?php

require_once __DIR__ . '/../views/login_view.php';
require_once __DIR__ . '/../models/login_model.php';

class LoginPresenter {
    public $model;
    public $view;

    public function __construct() {
        $this->model = new LoginModel();
        $this->view = new LoginView();
    }

    public function render($error = 0) {
        $this->view->render($error);
        $this->model->connection->close();
    }

    public function loginUser($postData) {
        if (isset($postData['username']) && isset($postData['email'])) {
            $result = $this->model->checkExistence($postData['username'], $postData['email']);
            if ($result !== false) {
                session_start();
                $_SESSION['user_name'] = $postData['username'];
                $_SESSION['email'] = $postData['email'];
                $_SESSION['id'] = $result;
                header("Location: " . BASE_URL);
                exit;
            } else {
                $this->render(1);
            }
        } else {
            $this->model->connection->close();
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("post is something different");
            exit;
        }
    }
}