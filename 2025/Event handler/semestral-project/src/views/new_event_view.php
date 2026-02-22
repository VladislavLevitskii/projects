<?php 

class NewEventView {
    public function render() {
        ?>
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
                <title>Create new event</title>
                <?php require_once __DIR__ . "/event_form.php" ?>
            </head>
            <body>
                <header>
                    <?php require "header_view.php" ?>
                </header>
                <main>
                    <?php 
                    if (!isset($_SESSION['id'])) {
                        http_response_code(404);
                        require_once __DIR__ . "/presenters/error_presenter.php";
                        $presenter = new ErrorPresenter();
                        $presenter->render("You are not registered :o");
                        exit;
                    } 
                    ?>
                    <h1>Create your own event !</h1>
                    <form action="<?= BASE_URL ?>/events/new/" method="post" enctype="multipart/form-data" class="event-form">
                        <label for="name">Name:</label>
                        <input type="text" id="name" name="name" minlength="1" required>

                        <label for="desc">Description:</label>
                        <textarea id="desc" name="description"></textarea>

                        <div class="date-row">
                            <div>
                                <label for="start">Start date:</label>
                                <input type="date" id="start" name="start_date" required>
                            </div>
                            <div>
                                <label for="end">End date:</label>
                                <input type="date" id="end" name="end_date" required>
                            </div>
                        </div>

                        <label for="hero">Hero image:</label>
                        <input type="file" id="hero" name="hero_image_path" class="file-input">

                        <label for="itemInput">Workshops:</label>
                        <div class="workshop-group">
                            <input type="text" id="itemInput" placeholder="Write your workshop(s)">
                            <button type="button" onclick="addItem()">Add to list</button>
                        </div>
                        
                        <ul id="itemList"></ul>
                        <input type="hidden" name="workshops" id="hiddenItems">

                        <button type="submit" class="submit-btn">Create Event</button>
                    </form>
                </main>
            </body>
            <script src="<?=BASE_URL?>/public/assets/js/adding_workshops.js"></script>
        </html>
        <?php
    }
}

?>
