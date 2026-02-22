# Event Handler System

A web-based semester project for managing events, organizers, and user registrations.

## Features
- User authentication and settings management.
- Event creation with "Hero Image" uploads.
- Dynamic workshop management (Add/Remove via JavaScript).
- Registration system with individual workshop selection.
- Automatic database schema initialization.

## Installation

1. **Clone the repository** to your local server environment:
   ```bash
   git@gitlab.mff.cuni.cz:teaching/nswi142/2025-26/vladislav-levitskii.git
   ```
2. **Configure Database Credentials**:
Open the file containing the `connect_and_create()` function in ```private/models/default_model.php``` and update the connection parameters:
```php
mysqli_connect("host", "username", "password", "database_name");
```

3. **Server Requirements**:
* PHP 7.4 or higher.
* MySQL/MariaDB.
* Enabled `file_uploads` in `php.ini` for Hero images.



## Database Initialization

The system features an **automated initialization process**. You do not need to import a `.sql` file manually.

Upon the first launch, the `connect_and_create()` function is triggered, which executes the following schema:

* **Users Table**: Stores unique usernames and emails.
* **Events Table**: Stores event details, image paths, and links to the `organizer_id`.
* **Registrations Table**: A junction table handling the many-to-many relationship between users and events, including specific workshop selections.

All tables utilize `FOREIGN KEY` constraints with `ON DELETE CASCADE` to ensure data integrity.

## Screenshots

There are some screenshots of the web page.

### Landing Page

![Landing Page](files_readme/New_events.png)

### Register for an Event Page

![Register for an Event Page](files_readme/Registration.png)


### Workshops

![Workshops](files_readme/Workshops.png)
