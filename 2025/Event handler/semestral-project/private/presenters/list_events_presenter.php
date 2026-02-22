<?php

require_once __DIR__ . '/../views/list_events_view.php';
require_once __DIR__ . '/../models/list_events_model.php';

class ListEventsPresenter {
    public function render() {

        $model = new ListEventsModel();
        $view = new ListEventsView();

        try {
            $data = $model->getData();
            $view->render($data);
        } catch (mysqli_sql_exception $e) {
            http_response_code(500);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Something wrong with the database, repeat later");
            exit;
        }
        $model->connection->close();
    }
}
