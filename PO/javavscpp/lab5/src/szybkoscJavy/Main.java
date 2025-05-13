package szybkoscJavy;
import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        long startTime = System.nanoTime();

        int n = scanner.nextInt();
        long[][] values = new long[n][n];
        for (int i = 0; i < n * n; i++) {
            values [i/n][i%n] = scanner.nextInt();
        }

        Graph G = new Graph(n, values);

        long trianglesInG = G.countTriangles();
        long endTime = System.nanoTime();
        System.out.println("czas w ms: " + (endTime - startTime)/1000000);
    }
}
