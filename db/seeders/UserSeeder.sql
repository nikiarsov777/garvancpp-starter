-- UserSeeder -- run with `kalpasan seed --class=UserSeeder`

-- migrate:up

INSERT INTO users (
    "name",
    email,
    email_verified_at,
    "password",
    remember_token,
    current_team_id,
    profile_photo_path,
    created_at,
    updated_at,
    two_factor_secret,
    two_factor_recovery_codes,
    two_factor_confirmed_at,
    bcol,
    "money",
    fcol,
    jcol
) VALUES
    ('TTT',  'ttt@example.com',  '2026-05-21 10:00:00', 'password1', 'tok_ttt_0001', 1,    '/photos/ttt.png',  '2026-05-21 10:00:00', '2026-05-21 10:00:00', NULL,          NULL,                  NULL,                  TRUE,  100.00::money,  1.5,     '{"role":"admin"}'::json),
    ('TTT1', 'ttt1@example.com', NULL,                  'password1', 'tok_ttt_0002', 1,    '/photos/ttt1.png', '2026-05-21 10:05:00', '2026-05-21 10:05:00', 'secret_ttt1', '["code-1","code-2"]', '2026-05-21 10:10:00', FALSE, 250.50::money,  2.71828, '{"role":"user","tier":1}'::json),
    ('TTT2', 'ttt2@example.com', '2026-05-21 10:30:00', 'password1', NULL,           2,    NULL,               '2026-05-21 10:30:00', '2026-05-21 10:30:00', NULL,          NULL,                  NULL,                  NULL,  1000.99::money, 3.14159, '{"role":"user","tier":2,"flags":["a","b"]}'::json)
;

-- migrate:down

DELETE FROM users WHERE email IN ('ttt@example.com', 'ttt1@example.com', 'ttt2@example.com');
