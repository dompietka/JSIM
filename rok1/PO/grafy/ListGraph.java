import java.util.List;
import java.util.ArrayList;

public class ListGraph extends Graph {
    private List<Integer>[] adj;

    public ListGraph(int n) {
         this.n = n;
        adj = (List<Integer>[]) new ArrayList[n]; 
        for (int i = 0; i < n; i++) {
            adj[i] = new ArrayList<>();
        }
    }

    @Override
    public void add(int a, int b) {
            adj[a].add(b);
    }

    private int isEdge(int a, int b) {
        for (int i = 0; i < adj[a].size(); i++) {
            if (adj[a].get(i) == b) {
                return i;
            }
        }
        return -1;
    }

    @Override
    public void remove(int a, int b) {
        int i = isEdge(a, b);
        assert(i!=-1) : "Edge does not exist";
        adj[a].remove(i);
    }

    @Override
    public List<Integer> adjacent(int a) {
        return adj[a];
    }
}