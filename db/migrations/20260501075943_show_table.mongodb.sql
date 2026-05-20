-- migrate:up
{
  "create": "cars",
  "validator": {
    "$jsonSchema": {
      "bsonType": "object",
      "required": ["brand", "model", "year"],
      "properties": {
        "brand": {
          "bsonType": "string",
          "description": "must be a string"
        },
        "model": {
          "bsonType": "string",
          "description": "must be a string"
        },
        "year": {
          "bsonType": "int",
          "description": "must be an integer"
        }
      }
    }
  }
}
-- migrate:down
{ "drop": "cars" }
