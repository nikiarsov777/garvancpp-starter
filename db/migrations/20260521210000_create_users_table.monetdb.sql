-- migrate:up
CREATE TABLE users (
    id BIGINT NOT NULL AUTO_INCREMENT,
    "name" VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL,
    email_verified_at TIMESTAMP NULL,
    "password" VARCHAR(255) NOT NULL,
    remember_token VARCHAR(100) NULL,
    current_team_id BIGINT NULL,
    profile_photo_path VARCHAR(2048) NULL,
    created_at TIMESTAMP NULL,
    updated_at TIMESTAMP NULL,
    two_factor_secret CLOB NULL,
    two_factor_recovery_codes CLOB NULL,
    two_factor_confirmed_at TIMESTAMP NULL,
    bcol BOOLEAN NULL,
    "money" DECIMAL(19,4) NULL,
    fcol DOUBLE NULL,
    jcol JSON NULL,
    CONSTRAINT users_email_unique UNIQUE (email),
    CONSTRAINT users_pkey PRIMARY KEY (id)
);
-- migrate:down
DROP TABLE users;
