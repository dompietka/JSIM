package szybkoscJavy;

public class Matrix {
    public long[][] matrix;
    public int size;

    public Matrix(int size, long[][] values) {
        this.size = size;
        this.matrix = new long[size][size];
        for (int i=0;i<size;i++){
            for (int j=0;j<size;j++){
                matrix[i][j] = values[i][j];
            }
        }
    }

    public void multiply(Matrix b){
        Matrix result = new Matrix(this.size, new long[this.size][this.size]);
        result.size = this.size;

        for (int i=0;i<this.size;i++){
            for (int j=0;j<this.size;j++){
                for (int k=0;k<this.size;k++){
                    result.matrix[i][j] += this.matrix[i][k]*b.matrix[k][j];
                }
            }
        }

        this.matrix = result.matrix;

    }

    public long trace() {
        long trace = 0;
        for (int i=0;i<this.size;i++){
            trace += this.matrix[i][i];
        }

        return trace;
    }

    public void print() {
        for (int i=0;i<this.size;i++){
            for (int j=0;j<this.size;j++){
                System.out.print(this.matrix[i][j]+"\t");
            }
            System.out.println("\n");
        }
    }
}
