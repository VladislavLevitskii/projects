<?php

require_once __DIR__ . '/../views/event_register_view.php';
require_once __DIR__ . '/../models/registration_model.php';
require_once __DIR__ . '/../models/event_model.php';

class EventRegisterPresenter {
    public $modelEvents;
    public $modelRegistrations;
    public $view;

    public function __construct() {
        $this->modelEvents = new EventModel();
        $this->modelRegistrations = new RegistrationModel();
        $this->view = new EventRegisterView();
    }

    public function render($id) {
        $data = $this->modelRegistrations->getDataByID($id);
        if ($data !== NULL && isset($_SESSION['id'])) {
            $this->view->render($data);
        } else {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render("event doesnt exist");
            exit;
        }
        
        $this->modelRegistrations->connection->close();
    }

    public function registerEvent($postData, $id, $referralName) {
        if ($referralName !== null) {
            $referralCode = $this->modelEvents->getIDReferralLink($referralName, $id);
        } else {
            $referralCode = null;
        }
        $result = $this->modelRegistrations->registerEvent($postData, $id, $_SESSION['id'], $referralCode);
        if ($result !== true) {
            http_response_code(404);
            require_once __DIR__ . "/error_presenter.php";
            $presenter = new ErrorPresenter();
            $presenter->render($result);
            exit;
        }
    }

    public function renderLinkToLogin() {
        $this->view->render(-1);
    }
}