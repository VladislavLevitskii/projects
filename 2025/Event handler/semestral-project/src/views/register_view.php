<?php 

class RegisterView {
    public function render($result) {
        ?>
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
                <title>Registration</title>
                <?php require_once __DIR__ . "/form-style.php" ?>
            </head>
            <body>
                <header>
                    <?php require "header_view.php" ?>
                </header>
                <main>
                    <?php
                    if ($result === 0) { ?>
                        <form action="<?= BASE_URL ?>/register/" method="POST" class="auth-form">
                            <label for="reg-user">Username:</label>
                            <input type="text" id="reg-user" name="username">

                            <label for="reg-mail">Email:</label>
                            <input type="text" id="reg-mail" name="email">

                            <button type="submit">Register</button>
                        </form>
                    <?php } else if ($result === 1) {
                        ?> 
                        <div style="display: flex; flex-direction: column; justify-content: center; align-items: center;">
                            <p style="text-align: center;">Succesfully registered</p> 
                            <a style="background: #007bff;color: white; text-align: center; margin: 0 auto; padding: 8px 16px; border-radius: 5px; font-weight: bold;" href="<?= BASE_URL ?>/">Home</a>
                        </div>
                        <?php
                    } else {
                        ?> 
                        <div style="display: flex; flex-direction: column; justify-content: center;">
                            <p style="text-align: center;"><?= htmlspecialchars($result) ?></p>
                            <a style="background: #007bff;color: white; text-align: center; margin: 0 auto; padding: 8px 16px; border-radius: 5px; font-weight: bold;" href="<?= BASE_URL ?>/login/">Login</a>
                        </div>
                        <?php
                    }
                    ?>
                </main>
            </body>
        </html>
        <?php
    }
}

?>
