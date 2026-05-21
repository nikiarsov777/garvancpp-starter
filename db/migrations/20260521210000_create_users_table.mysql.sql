-- migrate:up
CREATE TABLE users (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL,
    email_verified_at TIMESTAMP NULL DEFAULT NULL,
    `password` VARCHAR(255) NOT NULL,
    remember_token VARCHAR(100) NULL DEFAULT NULL,
    current_team_id BIGINT NULL DEFAULT NULL,
    profile_photo_path VARCHAR(2048) NULL DEFAULT NULL,
    created_at TIMESTAMP NULL DEFAULT NULL,
    updated_at TIMESTAMP NULL DEFAULT NULL,
    two_factor_secret TEXT NULL,
    two_factor_recovery_codes TEXT NULL,
    two_factor_confirmed_at TIMESTAMP NULL DEFAULT NULL,
    bcol TINYINT(1) NULL DEFAULT NULL,
    `money` DECIMAL(19,4) NULL DEFAULT NULL,
    fcol DOUBLE NULL DEFAULT NULL,
    jcol JSON NULL,
    PRIMARY KEY (id),
    UNIQUE KEY users_email_unique (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- migrate:down
DROP TABLE users;
