#ifndef POSTCONTROLLER_H
#define POSTCONTROLLER_H

#include "controller/BaseContoller.h"
#include <bits/stdc++.h>
using namespace std;

namespace AppControllersApi
{

    class PostController : public Garvan::BaseContoller
    {
    public:
        PostController();
        ~PostController();

        json index();

    private:
    };
}

#endif
