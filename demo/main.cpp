#include <abb.h>
#include <thread>
#include <map>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sstream>

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

VoidBlock wait(Duration dur) {
    struct Helpers {
        static void wait(abb::Island & island, abb::Answer<void()> & answer, Duration dur1) {
            island.increfExternal();
            Timer::current().schedule(std::bind(&Helpers::finish, std::ref(island), std::ref(answer)), dur1);
        }

        static void finish(abb::Island & island, abb::Answer<void()> & answer) {
            island.enqueueExternal(std::bind(&abb::Answer<void()>::setResult, &answer));
            island.decrefExternal();
        }
    };

    abb::Island & island = abb::Island::current();

    return abb::impl<VoidBlock>(std::bind(&Helpers::wait, std::ref(island), std::placeholders::_1, dur));
};

IntBlock increment(int val) {
    LOG("increment(" << val << ")");
    return abb::success(val + 1);
}

void display(std::string const & msg) {
    std::cerr << msg << std::endl;
}

void putFive(abb::Answer<void(int)> & answer) {
    answer.setResult(5);
}

#define STRING(val) ({ std::stringstream __ss; __ss << val; __ss.str(); })

class DoSthLoop {
public:
    DoSthLoop(): i(0) {}

    VoidBlock operator()() {
        if (this->i++ < 5) {
            return wait(
                std::chrono::milliseconds(200)
            ).pipe(
                std::bind(&display, STRING("DoSthLoop " << this->i))
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

    abb::run(DoSthLoop());
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

/*
function doSth() {
    var i = 0;

    return (function go() {
        return i++ < 5 ? wait(1000).pipe(display.bind(null, "aaa")).pipe(go) : success()
    })(go);
}
*/

int main() {
    Timer timer;

    abb::Island island;
    island.enqueueExternal(&doSth);
    island.enqueueExternal(&doSthWithErrors);
    island.enqueueExternal(&doSthOnErrors);
    island.run();

    return 0;
}
