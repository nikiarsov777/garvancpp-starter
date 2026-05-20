-- UserSeeder -- run with `kalpasan seed --class=UserSeeder`

-- migrate:up

INSERT INTO users (name, email, password) VALUES
     ('TTT', 'ttt@example.com', 'password1'),
     ('TTT1', 'ttt1@example.com', 'password1'),
     ('TTT2', 'ttt2@example.com', 'password1')
;

-- migrate:down

-- DELETE FROM users WHERE col_a IN ('value_a', 'value_c');
