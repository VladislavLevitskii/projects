<?php 

class EventView {
    public static function render($data) {
        $src = BASE_URL . "/data/" . $data['hero_image_path'];
        ?>
        
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
            <title><?= htmlspecialchars($data['name']) ?> | Details</title>
            <?php require_once __DIR__ . "/event_style.php" ?>
        </head>
        <body>
            <main>
                <header>
                    <?php require __DIR__ . "/header_view.php" ?>
                </header>
                <article class="event">
                    <h1><?= htmlspecialchars($data['name']) ?></h1>
                    <p>Description: <?= htmlspecialchars($data['description']) ?></p>
                    <p>Start date: <?= htmlspecialchars(explode(" ", $data['start_date'])[0]) ?></p>
                    <p>End date: <?= htmlspecialchars(explode(" ", $data['end_date'])[0]) ?></p>
                    <?php if ($data['hero_image_path'] !== NULL) {
                        ?>
                        <p>Hero image: </p>
                        <img class="event_image" src="<?=$src?>" alt="picture of the hero">
                        <?php
                    } ?>
                    <p>Workshops:</p>
                    <ul>
                    <?php
                        $workshops = explode(" ", $data['workshops']);
                        foreach ($workshops as $workshop) {
                            if ($workshop !== "") {
                                ?> <li><?= $workshop ?></li> <?php
                            }
                        }
                    ?>
                    </ul>
                    <p>Organizer: <?= htmlspecialchars($data['name_organizer']) ?></p>
                    <?php 
                    
                    if (isset($_SESSION['user_name']) && $_SESSION['user_name'] === htmlspecialchars($data['name_organizer'])) { ?>
                        <a href="edit/">Edit</a>
                        <form action="<?= BASE_URL ?>/events/<?=htmlspecialchars($data['id'])?>/" method="post">
                            <input type="hidden" name="delete" value="1">
                            <button type="submit">Delete event</button>
                        </form>
                        <?php
                    } else if (isset($_SESSION['user_name'])){
                        ?> <a href="<?= BASE_URL ?>/events/<?=htmlspecialchars($data['id'])?>/register/">Register for the event</a> <?php
                    }

                    ?>
                </article>
            </main>
        </body>
        </html>
        
        <?php
    }
}

?>