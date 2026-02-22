<?php 

class ErrorView {
    public function render($error_message) {
        ?>
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
                <title>Error 404 - not found</title>
                <style>

                main {
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                    justify-content: center;
                    min-height: 60vh;
                    text-align: center;
                    font-family: 'Inter', sans-serif;
                }

                h1 {
                    background: #fff5f5;
                    color: #e53e3e;
                    padding: 2rem 3rem;
                    border-radius: 12px;
                    border: 2px solid #feb2b2;
                    box-shadow: 0 10px 15px -3px rgba(229, 62, 62, 0.1);
                    box-sizing: border-box;
                    max-width: 90%;
                    position: relative;
                    overflow: hidden;
                }

                h1 {
                    animation: shake 0.5s ease-in-out;
                }

                @keyframes shake {
                    0%, 100% { transform: translateX(0); }
                    25% { transform: translateX(-5px); }
                    75% { transform: translateX(5px); }
                }
                    
                </style>
            </head>
            <body>
                <header>
                    <?php require "header_view.php" ?>
                </header>
                <main>
                    <h1>Error: <?= $error_message?></h1>
                </main>
            </body>
        </html>
        <?php
    }
}

?>
