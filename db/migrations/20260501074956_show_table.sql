-- migrate:up
CREATE TABLE show (
  brand VARCHAR(255),
  model VARCHAR(255),
  year INT
);
-- migrate:down
DROP TABLE show;
