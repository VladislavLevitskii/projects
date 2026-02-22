<?php 

class ListEventsMineView {
    public static function render($data) {
        ?>
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
            <title>All your events</title>
            
            <style>
                main {
                    max-width: 900px;
                    margin: 2rem auto;
                    padding: 0 1rem;
                    font-family: 'Inter', sans-serif;
                }

                h1 {
                    color: #1a1a1a;
                    border-bottom: 3px solid #007bff;
                    display: inline-block;
                    margin-bottom: 2rem;
                }

                main ul:not(ol ul) {
                    background: white;
                    list-style: none;
                    padding: 2rem;
                    margin-bottom: 2rem;
                    border-radius: 16px;
                    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.05);
                    border: 1px solid #eee;
                }

                main ul li {
                    margin-bottom: 0.8rem;
                    color: #4a5568;
                    line-height: 1.5;
                }

                main ul > li:first-child {
                    font-size: larger;
                    font-weight: bolder;
                    text-align: center;
                }

                main ul li strong {
                    color: #1a1a1a;
                    font-weight: 700;
                }

                .event_image {
                    width: 200px;
                    height: 120px;
                    object-fit: cover;
                    border-radius: 8px;
                    margin-top: 10px;
                    border: 1px solid #ddd;
                }

                ol {
                    background: #f8fafc;
                    padding: 1rem 2rem;
                    border-radius: 8px;
                    margin-top: 10px;
                }

                ol li {
                    font-weight: 600;
                    color: #2d3748;
                }

                a[href*="/events/"] {
                    display: inline-block;
                    background: #007bff;
                    color: white;
                    padding: 10px 20px;
                    border-radius: 8px;
                    text-decoration: none;
                    font-weight: bold;
                    margin: 10px 0;
                    transition: background 0.2s;
                }

                a[href*="/events/"]:hover {
                    background: #0056b3;
                }

                form {
                    background: #f1f5f9;
                    padding: 1.5rem;
                    border-radius: 12px;
                    margin-top: 15px;
                }

                form label {
                    display: flex;
                    align-items: center;
                    gap: 10px;
                    cursor: pointer;
                    margin-bottom: 5px;
                }

                input[type="checkbox"] {
                    width: 18px;
                    height: 18px;
                    accent-color: #007bff;
                }

                button[type="submit"] {
                    background: #2d3748;
                    color: white;
                    border: none;
                    padding: 8px 16px;
                    border-radius: 6px;
                    cursor: pointer;
                    font-weight: bold;
                    margin-top: 10px;
                }

                button[type="submit"]:hover {
                    background: #1a202c;
                }
                </style>

        </head>
        <body>
            <header>
                <?php require __DIR__ . "/header_view.php" ?>
            </header>
            <main>
                <h1>All your events: </h1>
                <?php 
                if (empty($data)) {
                    ?><h2>There are no events :(</h2><?php
                } else {
                    foreach($data as $item) { ?>
                        <ul>
                                <li>Name: <?=htmlspecialchars($item['name'])?></li>
                                <li>Description: <?=htmlspecialchars($item['description'])?></li>
                                <li>Start: <?=htmlspecialchars($item['start_date'])?></li>
                                <li>End: <?=htmlspecialchars($item['end_date'])?></li>
                                <?php
                                if (!empty($item['hero_image_path'])) {
                                ?>
                                    <li>
                                        <div>Hero image:</div>
                                        <img class="event_image" src="<?=BASE_URL . "/data/" . htmlspecialchars($item['hero_image_path']);?>" alt="picture of the hero">
                                    </li>
                                <?php
                                }
                                ?>
                                <?php
                                if (!empty(htmlspecialchars($item['user_workshops']))) {
                                    ?><li> Workshops you registered for:<?php
                                    $workshops_user = explode(" ", htmlspecialchars($item['user_workshops']));
                                    ?><ol><?php
                                    foreach($workshops_user as $workshop_user) {
                                        ?> <li><?=htmlspecialchars($workshop_user)?></li> <?php
                                    }
                                    ?></ol></li><?php
                                }
                                ?>
                                <li><a href="<?=BASE_URL?>/events/<?=$item['id']?>/">Details of the event</a></li>
                                <li>Change workshops: 
                                    <form action="" method="post">
                                        <?php 
                                            $workshops = explode(" ", $item['workshops']);
                                            foreach($workshops as $workshop) {
                                                ?> 
                                                
                                                <label>
                                                    <input type="checkbox" <?php if (isset($workshops_user) && in_array($workshop, $workshops_user)) { ?> checked <?php } ?> name="workshops[]" value="<?=$workshop?>">
                                                    <span><?=htmlspecialchars($workshop)?></span>
                                                </label>

                                                <?php
                                            } ?>
                                        <input type="hidden" name="id" value = "<?=htmlspecialchars($item['id'])?>">
                                        <button type="submit">Update</button>
                                    </form>
                                </li>
                        </ul>
                        <?php } ?>
                <?php } ?>
            </main>
        </body>
        </html>
        
        <?php
    }
}

?>
