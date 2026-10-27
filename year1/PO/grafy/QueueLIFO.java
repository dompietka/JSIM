import java.util.List;
import java.util.ArrayList;

public class QueueLIFO extends Queue {

    public QueueLIFO() {
        super();
    }
    
    @Override
    public void add(int x) {
        q.add(x);
    }
    
    @Override
    public int get() {
        assert(q.size() > 0) : "Queue is empty";
        int x = q.get(q.size()-1);
        q.remove(q.size()-1);
        return x;
    }
}