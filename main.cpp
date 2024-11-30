#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>

class ResourceAccessManager {
private:
    int whiteThreads = 0;
    int blackThreads = 0;
    int waitingWhite = 0;
    int waitingBlack = 0;

    CRITICAL_SECTION lock;
    CONDITION_VARIABLE condition;

public:
    ResourceAccessManager() {
        InitializeCriticalSection(&lock);
        InitializeConditionVariable(&condition);
    }

    ~ResourceAccessManager() {
        DeleteCriticalSection(&lock);
    }

    void enterWhite() {
        EnterCriticalSection(&lock);
        waitingWhite++;
        while (blackThreads > 0) {
            SleepConditionVariableCS(&condition, &lock, INFINITE);
        }
        waitingWhite--;
        whiteThreads++;
        std::cout << "Thread " << GetCurrentThreadId() << " (white) is now using the resource. Active white threads: " << whiteThreads << std::endl;
        LeaveCriticalSection(&lock);
    }

    void exitWhite() {
        EnterCriticalSection(&lock);
        whiteThreads--;
        std::cout << "Thread " << GetCurrentThreadId() << " (white) has finished using the resource. Remaining white threads: " << whiteThreads << std::endl;
        if (whiteThreads == 0) {
            WakeAllConditionVariable(&condition);
        }
        LeaveCriticalSection(&lock);
    }

    void enterBlack() {
        EnterCriticalSection(&lock);
        waitingBlack++;
        while (whiteThreads > 0) {
            SleepConditionVariableCS(&condition, &lock, INFINITE);
        }
        waitingBlack--;
        blackThreads++;
        std::cout << "Thread " << GetCurrentThreadId() << " (black) is now using the resource. Active black threads: " << blackThreads << std::endl;
        LeaveCriticalSection(&lock);
    }

    void exitBlack() {
        EnterCriticalSection(&lock);
        blackThreads--;
        std::cout << "Thread " << GetCurrentThreadId() << " (black) has finished using the resource. Remaining black threads: " << blackThreads << std::endl;
        if (blackThreads == 0) {
            WakeAllConditionVariable(&condition);
        }
        LeaveCriticalSection(&lock);
    }
};

void whiteTask(ResourceAccessManager& manager) {
    manager.enterWhite();
    Sleep(1000); 
    manager.exitWhite();
}

void blackTask(ResourceAccessManager& manager) {
    manager.enterBlack();
    Sleep(1000); 
    manager.exitBlack();
}

int main() {
    ResourceAccessManager manager;
    std::vector<std::thread> threads;

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(whiteTask, std::ref(manager));
        threads.emplace_back(blackTask, std::ref(manager));
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
