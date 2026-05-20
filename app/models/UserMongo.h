#ifndef USERMONGO_H
#define USERMONGO_H

#include "basemodel.h"

namespace AppModels
{
class UserMongo :  public BaseModel
{
public:
    UserMongo();
    // Конструктор, който позволява смяна на драйвера при нужда
    // UserMongo(const char* customDriver);
private:
    const char* driver = "MONGO_DB";
};
}
#endif // USERMONGO_H
