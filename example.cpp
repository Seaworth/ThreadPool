#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int main()
{
	// 创建线程池，里面有四个工作线程
	ThreadPool pool(4);
	// 通过future拿到每个工作线程运行完的结果，保存在results中
	std::vector< std::future<int> > results;

	// 将8个任务压入任务队列，线程池中的四个工作线程会不断地取任务
	// 任务执行完的结果压入results容器中
	// 8个任务函数是通过lambda表达式来实现的
	for (int i = 0; i < 8; ++i) {
		results.emplace_back(
			pool.enqueue([i] {
				std::cout << "hello " << i << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				std::cout << "world " << i << std::endl;
				return i * i;
			})
		);
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// 打印8个任务的结果
	for (auto && result : results)
		std::cout << result.get() << ' ';
	std::cout << std::endl;

	system("pause"); // 如果不是采用的VS编译器，注释该行
	return 0;
}
