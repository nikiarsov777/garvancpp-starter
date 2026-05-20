-- migrate:up
CREATE TABLE cars (
  brand VARCHAR(255),
  model VARCHAR(255),
  year INT
);
-- migrate:down
DROP TABLE cars;
