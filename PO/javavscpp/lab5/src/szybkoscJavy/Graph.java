package szybkoscJavy;

public class Graph extends Matrix {
    public Graph(int n, long[][] values) {
        super(n, values);
    }

    public long countTriangles(){

        Matrix a = new Matrix(this.size, this.matrix);
        a.multiply(this);
        a.multiply(this);
        return a.trace()/6;
    }
}
