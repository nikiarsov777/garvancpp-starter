#ifndef BASEMODEL_H
#define BASEMODEL_H

#include "model/model.h"

namespace AppModels {
class BaseModel : public Garvan::Model {
public:
    // Добавете конструктор, който приема име на драйвер/връзка
    BaseModel(const char* driver);
    BaseModel();
};
}


#endif // BASEMODEL_H
