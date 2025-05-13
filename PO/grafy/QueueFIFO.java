import java.util.List;
import java.util.ArrayList;

public class QueueFIFO extends Queue {

    public QueueFIFO() {
        super();
    }
    
    @Override
    public void add(int x) {
        q.add(x);
    }
    
    @Override
    public int get() {
        assert(q.size() > 0) : "Queue is empty";
        int x = q.get(0);
        q.remove(0);
        return x;
    }
}