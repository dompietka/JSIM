import java.util.List;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.Collections;

public class Main {
    public static void traverseGraph(Graph g, Queue q, int k){
        q.add(k);
        List<Integer> answer = new ArrayList<Integer>();
        boolean[] visited = new boolean[g.n];

        visited[k] = true;
        while(q.size() > 0){
            int x = q.get();
            answer.add(x);
            List<Integer> adjacent = g.adjacent(x);
            Collections.sort(adjacent);
            if (q instanceof QueueLIFO) {
                Collections.reverse(adjacent);
            }
            for(int v : adjacent){
                if(!visited[v]){
                    visited[v] = true;
                    q.add(v);
                }
            }
        }
        System.out.println(answer);
    }

    public static void main(String[] args){
        Scanner sc = new Scanner(System.in);
        int n = sc.nextInt();
        int m = sc.nextInt();
        Graph g_matrix = new MatrixGraph(n);
        Graph g_list = new ListGraph(n);
        for(int i = 0; i < m; i++){
            int a = sc.nextInt();
            int b = sc.nextInt();
            g_list.add(a, b);
            g_matrix.add(a, b);
        }

        QueueFIFO qFIFO = new QueueFIFO();
        QueueLIFO qLIFO = new QueueLIFO();
        System.out.println("MatrixGraph, DFS");
        traverseGraph(g_matrix, qLIFO, 0);
        System.out.println("MatrixGraph, BFS");
        traverseGraph(g_matrix, qFIFO, 0);
        System.out.println("ListGraph, DFS");
        traverseGraph(g_list, qLIFO, 0);
        System.out.println("ListGraph, BFS");
        traverseGraph(g_list, qFIFO, 0);
        sc.close();
    }
}
