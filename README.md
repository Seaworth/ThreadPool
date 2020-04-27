线程池ThreadPool
==========

用`C++11`来实现一个简单的线程池。

基本用法:
```c++
// 创建带有4个工作线程的线程池
// create thread pool with 4 worker threads
ThreadPool pool(4);

// 将任务压入工作队列，线程处理后的结果保存在future中
// enqueue and store future
auto result = pool.enqueue([](int answer) { return answer; }, 42);

// 从future中获得线程处理后的结果
// get result from future
std::cout << result.get() << std::endl;

```

源码中用到了许多`C++11`的特性，需要了解以下的知识点，能够更清晰地理解实现的过程：
- using定义别名
- lambda表达式
- 模板、可变参数模板
- 尾置返回类型
- std::chrono
- std::mutex
- std::bind
- std::function -  [官方文档](http://www.cplusplus.com/reference/functional/function/)
- std::unique_lock -  [官方文档](http://www.cplusplus.com/reference/mutex/unique_lock/?kw=unique_lock)
- std::condition_variable -  [官方文档](http://www.cplusplus.com/reference/condition_variable/condition_variable/?kw=condition_variable)
- std::forward -  [官方文档](http://www.cplusplus.com/reference/utility/forward/)，[讲解C++11左值，右值，左值引用，右值引用的博客](https://blog.csdn.net/xiaolewennofollow/article/details/52559306)
- std::packaged_task -  [官方文档](http://www.cplusplus.com/reference/future/packaged_task/)

一些有参考意义的博客：
1. [C++11的简单线程池代码阅读](https://www.cnblogs.com/oloroso/p/5881863.html)
2. [分享一个C++11写的线程池](https://blog.csdn.net/wangshubo1989/article/details/51254886)
