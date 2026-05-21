-- migrate:up
CREATE TABLE users (
    id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL,
    email_verified_at DATETIME NULL,
    password VARCHAR(255) NOT NULL,
    remember_token VARCHAR(100) NULL,
    current_team_id INTEGER NULL,
    profile_photo_path VARCHAR(2048) NULL,
    created_at DATETIME NULL,
    updated_at DATETIME NULL,
    two_factor_secret TEXT NULL,
    two_factor_recovery_codes TEXT NULL,
    two_factor_confirmed_at DATETIME NULL,
    bcol INTEGER NULL,
    money NUMERIC NULL,
    fcol REAL NULL,
    jcol TEXT NULL,
    CONSTRAINT users_email_unique UNIQUE (email)
);
-- migrate:down
DROP TABLE users;
