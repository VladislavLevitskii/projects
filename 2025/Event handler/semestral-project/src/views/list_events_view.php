<?php 

class ListEventsView {
    public static function render($data) {
        ?>
        
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
            <title>All events</title>
            <?php require_once __DIR__ . "/events_style.php" ?>
            <style>

            .selected {
                border: 3px solid darkblue;
                box-sizing: border-box;
            }

            #selector {
                display: flex;
                justify-content: center;
                gap: 10px;
                margin-bottom: 30px;
            }

            #selector div {
                width: 30px;
                height: 30px;
                background: #007bff;
                display: flex;
                align-items: center;
                justify-content: center;
                border-radius: 10px;
                color: white;
            }

            #selector div:hover {
                background: #0056b3;
                cursor: pointer;
            }

            </style>
        </head>
        <body>
            <main>
                <header>
                    <?php require __DIR__ . "/header_view.php" ?>
                </header>
                <h1>All events: </h1>
                <ul id = "events">
                <?php 
                if ($data->num_rows === 0) {
                    ?><h2>There are no events :(</h2><?php
                } else {
                    foreach($data as $item) { ?>
                        <li>
                            <ul>
                                    <li>Name: <?=htmlspecialchars($item['name'])?></li>
                                    <li>Start: <?=htmlspecialchars(explode(" ", $item['start_date'])[0])?></li>
                                    <li>End: <?=htmlspecialchars(explode(" ", $item['end_date'])[0])?></li>
                                    <li>Name of organizer: <?=htmlspecialchars($item['name_organizer'])?> </li>
                                    <li><a href="<?=BASE_URL?>/events/<?=htmlspecialchars($item['id'])?>/">Details</a></li>
                            </ul>
                        </li>
                        <?php } ?>
                <?php } ?>
                </ul>
                <div id = "selector"></div>
            </main>
            <script src="<?=BASE_URL?>/public/assets/js/pagination.js"></script>
        </body>
        </html>
        
        <?php
    }
}

?>
