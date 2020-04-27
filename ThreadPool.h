#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

// 线程池类
class ThreadPool {
public:
	// 构造函数，传入线程数
	ThreadPool(size_t); 
	// 任务入队，传入函数和函数参数。这是可变参数的模板，采用了尾指返回类型
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args)
		->std::future<typename std::result_of<F(Args...)>::type>;
	// 析构
	~ThreadPool();
private:
	// 工作线程组
	// need to keep track of threads so we can join them
	std::vector< std::thread > workers;
	//任务队列
	// the task queue
	std::queue< std::function<void()> > tasks;

	// synchronization 同步化
	std::mutex queue_mutex; // 队列互斥锁
	std::condition_variable condition; // 条件变量
	bool stop; // 线程池停止标志
};

// 构造函数仅启动一些工作线程
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
	: stop(false)
{
	for (size_t i = 0; i < threads; ++i)
		workers.emplace_back( // emplace_back和push_back的作用相同，性能更好，避免额外类的复制和移动操作
			[this] // lambda表达式
			{
				for (;;) // 死循环，线程池中的线程不断从任务队列取任务
				{
					std::function<void()> task; // 一个包装可调用函数的类，被包裹的函数void()，返回值为void，参数为void

					// {...}程序块
					{
						// 上锁，lock离开程序块，自动释放锁
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						// 等待条件成立
						this->condition.wait(lock,
							[this] { return this->stop || !this->tasks.empty(); });
						// 当前线程被阻塞等待，直到被通知
						// 在阻塞线程时，该函数会自动调用lock.unlock()，从而允许其他锁定的线程继续执行
						// 一旦被其他线程notified，再次上锁lock.lock()
						// 然后进行条件判断（也就是lambda中的条件判断）
						// 如果[this->stop || !this->tasks.empty()]的结果为false，继续阻塞，为true，解除阻塞

						// 如果线程池停止并且任务队列为空，结束返回
						if (this->stop && this->tasks.empty())
							return;
						// 取任务队列的头部（首任务），move给task
						task = std::move(this->tasks.front());
						// 从队列弹出
						this->tasks.pop();
					}
					// 执行取出的任务
					task();
				}
			}
		);
}

// 添加新的工作任务到线程池。这是可变参数的模板，采用了尾指返回类型
// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type; // using使用别名

	// 将任务函数和其参数绑定，构建一个packaged_task，最后返回shared_ptr指针给task
	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

	// 获取任务的future，也就是任务的返回值
	std::future<return_type> res = task->get_future();
	{ // {}代码块
		// 上锁
		std::unique_lock<std::mutex> lock(queue_mutex);

		// 线程池停止后，不允许入队操作
		// don't allow enqueueing after stopping the pool
		if (stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		// 将任务添加到任务队列tasks中
		tasks.emplace([task]() { (*task)(); });
	}
	// 发送通知，解除某个正被此条件阻塞的线程
	condition.notify_one();
	return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	{ // {} 代码块
		// 上锁
		std::unique_lock<std::mutex> lock(queue_mutex);
		// 标志位stop设置为true
		stop = true;
	}
	// 通知所有线程，唤醒线程后，因为stop为true，都会被结束
	condition.notify_all();
	// 将所有的线程join，线程会被安全地销毁
	for (std::thread &worker : workers)
		worker.join();
}

#endif
