#include <abb.h>
#include <thread>
#include <map>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <iostream>

#define LOG(msg) do { std::cerr << "File " << __FILE__ << ", line " << __LINE__ << ": " << msg << std::endl; } while(0)

typedef std::chrono::system_clock::duration Duration;

class Timer {
public:
    Timer(): stopping(false) {
        Timer::currentTimerPtr = this;
        std::thread thread(std::bind(&Timer::run, this));
        this->thread.swap(thread);
    }

    ~Timer() {
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->stopping = true;
            this->condition.notify_all();
        }
        this->thread.join();
    }

    void schedule(std::function<void()> task, std::chrono::system_clock::duration dur) {
        std::unique_lock<std::mutex> lock(this->mutex);
        this->tasks.insert(std::make_pair(std::chrono::system_clock::now() + dur, task));
        this->condition.notify_all();
    }

    static Timer & current() {
        return *Timer::currentTimerPtr;
    }

private:
    void run() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(this->mutex);
                while (!this->stopping && this->tasks.empty()) {
                    this->condition.wait(lock);
                }
                if (this->tasks.empty()) {
                    return;
                }
                while (this->tasks.begin()->first > std::chrono::system_clock::now()) {
                    this->condition.wait_until(lock, this->tasks.begin()->first);
                }

                task = this->tasks.begin()->second;
                this->tasks.erase(this->tasks.begin());
            }
            task();
        }
    }

    static Timer * currentTimerPtr;

    std::thread thread;
    std::mutex mutex;
    std::condition_variable condition;
    std::multimap<std::chrono::system_clock::time_point, std::function<void()>> tasks;
    bool stopping;
};

Timer * Timer::currentTimerPtr = nullptr;


typedef abb::Block<void(int)> IntBlock;
typedef abb::Block<void()> VoidBlock;
typedef VoidBlock::ReplyType VoidReply;

VoidBlock wait(Duration dur) {
    struct Helpers {
        static void wait(VoidReply & reply, Duration dur1) {
            if (reply.isAborted()) {
                LOG("wait reply aborted, continue anyway");
            }
            Timer::current().schedule(std::bind(&Helpers::finish, std::ref(reply)), dur1);
        }

        static void finish(VoidReply & reply) {
            reply.getIsland().enqueueExternal(std::bind(&VoidReply::setResult, &reply));
        }
    };

    return abb::impl<VoidBlock>(std::bind(&Helpers::wait, std::placeholders::_1, dur));
};

IntBlock increment(int val) {
    LOG("increment(" << val << ")");
    return abb::success(val + 1);
}

void display(std::string const & msg) {
    std::cerr << msg << std::endl;
}

void putFive(abb::Reply<int> & answer) {
    answer.setResult(5);
}

#define STRING(val) ({ std::stringstream __ss; __ss << val; __ss.str(); })

class Countdown {
public:
    explicit Countdown(int value): i(value) {}

    ~Countdown() {
        this->i = 666;
    }

    VoidBlock operator()() {
        display(STRING("DoSthLoop " << this->i));
        if (this->i > 0) {
            this->i--;
            return wait(
                std::chrono::milliseconds(1000)
            ).pipe(
                std::ref(*this)
            );
        } else {
            return abb::success();
        }
    }

private:
    int i;
};

typedef abb::Block<abb::Und, void(std::string)> StringErrorBlock;

StringErrorBlock notFound() {
    return abb::error<StringErrorBlock>("not found");
}

void doSth() {
    abb::impl<IntBlock>(&putFive).pipe(&increment).pipe(&increment);
}

void doSthWithErrors() {
    typedef abb::Block<void(int), void(std::string)> BlockType;

    struct Funs {
        static BlockType inc(int i) {
            LOG("inc(" << i << ")");
            return abb::success<BlockType>(i + 1);
        }
        static BlockType suppress(std::string msg) {
            LOG("suppress(" << msg << ")");
            return abb::success<BlockType>(0);
        }
    };

    abb::error<BlockType>("bad")
        .pipe(&Funs::inc, &Funs::suppress)
        .pipe(&Funs::inc, &Funs::suppress)
        .pipe(&Funs::inc)
        .pipe(abb::pass, &Funs::suppress);
}

void doSthOnErrors() {
    typedef abb::Block<void(int)> BlockType;

    struct Funs {
        static BlockType inc(int i) {
            LOG("inc(" << i << ")");
            return abb::success(i + 1);
        }
    };

    abb::success(1).pipe(&Funs::inc, abb::pass);
}

struct NoncopyableIncrement {
    NoncopyableIncrement() {}
    NoncopyableIncrement(NoncopyableIncrement const&) = delete;
    NoncopyableIncrement(NoncopyableIncrement &&) = default;

    IntBlock operator()(int value) {
        return abb::success(value + 1);
    }
};

void doSthNoncopyableIncrement() {
    abb::success(10).pipe(NoncopyableIncrement());
}

typedef std::unique_ptr<int> UniqueInt;
typedef abb::Block<void(UniqueInt)> UniqueIntBlock;

void doSthWithUniqueInt() {
    struct Funs {
        static UniqueIntBlock inc(UniqueInt i) {
            (*i)++;
            return abb::success(std::move(i));
        }
    };
    abb::success(std::unique_ptr<int>(new int(5))).pipe(&Funs::inc);
}

VoidBlock doSthWait() {
    return wait(std::chrono::milliseconds(200)).pipe([]() {
        LOG("hello");
    });
}

int main() {
    Timer timer;

    abb::Island island;
    island.enqueueExternal(abb::make<Countdown>(10));
//    island.enqueueExternal(&doSth);
//    island.enqueueExternal(&doSthWithErrors);
//    island.enqueueExternal(&doSthOnErrors);
    island.run();

    return 0;
}
