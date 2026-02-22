<?php 

class SettingsView {
    public function render($data, $result) {
        ?>
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="stylesheet" href="<?= BASE_URL ?>/assets/styles.css">
                <title>Settings</title>
                <?php require_once __DIR__ . "/form-style.php" ?>
                <style>
                    .auth-subform, .delete-area {
                        background: none;
                        box-shadow: none;
                        padding: 0;
                        margin: 0;
                        max-width: none;
                    }

                    .delete-area {
                        margin-top: 15px;
                        border-top: 1px solid #eee;
                        padding-top: 15px;
                    }

                    .auth-form .btn-secondary {
                        background: #fff;
                        color: #dc3545;
                        border: 1px solid #dc3545;
                        width: 100%;
                    }

                    .auth-form .btn-secondary:hover {
                        background: #dc3545;
                        color: white;
                    }
                </style>
            </head>
            <body>
                <header>
                    <?php include "header_view.php" ?>
                </header>
                <main>
                    <?php 
                    if ($result === 1) {
                        ?> <p style="margin: 50px 0 0 50px; text-align: center;">Changed successfully !</p> <?php
                    }
                    ?>

                    <?php 
                    if ($result !== 2) {
                        ?> 
                        <div class="auth-form"> 
                            <form action="<?= BASE_URL ?>/settings/" method="post" class="auth-subform">
                                <label>Name:</label>
                                <input type="text" name="name" value="<?=htmlspecialchars($data['username'])?>">
                                
                                <label>Email:</label>
                                <input type="text" name="email" value="<?=htmlspecialchars($data['email'])?>">
                                
                                <button type="submit">Edit</button>
                            </form>

                            <form action="<?= BASE_URL ?>/settings/" method="post" class="delete-area">
                                <input type="hidden" name="delete" value="1">
                                <button type="submit" class="btn-secondary">Delete account</button>
                            </form>
                            
                        </div>
                        
                        <?php
                    } else if ($result === 2){
                        ?> 
                        <div style="display: flex; flex-direction: column; justify-content: center;">
                            <p style="text-align: center;">Account deleted !</p>
                            <a style="background: #007bff;color: white; text-align: center; margin: 0 auto; padding: 8px 16px; border-radius: 5px; font-weight: bold;" href="<?= BASE_URL ?>">Login</a>
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