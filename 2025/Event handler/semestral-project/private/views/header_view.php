<nav class="header_nav">
    <ul>
        <li>
            <a href="<?= BASE_URL ?>/">
                <img src="<?= BASE_URL ?>/assets/logo.png" alt="Logo">
                <p>Event handler</p>
            </a>
        </li>
        <li class="header_right">
            <ul>
                <li><a href="<?= BASE_URL ?>/">Home</a></li>
                <li><a href="<?= BASE_URL ?>/events/">All events</a></li>
                <?php 
                
                if (!isset($_SESSION['user_name'])) {
                    ?>
                        <li><a href="<?= BASE_URL ?>/login/">Login</a></li>
                        <li><a href="<?= BASE_URL ?>/register/">Register</a></li>
                    <?php
                } else {
                    ?>
                        <li>Logged: <?= htmlspecialchars($_SESSION['user_name']) ?></li>
                        <li><a href="<?= BASE_URL ?>/settings/">Settings</a></li>
                        <li><a href="<?= BASE_URL ?>/logout/">Logout</a></li>
                        <li><a href="<?= BASE_URL ?>/events/mine/">Your registrations</a></li>
                        <li><a href="<?= BASE_URL ?>/events/new/">Create new event</a></li>
                    <?php
                }

                ?>
            </ul>
        </li>
    </ul>
</nav>