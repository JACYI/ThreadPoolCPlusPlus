//
// Created by yiyonghao on 2023/8/30.
//

#ifndef THREADPOOL_THREAD_POOL_H
#define THREADPOOL_THREAD_POOL_H
#include <functional>
#include <bits/stdc++.h>

namespace std {
#define THREAD_POOL_MAX_SIZE 16
//#define THREADPOOL_AUTO_GROW

    class thread_pool {
    private:
        unsigned short init_size;
        using Task = function<void()>;
        queue<Task> tasks_list;
        vector<thread> threads_list;
        mutex tp_lock;
#ifdef THREADPOOL_AUTO_GROW
        mutex grow_lock;
#endif
        condition_variable task_cv;
        atomic<bool> run{ true };
        atomic<int> thread_num{ 0 };
    public:
        inline explicit thread_pool(unsigned short size = 4) { init_size = size; }
        inline ~thread_pool()
        {
            run = false;
            task_cv.notify_all();
            for(thread &t : threads_list)
            {
                if(t.joinable())
                    t.join();
            }
        }

        template<class F, class... Args>
        auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>
        {
            if(!run)
                throw runtime_error("commit thread pool is stopped.");
            using FRetType = decltype(f(args...));
            auto task = make_shared<packaged_task<FRetType()>>(
                    bind(forward<F>(f), forward<Args>(args)...)
                    );
            future<FRetType> future = task->get_future();
            {
                /* add task to task list */
                lock_guard<mutex> lock{ tp_lock };
                tasks_list.emplace([task]() {
                    (*task)();
                });
            }
#ifdef THREADPOOL_AUTO_GROW
            if(thread_num < 1 && threads_list.size() < THREAD_POOL_MAX_SIZE)
                addThread(1);
#endif
            task_cv.notify_one();

            return future;
        }

        int rest_threads_count() { return thread_num; };
        int threads_count() { return threads_list.size(); }

    private:
        void addThread(unsigned short size) {
            if(!run)
                throw runtime_error("Grow on thread pool");
            for(; threads_list.size() < THREAD_POOL_MAX_SIZE && size > 0; --size)
            {
                threads_list.emplace_back([this] {
                    while(true)
                    {
                        Task task;
                        {
                            unique_lock<mutex> lock{ tp_lock };
                            task_cv.wait(lock, [this]{
                                return !run || !tasks_list.empty();
                            });
                            /* exit condition */
                            if(!run && tasks_list.empty())
                                return;
                            thread_num--;
                            task = std::move(tasks_list.front());
                            tasks_list.pop();
                        }
                        /* execute the task */
                        task();
                        /* return thread for next execution */
                        {
                            unique_lock<mutex> lock{ tp_lock };
                            thread_num++;
                        }
                    }
                });
            }
        }
    };

}
#endif//THREADPOOL_THREAD_POOL_H
