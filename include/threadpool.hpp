#ifndef CA15F37A_BEAC_4F9A_B785_4A98573C4095
#define CA15F37A_BEAC_4F9A_B785_4A98573C4095

#include <functional>
#include <future>
#include <thread>
#include <concurrentequeue.hpp>

namespace mc
{
    //  Thread pool of chapter 3 - aad savine
    typedef std::packaged_task<bool(void)> Task;
    typedef std::future<bool> TaskHandle;

    class ThreadPool
    {
        //	The one and only instance
        static ThreadPool myInstance;

        //	The task queue
        ConcurrentQueue<Task> myQueue;

        //	The threads
        std::vector<std::thread> myThreads;

        //  Active indicator
        bool myActive;

        //	Interruption indicator
        bool myInterrupt;

        //	Thread number
        static thread_local size_t myTLSNum;

        //	The function that is executed on every std::thread
        void threadFunc(const size_t num)
        {
            myTLSNum = num;

            Task t;

            //	"Infinite" loop, only broken on destruction
            while (!myInterrupt)
            {
                //	Pop and executes tasks
                myQueue.pop(t);
                if (!myInterrupt)
                    t();
            }
        }

        //  The constructor stays private, ensuring single instance
        ThreadPool() : myActive(false), myInterrupt(false) {}

    public:
        //	Access the instance
        static ThreadPool *getInstance() { return &myInstance; }

        //	Number of threads
        size_t numThreads() const { return myThreads.size(); }

        //	The number of the caller std::thread
        static size_t threadNum() { return myTLSNum; }

        //	Starter
        void start(const size_t nThread = std::thread::hardware_concurrency() - 1)
        {
            if (!myActive) //  Only start once
            {
                myThreads.reserve(nThread);

                //	Launch threads on threadFunc and keep handles in a std::vector
                for (size_t i = 0; i < nThread; i++)
                    myThreads.push_back(std::thread(&ThreadPool::threadFunc, this, i + 1));

                myActive = true;
            }
        }

        //	Destructor
        ~ThreadPool() { stop(); }

        void stop()
        {
            if (myActive)
            {
                //	Interrupt mode
                myInterrupt = true;

                //	Interrupt all waiting threads
                myQueue.interrupt();

                //	Wait for them all to join
                for_each(myThreads.begin(), myThreads.end(), std::mem_fn(&std::thread::join));

                //  Clear all threads
                myThreads.clear();

                //  Clear the queue and reset interrupt
                myQueue.clear();
                myQueue.resetInterrupt();

                //  Mark as inactive
                myActive = false;

                //  Reset interrupt
                myInterrupt = false;
            }
        }

        //	Forbid copies etc
        ThreadPool(const ThreadPool &rhs) = delete;

        ThreadPool &operator=(const ThreadPool &rhs) = delete;

        ThreadPool(ThreadPool &&rhs) = delete;

        ThreadPool &operator=(ThreadPool &&rhs) = delete;

        //	Spawn task
        template <typename Callable>
        TaskHandle spawnTask(Callable c)
        {
            Task t(std::move(c));
            TaskHandle f = t.get_future();
            myQueue.push(std::move(t));
            return f;
        }

        //	Run queued tasks synchronously
        //	while waiting on a future,
        //	return true if at least one task was run
        bool activeWait(const TaskHandle &f)
        {
            Task t;
            bool b = false;

            //	Check if the future is ready without blocking
            //	The only syntax C++11 provides for that is
            //	wait 0 seconds and return status
            while (f.wait_for(std::chrono::duration<int, std::deci>::zero()) != std::future_status::ready)
            {
                //	Non blocking
                if (myQueue.tryPop(t))
                {
                    t();
                    b = true;
                }
                else //	Nothing in the queue: go to sleep
                {
                    f.wait();
                }
            }

            return b;
        }
    };
} // namespace mc

#endif /* CA15F37A_BEAC_4F9A_B785_4A98573C4095 */
