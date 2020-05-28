#include "workqueue.h"
#include "downlink.h"

#include <condition_variable>
#include <iostream>
#include <thread>


static std::mutex pending_queues_mutex;
static std::queue<std::shared_ptr<WorkQueue>> pending_queues;
static std::condition_variable pending_queues_not_empty;


void WorkQueue::pump() {
	std::unique_lock<std::mutex> pending_queues_lock(pending_queues_mutex);
    std::thread::id this_id = std::this_thread::get_id();
    for (;;) {
		while (pending_queues.empty()) {
			pending_queues_not_empty.wait(pending_queues_lock);
		}
		if (elog.trace())
            std::clog << "Thread(" << this_id << ") b4 move pending_queues size=" << pending_queues.size() << std::endl << std::flush;
		auto ptr = std::move(pending_queues.front());
		pending_queues.pop();
        if (elog.trace())
            std::clog << "Thread(" << this_id << ") after pop pending_queues size=" << pending_queues.size() << std::endl << std::flush;
		pending_queues_lock.unlock();
		std::unique_lock<std::mutex> work_queue_lock(ptr->work_queue_mutex);
		auto &work = ptr->work_queue.front();
		work_queue_lock.unlock();
		work();
		work_queue_lock.lock();
		ptr->work_queue.pop();
		bool still_pending = !ptr->work_queue.empty();
		work_queue_lock.unlock();
		pending_queues_lock.lock();
		if (still_pending) {
            if (elog.trace())
                std::clog << "Thread(" << this_id << ") b4 still pending_queues size=" << pending_queues.size() << std::endl << std::flush;
			pending_queues.push(std::move(ptr));
            if (elog.trace())
                std::clog << "Thread(" << this_id << ") still pending_queues size=" << pending_queues.size() << std::endl << std::flush;
		}
	}
}

void WorkQueue::pump_1_queue() {
    std::unique_lock<std::mutex> pending_queues_lock(pending_queues_mutex);
    std::thread::id this_id = std::this_thread::get_id();
    for (;;) {
        while (pending_queues.empty()) {
            pending_queues_not_empty.wait(pending_queues_lock);
        }
        if (elog.trace())
            std::clog << "Thread(" << this_id << ") b4 move pending_queues size=" << pending_queues.size() << std::endl << std::flush;
        auto ptr = std::move(pending_queues.front());
        pending_queues.pop();
        if (elog.trace())
            std::clog << "Thread(" << this_id << ") after pop pending_queues size=" << pending_queues.size() << std::endl << std::flush;
        //pending_queues_lock.unlock();
        std::unique_lock<std::mutex> work_queue_lock(ptr->work_queue_mutex);
        auto &work = ptr->work_queue.front();
        //work_queue_lock.unlock();
        work();
        //work_queue_lock.lock();
        ptr->work_queue.pop();
        bool still_pending = !ptr->work_queue.empty();
        work_queue_lock.unlock();
        //pending_queues_lock.lock();
        if (still_pending) {
            if (elog.trace())
                std::clog << "Thread(" << this_id << ") b4 still pending_queues size=" << pending_queues.size() << std::endl << std::flush;
            pending_queues.push(std::move(ptr));
            if (elog.trace())
                std::clog << "Thread(" << this_id << ") still pending_queues size=" << pending_queues.size() << std::endl << std::flush;
        }
    }
}

void WorkQueue::enqueue(std::function<void (void) /* noexcept */> &&work) {
	bool newly_pending;
	{
		std::lock_guard<std::mutex> work_queue_lock(work_queue_mutex);
		newly_pending = work_queue.empty();
		work_queue.push(std::move(work));
	}
	if (newly_pending) {
		{
			std::lock_guard<std::mutex> pending_queues_lock(pending_queues_mutex);
			pending_queues.push(this->shared_from_this());
            if (elog.trace()) {
                std::thread::id this_id = std::this_thread::get_id();
                int nQueues = pending_queues.size();
                std::clog << "Thread(" << this_id << ") enqueue pending_queues size=" << nQueues << std::endl << std::flush;
            }
		}
		pending_queues_not_empty.notify_one();
	}
}

void WorkQueue::enqueue_1_queue(std::function<void (void) /* noexcept */> &&work) {
    {
        std::lock_guard<std::mutex> pending_queues_lock(pending_queues_mutex);
        if (pending_queues.empty()) {
            {
                std::lock_guard<std::mutex> work_queue_lock(work_queue_mutex);
                work_queue.push(std::move(work));
            }
            pending_queues.push(this->shared_from_this());
        }
        else {
            auto ptr = std::move(pending_queues.front());
            pending_queues.pop();
            std::lock_guard<std::mutex> work_queue_lock(ptr->work_queue_mutex);
            ptr->work_queue.push(std::move(work));
            pending_queues.push(std::move(ptr));
        }
    }
    pending_queues_not_empty.notify_one();
}
