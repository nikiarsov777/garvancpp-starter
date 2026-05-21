-- migrate:up
CREATE TABLE public.users (
    id bigserial NOT NULL,
    "name" varchar(255) NOT NULL,
    email varchar(255) NOT NULL,
    email_verified_at timestamp(0) NULL,
    "password" varchar(255) NOT NULL,
    remember_token varchar(100) NULL,
    current_team_id int8 NULL,
    profile_photo_path varchar(2048) NULL,
    created_at timestamp(0) NULL,
    updated_at timestamp(0) NULL,
    two_factor_secret text NULL,
    two_factor_recovery_codes text NULL,
    two_factor_confirmed_at timestamp(0) NULL,
    bcol bool NULL,
    "money" money NULL,
    fcol float8 NULL,
    jcol json NULL,
    CONSTRAINT users_email_unique UNIQUE (email),
    CONSTRAINT users_pkey PRIMARY KEY (id)
);
-- migrate:down
DROP TABLE public.users;
