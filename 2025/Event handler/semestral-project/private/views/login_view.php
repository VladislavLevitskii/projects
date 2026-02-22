<?php 

class LoginView {
    public function render($error) {
        ?>
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
                <title>Login</title>
                <?php require_once __DIR__ . "/form-style.php" ?>
            </head>
            <body>
                <header>
                    <?php require "header_view.php" ?>
                </header>
                <main>
                    <form action="<?= BASE_URL ?>/login/" method="POST" class="auth-form">
                        <label for="user">Username:</label>
                        <input type="text" id="user" name="username">

                        <label for="mail">Email:</label>
                        <input type="text" id="mail" name="email">

                        <button type="submit">Login</button>
                    </form>
                    <?php
                    if ($error === 1) {
                        ?> <p>Wrong name or email</p> <?php
                    }
                    ?>
                </main>
            </body>
        </html>
        <?php
    }
}

?>
