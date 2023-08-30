#include "internal/engine/manager/gjob_manager.h"

#include <taskflow/taskflow.hpp>
#include "internal/engine/manager/glogger_manager.h"
#include "spdlog/fmt/fmt.h"
#include <string>

struct FireAndForget {
    tf::Executor executor;
    tf::Taskflow taskflow;
    tf::Task task = taskflow.placeholder();
    std::future<void> future;
    int timeOutMillis = 0;
    FireAndForget(IJobManager::fire_and_forget_fn callable,void* arg,int timeout,TIME_UNIT unit) {
        task.work([&] {
            callable(arg);
          });
        future = executor.run(taskflow);
        switch (unit)
        {
        case TIME_UNIT_MILLIS:
            timeOutMillis = timeout;
            break;
        case TIME_UNIT_SECONDS:
            timeOutMillis = timeout * 1000;
            break;
        case TIME_UNIT_MINUTES:
            timeOutMillis = timeout * 1000 * 60;
            break;
        default:
            break;
        }
    }

    ~FireAndForget() {
        auto status = future.wait_for(std::chrono::milliseconds(timeOutMillis));
        if (status == std::future_status::timeout)
        {
            std::stringstream ss;
            ss << std::this_thread::get_id();
            GLoggerManager::get_instance()->log_c("GJobManager",fmt::format("A Thread fired but couldn't forget {}",ss.str()).c_str());
        }
    }
};

void GJobManager::fire_and_forget(fire_and_forget_fn fn, void* arg)
{
    fire_and_forget(fn,arg, 10,TIME_UNIT_SECONDS);
}

void GJobManager::fire_and_forget(fire_and_forget_fn fn, void* arg, int timeout, TIME_UNIT unit)
{
    std::async([&] {
        fn(arg);
      });
}
