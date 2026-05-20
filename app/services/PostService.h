#ifndef POSTSERVICE_H
#define POSTSERVICE_H

#pragma once

#include "service/BaseService.h"

namespace AppServices
{

    class PostService : public Garvan::BaseService
    {
    public:
        PostService();
        ~PostService();

        json index() override;

    private:
    };
}

#endif
