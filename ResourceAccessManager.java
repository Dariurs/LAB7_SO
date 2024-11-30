import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class ResourceAccessManager {
    private int whiteThreads = 0;
    private int blackThreads = 0;
    private int waitingWhite = 0;
    private int waitingBlack = 0;

    private final Lock lock = new ReentrantLock();
    private final Condition condition = lock.newCondition();

    public void enterWhite() throws InterruptedException {
        lock.lock();
        try {
            waitingWhite++;
            while (blackThreads > 0) {
                condition.await();
            }
            waitingWhite--;
            whiteThreads++;
            System.out.println(Thread.currentThread().getName() + " (white) is now using the resource. Active white threads: " + whiteThreads);
        } finally {
            lock.unlock();
        }
    }

    public void exitWhite() {
        lock.lock();
        try {
            whiteThreads--;
            System.out.println(Thread.currentThread().getName() + " (white) has finished using the resource. Remaining white threads: " + whiteThreads);
            if (whiteThreads == 0) {
                condition.signalAll();
            }
        } finally {
            lock.unlock();
        }
    }

    public void enterBlack() throws InterruptedException {
        lock.lock();
        try {
            waitingBlack++;
            while (whiteThreads > 0) {
                condition.await();
            }
            waitingBlack--;
            blackThreads++;
            System.out.println(Thread.currentThread().getName() + " (black) is now using the resource. Active black threads: " + blackThreads);
        } finally {
            lock.unlock();
        }
    }

    public void exitBlack() {
        lock.lock();
        try {
            blackThreads--;
            System.out.println(Thread.currentThread().getName() + " (black) has finished using the resource. Remaining black threads: " + blackThreads);
            if (blackThreads == 0) {
                condition.signalAll();
            }
        } finally {
            lock.unlock();
        }
    }

    public static void main(String[] args) {
        ResourceAccessManager manager = new ResourceAccessManager();

        Runnable whiteTask = () -> {
            try {
                manager.enterWhite();
                Thread.sleep(1000);
                manager.exitWhite();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        };

        Runnable blackTask = () -> {
            try {
                manager.enterBlack();
                Thread.sleep(1000);
                manager.exitBlack();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        };

        for (int i = 0; i < 5; i++) {
            new Thread(whiteTask, "WhiteThread-" + i).start();
            new Thread(blackTask, "BlackThread-" + i).start();
        }
    }
}
