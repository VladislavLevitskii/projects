<?php 

class EventEditView {
    public function render($data, $edited) {
        $dateStart = explode(" ", $data['start_date']);
        $dateEnd = explode(" ", $data['end_date']);
        $src = BASE_URL . "/data/" . $data['hero_image_path'];
        ?>
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
                <title>Edit the event</title>
                <?php require_once __DIR__ . "/event_form.php" ?>
            </head>
            <body>
                <header>
                    <?php require "header_view.php" ?>
                </header>
                <main>
                    <article class="event">
                        <?php
                        if ($edited === 1) {
                            ?> <p style="text-align: center;">Succesfully edited !</p> <?php
                        }
                        ?>
                        <form action="<?= BASE_URL ?>/events/<?= $data['id'] ?>/edit/" method="post" enctype="multipart/form-data" class="event-form">
                            <input type="hidden" name="id" value="<?=htmlspecialchars($data['id'])?>">

                            <label for="edit-name">Name:</label>
                            <input type="text" id="edit-name" name="name" value="<?=htmlspecialchars($data['name'])?>" required>

                            <label for="edit-desc">Description:</label>
                            <textarea id="edit-desc" name="description"><?=htmlspecialchars($data['description'])?></textarea>

                            <div class="date-row">
                                <div>
                                    <label for="edit-start">Start date:</label>
                                    <input type="date" id="edit-start" name="start_date" value="<?=htmlspecialchars($dateStart[0])?>" required>
                                </div>
                                <div>
                                    <label for="edit-end">End date:</label>
                                    <input type="date" id="edit-end" name="end_date" value="<?=htmlspecialchars($dateEnd[0])?>" required>
                                </div>
                            </div>

                            <label>Hero image now:</label>
                            <?php if ($data['hero_image_path'] !== NULL): ?>
                                <img class="event_image" src="<?=htmlspecialchars($src)?>" alt="hero image">
                            <?php else: ?>
                                <p style="font-style: italic; color: #888;">Now there is no image</p>
                            <?php endif; ?>

                            <label for="edit-hero">Change image:</label>
                            <input type="file" id="edit-hero" name="hero_image_path">

                            <label for="itemInput">Workshops:</label>
                            <div class="workshop-group">
                                <input type="text" id="itemInput" placeholder="Write your workshop(s)">
                                <button type="button" onclick="addItem()">Add to list</button>
                            </div>

                            <ul id="itemList">
                                </ul>
                            <input type="hidden" name="workshops" id="hiddenItems" value="<?=htmlspecialchars($data['workshops'])?>">

                            <button type="submit" class="submit-btn">Save Changes</button>
                        </form>
                    </article>
                </main>
            </body>
            <script src="<?=BASE_URL?>/public/assets/js/workshop_manager.js"></script>
        </html>
        <?php
    }
}

?>