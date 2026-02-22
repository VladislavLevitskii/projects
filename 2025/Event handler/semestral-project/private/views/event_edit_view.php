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
                            ?> <p>Succesfully edited !</p> <?php
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
            <script>
                let hiddenItems = document.getElementById('hiddenItems');
                let itemsArray = hiddenItems.value ? hiddenItems.value.split(" ") : [];
                itemsArray.forEach(element => {
                    const li = document.createElement('li');
                    li.textContent = element + " ";
                    const button = document.createElement('button');
                    button.textContent = '';
                    button.type = '';
                    button.className = 'delete-ws';
                    button.

                    button.onclick = function() {
                        li.remove();
                        
                        const index = itemsArray.indexOf(element);
                        if (index > -1) {
                            itemsArray.splice(index, 1);
                        }
                        
                        hiddenItems.value = itemsArray.join(' ');
                    };
                    li.appendChild(button);
                    document.getElementById('itemList').appendChild(li);
                });

                function addItem() {
                    const input = document.getElementById('itemInput');
                    const value = input.value.trim();
                    
                    if (value && !value.includes(" ")) {
                        itemsArray.push(value);
                        
                        const li = document.createElement('li');
                        li.textContent = value + " ";
                        
                        const button = document.createElement('button');
                        button.textContent = '';
                        button.type = 'button';
                        button.className = 'delete-ws';
                        
                        button.onclick = function() {
                            li.remove();
                            
                            const index = itemsArray.indexOf(value);
                            if (index > -1) {
                                itemsArray.splice(index, 1);
                            }
                            
                            document.getElementById('hiddenItems').value = itemsArray.join(' ');
                        };
                        
                        li.appendChild(button);
                        document.getElementById('itemList').appendChild(li);
                        document.getElementById('hiddenItems').value = itemsArray.join(' ');
                        
                        input.value = '';
                    } else {
                        alert("Item should not have spaces !");
                    }
                }

            </script>
        </html>
        <?php
    }
}

?>