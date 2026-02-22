<?php

class Router {
    
    public function run() {
        $url = $_SERVER['REQUEST_URI'];
        
        $basePath = BASE_URL;
        if (str_starts_with($url, $basePath)) {
            $url = substr($url, strlen($basePath));
        }
        
        switch (true) {
            case $url === '/':
                require_once __DIR__ . '/presenters/landing_page_presenter.php';
                $presenter = new LandingPagePresenter();
                $presenter->render();
                break;

            case preg_match('~^/events/(\d+)/$~', $url, $matches):
                require_once __DIR__ . '/presenters/event_presenter.php';
                $id = $matches[1];
                $presenter = new EventPresenter();
                if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete']) && isset($_SESSION['user_name'])) {
                    $presenter->deleteEvent($id);
                    header("Location: " . BASE_URL . "/events/");
                    exit();
                } else if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['referal_link']) && isset($_SESSION['user_name'])) {
                    $presenter->createReferalLink($_POST['referal_link'], $_SESSION['id'], $id);
                }
                $referralName = null;
                if (isset($_POST['referal_link'])) {
                    $referralName = $_POST['referal_link'];
                }
                $presenter->render($id, $referralName);
                break;

            case preg_match('~^/events/(\d+)/edit/$~', $url, $matches):
                require_once __DIR__ . '/presenters/event_edit_presenter.php';
                $id = $matches[1];
                $presenter = new EventEditPresenter();
                if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_SESSION['user_name'])) {
                    $presenter->changeEvent($_POST, $_FILES);
                } else {
                    $presenter->render($id);
                }
                break;

            case preg_match('~^/events/(\d+)/register/(\?.*)?$~', $url, $matches):
                require_once __DIR__ . '/presenters/event_register_presenter.php';
                $id = $matches[1];
                
                if (isset($_GET['ref'])) {
                    $_SESSION['referral_link_identifier'] = $_GET['ref'];
                }
                
                $presenter = new EventRegisterPresenter();
                
                if (!isset($_SESSION['user_name'])) {
                    $presenter->renderLinkToLogin();
                } else {
                    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
                        $referralName = $_SESSION['referral_link_identifier'] ?? null;
                        $presenter->registerEvent($_POST, $id, $referralName);
                        header("Location: " . BASE_URL . "/events/" . $id . "/");
                        exit();
                    } else {
                        $presenter->render($id);
                    }
                }

                break;

            case $url === '/events/':
                require_once __DIR__ . '/presenters/list_events_presenter.php';
                $presenter = new ListEventsPresenter();
                $presenter->render();
                break;

            case $url === '/events/mine/':
                require_once __DIR__ . '/presenters/list_events_mine_presenter.php';
                $presenter = new ListEventsMinePresenter();
                if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_SESSION['user_name'])) {
                    $presenter->updateRegistration($_POST);
                } else {
                    $presenter->render();
                }
                break;

            case $url === '/login/':
                require_once __DIR__ . '/presenters/login_presenter.php';
                $presenter = new LoginPresenter();
                if ($_SERVER['REQUEST_METHOD'] === 'POST') {
                    $presenter->loginUser($_POST);
                } else {
                    $presenter->render();
                }
                break;

            case $url === '/logout/':
                session_destroy();
                header("Location: " . BASE_URL);
                exit;

            case $url === '/register/':
                require_once __DIR__ . '/presenters/register_presenter.php';
                $presenter = new RegisterPresenter();
                if ($_SERVER['REQUEST_METHOD'] === 'POST') {
                    $presenter->registerUser($_POST);
                } else {
                    $presenter->render(0);
                }
                break;

            case $url === '/events/new/':
                require_once __DIR__ . '/presenters/new_event_presenter.php';
                $presenter = new NewEventPresenter();
                if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_SESSION['user_name'])) {
                    $presenter->createEvent($_POST, $_FILES);
                } else {
                    $presenter->render();
                }
                break;

            case $url === '/settings/':
                require_once __DIR__ . '/presenters/settings_presenter.php';
                $presenter = new SettingsPresenter();
                if (isset($_SESSION['user_name']) && $_SERVER['REQUEST_METHOD'] === 'POST' && !isset($_POST['delete'])) {
                    $presenter->changeSettings($_POST);
                } else if (isset($_SESSION['user_name']) && $_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete']) && isset($_SESSION['user_name'])) {
                    $presenter->deleteAccount($_SESSION['id']);
                } else {
                    $presenter->render();
                }
                break;

            default:
                http_response_code(404);
                require_once __DIR__ . "/presenters/error_presenter.php";
                $presenter = new ErrorPresenter();
                $presenter->render("page doesnt exist");
                exit;
        }

    }

}
