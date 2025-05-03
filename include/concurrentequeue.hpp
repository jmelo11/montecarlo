#ifndef F34F0A7B_82E8_4370_836D_F185E60A0BAB
#define F34F0A7B_82E8_4370_836D_F185E60A0BAB

#include <queue>
#include <mutex>
#include <condition_variable>

namespace mc
{
    /**
     * @brief A concurrent queue. Taken from A. Savine's book, AAD and parallel simulation.
     *
     */
    template <class T>
    class ConcurrentQueue
    {
        std::queue<T> myQueue;
        mutable std::mutex myMutex;
        std::condition_variable myCV;
        bool myInterrupt;

    public:
        ConcurrentQueue() : myInterrupt(false) {}
        ~ConcurrentQueue() { interrupt(); }

        bool empty() const
        {
            //	Lock
            std::lock_guard<std::mutex> lk(myMutex);
            //	Access underlying queue
            return myQueue.empty();
        } //	Unlock

        //	Pop into argument
        bool tryPop(T &t)
        {
            //	Lock
            std::lock_guard<std::mutex> lk(myMutex);
            if (myQueue.empty())
                return false;
            //	Move from queue
            t = std::move(myQueue.front());
            //	Combine front/pop
            myQueue.pop();

            return true;
        } //	Unlock

        //	Pass t byVal or move with push( move( t))
        void push(T t)
        {
            {
                //	Lock
                std::lock_guard<std::mutex> lk(myMutex);
                //	Move into queue
                myQueue.push(std::move(t));
            } //	Unlock before notification

            //	Unlock before notification
            myCV.notify_one();
        }

        //	Wait if empty
        bool pop(T &t)
        {
            //	(Unique) lock
            std::unique_lock<std::mutex> lk(myMutex);

            //	Wait if empty, release lock until notified
            while (!myInterrupt && myQueue.empty())
                myCV.wait(lk);

            //	Re-acquire lock, resume

            //  Check for interruption
            if (myInterrupt)
                return false;

            //	Combine front/pop
            t = std::move(myQueue.front());
            myQueue.pop();

            return true;

        } //	Unlock

        void interrupt()
        {
            {
                std::lock_guard<std::mutex> lk(myMutex);
                myInterrupt = true;
            }
            myCV.notify_all();
        }

        void resetInterrupt() { myInterrupt = false; }

        void clear()
        {
            std::queue<T> empty;
            std::swap(myQueue, empty);
        }
    };
}

#endif /* F34F0A7B_82E8_4370_836D_F185E60A0BAB */
