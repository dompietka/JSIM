import java.util.List;
import java.util.ArrayList;

public class MatrixGraph extends Graph {
    private boolean[][] adj;

    public MatrixGraph(int n) {
        this.n = n;
        adj = new boolean[n][n];
    }

    @Override
    public void add(int a, int b) {
        adj[a][b] = true;
    }

    @Override
    public void remove(int a, int b) {
        assert(adj[a][b]) : "Edge does not exist";
        adj[a][b] = false;
    }

    @Override
    public List<Integer> adjacent(int a) {
        List<Integer> list = new ArrayList<>();
        for (int i = 0; i < n; i++) {
            if (adj[a][i]) {
                list.add(i);
            }
        }
        return list;
    }
}