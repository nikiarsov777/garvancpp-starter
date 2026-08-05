#include "AppServiceProvider.h"

#include "queue/JobDispatcher.h"
#include "queue/drivers/SyncDriver.h"

using namespace AppProviders;

void AppServiceProvider::register_()
{
    // Phase A: единственият driver е sync. В следващи фази:
    //   Phase B: JobDispatcher::bind("async",
    //                std::make_unique<Garvan::InMemoryAsyncDriver>(N));
    //   Phase C: JobDispatcher::bind("database",
    //                std::make_unique<Garvan::DatabaseDriver>(cfg));
    Garvan::JobDispatcher::bind("sync",
        std::make_unique<Garvan::SyncDriver>());
}

void AppServiceProvider::boot()
{
    // Phase A: нищо за pull. Phase B ще стартира worker thread pool
    // на in-memory driver-a тук.
}
