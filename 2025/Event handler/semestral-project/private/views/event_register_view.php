<?php 

class EventRegisterView {
    public static function render($data) {
        ?>
        
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
            <title>Registration for the event</title>
        <style>
        main {
            max-width: 600px;
            margin: 3rem auto;
            padding: 2.5rem;
            background: #ffffff;
            border-radius: 16px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.05);
            font-family: 'Inter', sans-serif;
            border: 1px solid #f0f0f0;
            box-sizing: border-box;
        }

        h1 {
            font-size: 1.5rem;
            color: #1a1a1a;
            line-height: 1.4;
            margin-bottom: 1.5rem;
            text-align: center;
        }

        p {
            font-weight: 600;
            color: #4a5568;
            margin-bottom: 1rem;
            text-transform: uppercase;
            font-size: 0.85rem;
            letter-spacing: 0.05em;
        }

        form {
            display: flex;
            flex-direction: column;
        }

        form label {
            display: flex;
            align-items: center;
            background: #f8fafc;
            padding: 12px 16px;
            border-radius: 10px;
            margin-bottom: 8px;
            cursor: pointer;
            transition: all 0.2s ease;
            border: 2px solid transparent;
        }

        form label:has(input[type="checkbox"]:checked) {
            background: #f0fff4;
            border-color: #28a745;
        }

        form label:hover {
            background: #f1f5f9;
        }

        input[type="checkbox"] {
            width: 20px;
            height: 20px;
            margin-right: 12px;
            cursor: pointer;
            accent-color: #28a745;
        }

        input[type="checkbox"]:checked + span {
            font-weight: bold;
            color: #218838;
        }

        button[type="submit"] {
            margin-top: 1.5rem;
            background: #28a745;
            color: white;
            border: none;
            padding: 14px;
            border-radius: 10px;
            font-size: 1rem;
            font-weight: 700;
            cursor: pointer;
            transition: transform 0.1s, background 0.2s;
        }

        button[type="submit"]:hover {
            background: #218838;
        }

        button[type="submit"]:active {
            transform: scale(0.98);
        }
        </style>
        </head>
        <body>
            <header>
                <?php require __DIR__ . "/header_view.php" ?>
            </header>
            <main>
                <h1>Do you want to registrate for the event: <?= $data['name']?> ?</h1>
                <?php if (empty($data['workshops'])) {
                    ?> 
                    <form action="" method="post">
                        <button type="submit">Register</button>
                    </form> 
                    <?php
                    } else { ?>
                <p>Select workshops:</p>
                <form action="" method="post">
                    <?php 
                        $workshops = explode(" ", $data['workshops']);
                        foreach($workshops as $workshop) {
                            if (trim($workshop) !== "") { 
                                ?> 
                                <label class="workshop-option">
                                    <input type="checkbox" name="workshops[]" value="<?=htmlspecialchars($workshop)?>">
                                    <span><?=htmlspecialchars($workshop)?></span>
                                </label>
                                <?php
                            }
                        }
                    ?>
                    <button type="submit">Register</button>
                </form>
                <?php } ?>
            </main>
        </body>
        </html>
        
        <?php
    }
}

?>