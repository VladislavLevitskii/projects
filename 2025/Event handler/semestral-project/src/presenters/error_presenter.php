<?php

require_once __DIR__ . '/../views/error_view.php';

class ErrorPresenter {
    public function render($error_message) {

        $view = new ErrorView();
        $view->render($error_message);
    }
}