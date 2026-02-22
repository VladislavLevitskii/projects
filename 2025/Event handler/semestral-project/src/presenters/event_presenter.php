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

    public function render($id, $referalLinkName) {
        try {
            $data = $this->model->getDataByID($id);
        } catch (mysqli_sql_exception $e) {
            http_response_code(500);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Something wrong with the database, repeat later");
            exit;
        }

        if ($data !== NULL) {
            $this->view = new EventView();
            $userName = $_SESSION['user_name'] ?? null;
            $organizerName = $data['name_organizer'];
            $data['is_owner'] = ($userName !== null && $userName === $organizerName);
            $data['is_logged_in'] = ($userName !== null);

            if ($data['is_owner']) {
                $data['referral_stats'] = $this->model->getReferralStats($id);
            } else {
                $data['referral_stats'] = [];
            }

            if ($referalLinkName !== null) {
                $domain = $_SERVER['HTTP_HOST'];
                $referralUrl = $domain . BASE_URL . "/events/" . $id . "/register/?ref=" . htmlspecialchars($referalLinkName);
                $data['referal_link'] = $referralUrl;
            }
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
        try {
            $this->model->deleteEvent($id);
        } catch (mysqli_sql_exception $e) {
            http_response_code(500);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Something wrong with the database, repeat later");
            exit;
        }
    }

    public function createReferalLink($referalName, $userId, $idEvent) {
        if (preg_match('/^[a-zA-Z0-9]+$/', $referalName)) {

            try {
                $this->model->createReferalLink($referalName, $userId, $idEvent);
            } catch (mysqli_sql_exception $e) {
                http_response_code(500);
                require_once __DIR__ . "/error_presenter.php";
                $presenter = new ErrorPresenter();
                
                if ($e->getCode() == 1062) { // duplicity
                    http_response_code(400);
                    $presenter->render("This name of referral link for this event already exists");
                } else {
                    http_response_code(500);
                    $presenter->render("Something wrong with the database");
                }

                exit;
            }

        } else {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("Invalid referal link name, use only a-Z and numbers");
            exit;
        }
    }
}
