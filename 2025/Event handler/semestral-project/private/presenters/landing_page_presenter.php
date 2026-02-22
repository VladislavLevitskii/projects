<?php

require_once __DIR__ . '/../views/landing_page_view.php';
require_once __DIR__ . '/../models/landing_page_model.php';

class LandingPagePresenter {
    public function render() {

        $model = new LandingPageModel();
        try {
            $data = $model->getData();
            $view = new LandingPageView();
            $view->render($data);
        } catch (mysqli_sql_exception $e) {
            $model->connection->close();
            http_response_code(500);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Something wrong with the database, repeat later");
            exit;
        }

    }
}
