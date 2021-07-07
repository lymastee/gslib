/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the gslib project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef thdpool_3a298e4f_792c_44ff_bc62_c639d1eb697c_h
#define thdpool_3a298e4f_792c_44ff_bc62_c639d1eb697c_h

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <gslib/std.h>

__gslib_begin__

using std::thread;
using std::function;

class thread_pool
{
public:
    thread_pool(int threads)
    {
        for(int i = 0; i < threads; i ++) {
            _workers.emplace_back(
                [this]() {
                    for(;;) {
                        function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(_mtx_worker);
                            _cv_worker.wait(lock, [this] { return _stop || !_tasks.empty(); });
                            if(_stop && _tasks.empty())
                                return;
                            task = std::move(_tasks.front());
                            _tasks.pop();
                        }
                        task();
                        {
                            std::unique_lock<std::mutex> lock(_mtx_main);
                            -- _undone;
                        }
                        _cv_main.notify_one();
                    }
                }
            );
        }
    }
    ~thread_pool()
    {
        {
            std::unique_lock<std::mutex> lock(_mtx_worker);
            _stop = true;
        }
        _cv_worker.notify_all();
        for(thread& worker : _workers)
            worker.join();
    }

protected:
    vector<thread>          _workers;
    queue<function<void()>> _tasks;
    std::mutex              _mtx_worker;
    std::mutex              _mtx_main;
    std::condition_variable _cv_worker;
    std::condition_variable _cv_main;
    volatile bool           _stop = false;
    volatile int            _undone = 0;

public:
    template<class _func, class... _args>
    auto add(_func&& f, _args&&... args) ->
        std::future<typename std::result_of<_func(_args...)>::type>
    {
        using return_type = typename std::result_of<_func(_args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<_func>(f), std::forward<_args>(args)...)
            );
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(_mtx_worker);
            if(_stop)
                throw std::runtime_error("enqueue on stopped thread pool.");
            _tasks.emplace([task]() { (*task)(); });
        }
        {
            std::unique_lock<std::mutex> lock(_mtx_main);
            ++ _undone;
        }
        _cv_worker.notify_one();
        return res;
    }
    void join()
    {
        std::unique_lock<std::mutex> lock(_mtx_main);
        _cv_main.wait(lock, [this] { return !_undone; });
    }
};

__gslib_end__

#endif

