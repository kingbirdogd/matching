#ifndef COMMON_WORKQUEUE_H
#define COMMON_WORKQUEUE_H

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "common/compiler.h"
#include <iostream>

/**
 * @brief Holds a queue of work items and arranges for serial execution of those work items.
 *
 * Although enqueued work items are not guaranteed to execute in any particular worker thread,
 * each enqueued work item is guaranteed to have finished executing before any other work item enqueued subsequently to the same work queue may begin executing,
 * and memory accesses by any given work item *happen before* memory accesses by any other work item enqueued subsequently to the same work queue.
 *
 * At least one thread per process must call pump() to execute enqueued work items.
 *
 * Each instance of this class \b MUST be owned by a \c std::shared_ptr.
 * The internal logic will then ensure that WorkQueue instances are not destroyed until all work items enqueued therein have been dequeued.
 */
class WorkQueue : public std::enable_shared_from_this<WorkQueue> {

public:
	/**
	 * @brief Dequeues and executes work items enqueued via enqueue().
	 *
	 * At least one thread per process must call this function to execute enqueued work items.
	 *
	 * This function dequeues from WorkQueue instances in a round-robin manner.
	 *
	 * **This function never returns.**
	 * The calling thread is permanently consigned to executing work items.
	 */
	_noreturn static void pump();
    _noreturn static void pump_1_queue();
  ~WorkQueue() {
    std::cout << "Destruct WorkQueue" << std::endl;
  }

private:
	std::mutex work_queue_mutex;
	std::queue<std::function<void (void) /* noexcept */>> work_queue;

public:
	/**
	 * @brief Enqueues a work item in this queue.
	 */
	void enqueue(std::function<void (void) /* noexcept */> &&work);
    void enqueue_1_queue(std::function<void (void) /* noexcept */> &&work);

};
#endif