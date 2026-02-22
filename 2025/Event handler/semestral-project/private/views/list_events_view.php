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
            <script>
                function jump_to_page(index_page) {
                    page = index_page;
                    const min_index = items_per_page * (index_page - 1) + 1;
                    const max_index = items_per_page * index_page;
                    Array.from(list.children).forEach((child, index) => {
                        if (index + 1 > max_index ||  index + 1 < min_index) {
                            child.style.display = 'none';
                        } else {
                            child.style.display = 'list-item';
                        }
                    });
                    Array.from(buttons.children).forEach((child, index) => {
                        if (index + 1 == index_page) {
                            child.classList.add('selected');
                        } else {
                            child.classList.remove('selected');
                        }
                    });
                }

                function create_buttons() {
                    if (number_of_pages === 1) {
                        return;
                    }
                    for (let index = 1; index < number_of_pages + 1; index++) {
                        const button = document.createElement('div');
                        button.textContent = index;
                        button.addEventListener('click', () => jump_to_page(button.textContent));
                        buttons.appendChild(button);
                    }
                }

                const list = document.getElementById('events');
                const total_items = list.childElementCount;
                const items_per_page = 7;
                const number_of_pages = Math.floor(total_items / (items_per_page + 1)) + 1;
                const buttons = document.getElementById('selector');
                let page = 0;
                create_buttons();
                jump_to_page(1);
            </script>
        </body>
        </html>
        
        <?php
    }
}

?>
