-- migrate:up
{
  "create": "users",
  "validator": {
    "$jsonSchema": {
      "bsonType": "object",
      "required": ["name", "email", "password"],
      "properties": {
        "name": {
          "bsonType": "string",
          "maxLength": 255,
          "description": "user display name, required"
        },
        "email": {
          "bsonType": "string",
          "maxLength": 255,
          "description": "user email, required and unique"
        },
        "email_verified_at": {
          "bsonType": ["date", "null"],
          "description": "timestamp when email was verified"
        },
        "password": {
          "bsonType": "string",
          "maxLength": 255,
          "description": "password hash, required"
        },
        "remember_token": {
          "bsonType": ["string", "null"],
          "maxLength": 100
        },
        "current_team_id": {
          "bsonType": ["long", "int", "null"]
        },
        "profile_photo_path": {
          "bsonType": ["string", "null"],
          "maxLength": 2048
        },
        "created_at": {
          "bsonType": ["date", "null"]
        },
        "updated_at": {
          "bsonType": ["date", "null"]
        },
        "two_factor_secret": {
          "bsonType": ["string", "null"]
        },
        "two_factor_recovery_codes": {
          "bsonType": ["string", "null"]
        },
        "two_factor_confirmed_at": {
          "bsonType": ["date", "null"]
        },
        "bcol": {
          "bsonType": ["bool", "null"]
        },
        "money": {
          "bsonType": ["decimal", "double", "null"],
          "description": "monetary amount"
        },
        "fcol": {
          "bsonType": ["double", "null"]
        },
        "jcol": {
          "bsonType": ["object", "array", "null"]
        }
      }
    }
  }
}
-- migrate:down
{ "drop": "users" }
