#ifndef USERMODEL_H
#define USERMODEL_H

#include "basemodel.h"

namespace AppModels
{
class User : public BaseModel
{
public:
    string email;
    User();
    virtual ~User();

    // pqxx::result getAll();
    // pqxx::result all() override;

    User *posts();

protected:

private:
};
}

#endif // USERMODEL_H
