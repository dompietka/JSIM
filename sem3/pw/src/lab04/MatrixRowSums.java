package lab04;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.IntBinaryOperator;

public class MatrixRowSums {
    private static final int N_ROWS = 10;
    private static final int N_COLUMNS = 100;


    private static IntBinaryOperator matrixDefinition = (row, col) -> {
        int a = 2 * col + 1;
        return (row + 1) * (a % 4 - 2) * a;
    };

    private static void printRowSumsSequentially() {
        for (int r = 0; r < N_ROWS; ++r) {
            int sum = 0;
            for (int c = 0; c < N_COLUMNS; ++c) {
                sum += matrixDefinition.applyAsInt(r, c);
            }
            System.out.println(r + " -> " + sum);
        }
    }

    private static void printRowSumsInParallel() throws InterruptedException {
        List<Thread> threads = new ArrayList<>();
        AtomicInteger[] rowSums = new AtomicInteger[N_ROWS];
        CountDownLatch[] rowSumLatch = new CountDownLatch[N_ROWS];
        for (int r = 0; r < N_ROWS; ++r) {
            rowSums[r] = new AtomicInteger(0);
            rowSumLatch[r] = new CountDownLatch(N_COLUMNS);
        }
        

        for (int c = 0; c < N_COLUMNS; ++c) {
            final int myColumn = c;
            threads.add(new Thread(() -> {
                for (int r = 0; r < N_ROWS; ++r) {
                    if (Thread.currentThread().isInterrupted()) {
                    rowSumLatch[r].countDown();
                    return;
                    }
                    int value = matrixDefinition.applyAsInt(r, myColumn);
                    rowSums[r].addAndGet(value);
                    rowSumLatch[r].countDown();
                }
                
                    
            }));
        }

        for (Thread t : threads) {
            t.start();
        }

        for (int r = 0; r < N_ROWS; ++r) {

            rowSumLatch[r].await(); 
            System.out.println(r + " -> " + rowSums[r].get());
        }

        try {
            for (Thread t : threads) {
                t.join();
            }
        } catch (InterruptedException e) {
            for (Thread t : threads) {
                t.interrupt();
            }
            throw e;
        }

        /*for (int r = 0; r < N_ROWS; ++r) {
            System.out.println(r + " -> " + rowSums[r].get());
        }*/
    }

    public static void main(String[] args) {
        try {
            System.out.println("-- Sequentially --");
            printRowSumsSequentially();
            System.out.println("-- In parallel --");
            printRowSumsInParallel();
            System.out.println("-- End --");
        } catch (InterruptedException e) {
            System.err.println("Main interrupted.");
        }
    }
}
