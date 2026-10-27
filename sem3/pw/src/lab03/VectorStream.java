package lab03;

import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.function.IntBinaryOperator;

public class VectorStream {
    private static final int STREAM_LENGTH = 10;
    private static final int VECTOR_LENGTH = 100;

    /**
     * Function that defines how vectors are computed: the i-th element depends on
     * the previous sum and the index i.
     * The sum of elements in the previous vector is initially given as zero.
     */
    private final static IntBinaryOperator vectorDefinition = (previousSum, i) -> {
        int a = 2 * i + 1;
        return (previousSum / VECTOR_LENGTH + 1) * (a % 4 - 2) * a + 1;
    };

    private static void computeVectorStreamSequentially() {
        int[] vector = new int[VECTOR_LENGTH];
        int sum = 0;
        for (int vectorNo = 0; vectorNo < STREAM_LENGTH; ++vectorNo) {
            for (int i = 0; i < VECTOR_LENGTH; ++i) {
                vector[i] = vectorDefinition.applyAsInt(sum, i);
            }
            sum = 0;
            for (int x : vector) {          
                sum += x;
            }
            System.out.println(vectorNo + " -> " + sum);
        }
    }

    public static void calculateVectorSum(int vectorNo, int[] vector, int[] sums) {
        int sum = 0;
        for (int x : vector) {
            sum += x;
        }

    }

static class VectorWorker implements Runnable {
        private final int index;
        private final int[] vector;
        private final int[] prev_sum;
        private final CyclicBarrier barrier;
        private final int streamLength;

        VectorWorker(int index, int[] vector, int[] prev_sum, CyclicBarrier barrier, int streamLength) {
            this.index = index;
            this.vector = vector;
            this.prev_sum = prev_sum;
            this.barrier = barrier;
            this.streamLength = streamLength;
        }

        @Override
        public void run() {
            try {
                for (int vectorNo = 0; vectorNo < streamLength; vectorNo++) {
                    // obliczamy wartość elementu wektora
                    vector[index] = vectorDefinition.applyAsInt(prev_sum[0], index);

                    // czekamy na pozostałe wątki i potem barrier action
                    barrier.await();
                }
            } catch (InterruptedException | BrokenBarrierException e) {
                Thread.currentThread().interrupt();
            }
        }
    }


   private static void computeVectorStreamInParallel() throws InterruptedException {
        int[] vector = new int[VECTOR_LENGTH];
        int[] prev_sum = {0}; // przechowuje sumę poprzedniego wektora
        int[] vector_nr = {0}; //to są tablice jednoelementowe, 
        //żeby można było je modyfikować w klasie wewnętrznej, 
        //wtedy "final" sie tyczy wskaznika na tablice, a nie jej zawartosci

        final class BarrierAction implements Runnable {
        

        @Override
        public void run() {
            int sum = 0;
            for (int x : vector) {
                sum += x;
            }
            System.out.println(vector_nr[0] + " -> " + sum);
            prev_sum[0] = sum;
            vector_nr[0]++;
        }
        //akcja bariery wykonuje sie jako jedyny watek, wiec nie ma problemu z dostepem do wektora i sumy
    }

        // Barrier action sumuje wektor i wypisuje wynik
        CyclicBarrier barrier = new CyclicBarrier(VECTOR_LENGTH, new BarrierAction());

        Thread[] threads = new Thread[VECTOR_LENGTH];
        for (int i = 0; i < VECTOR_LENGTH; i++) {
            threads[i] = new Thread(new VectorWorker(i, vector, prev_sum, barrier, STREAM_LENGTH));
            threads[i].start();
        }

        for (Thread t : threads) {
            t.join();
        }
    }

    public static void main(String[] args) {
        
        try {
            System.out.println("-- Sequentially --");
            computeVectorStreamSequentially();
            System.out.println("-- Parallel --");
            computeVectorStreamInParallel();
            System.out.println("-- End --");
        } catch (InterruptedException e) {
            System.err.println("Main interrupted.");
        }
    }
}
