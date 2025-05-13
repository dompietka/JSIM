import java.util.List;
import java.util.ArrayList;

public abstract class Queue {
    protected List<Integer> q;

    public Queue() {
        q = new ArrayList<>();
    }
    public int size() {
        return q.size();
    }

    public abstract void add(int x);
    public abstract int get();
}