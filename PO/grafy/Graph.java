import java.util.List;
import java.util.ArrayList;

public abstract class Graph{
    protected int n;
    public abstract void add(int a, int b);
    public abstract void remove(int a, int b);
    public abstract List<Integer> adjacent(int a);
}