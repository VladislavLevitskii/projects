<?php 

class LandingPageView {
    public static function render($data) {
        ?>
        
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
            <title>Home</title>
            <?php require_once __DIR__ . "/events_style.php" ?>
        </head>
        <body>
            <main>
                <header>
                    <?php require __DIR__ . "/header_view.php" ?>
                </header>
                <h1>3 Newest events: </h1>
                <?php 
                if ($data->num_rows === 0) {
                    ?> <h2>There are no future events :(</h2> <?php
                } else {
                    ?>
                    <ul id="events">
                    <?php $i = 0;
                    foreach($data as $item) { ?>
                    <li>
                        <ul>
                            <?php 
                                ?> <li>Name: <?= htmlspecialchars($item['name']) ?></li> <?php
                                ?> <li>Start: <?= htmlspecialchars(explode(" ", $item['start_date'])[0]) ?></li> <?php
                                ?> <li>End: <?= htmlspecialchars(explode(" ", $item['end_date'])[0]) ?></li> <?php
                                ?> <li>Name of organizer: <?= htmlspecialchars($item['name_organizer']) ?></li> <?php
                                ?> <li><a href="<?=BASE_URL?>/events/<?=htmlspecialchars($item['id'])?>/">Details</a></li> <?php
                            ?>
                        </ul>
                    </li>
                        <?php } ?>
                    </ul>
                <?php } ?>
            </main>
        </body>
        </html>
        
        <?php
    }
}

?>
