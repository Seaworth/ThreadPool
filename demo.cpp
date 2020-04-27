#include <iostream>
#include "ThreadPool.h"

void func()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "worker thread ID:" << std::this_thread::get_id() << std::endl;
}

int main()
{
	ThreadPool pool(4);
	int i = 20;
	while (i--)
	{
		pool.enqueue(func);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	system("pause"); // 如果编译器不是VS，则注释该行
	return 0;
}
