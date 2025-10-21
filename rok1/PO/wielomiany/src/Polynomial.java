public class Polynomial {
    public int deg;
    private float[] coefficients = new float[11];

    public Polynomial(float[] coefficients) {
        this.deg=-1;
        assert coefficients.length <= 11 : "degree must be smaller than 10";
        for (int i = 0; i < coefficients.length; i++) {
            this.coefficients[i] = coefficients[i];
            if (coefficients[i] != 0 ) this.deg=i;
        }
    }

    public Polynomial(int[] coefficients) {
        this.deg=-1;
        assert coefficients.length <= 11 : "degree must be smaller than 10";
        for (int i = 0; i < coefficients.length; i++) {
            this.coefficients[i] = coefficients[i];
            if (coefficients[i] != 0 ) this.deg=i;
        }
    }

    public Polynomial(int deg, float coefficient) {
        this.deg=deg;
        assert deg <= 10 : "degree must be smaller than 10";
        coefficients[deg]=coefficient;
    }

    public Polynomial() {
        this.deg=-1;
    }

    @Override
    public String toString() {
        String output = "";
        if (deg == -1) { return "0";}

        if (this.deg > 1) {
            if (coefficients[deg]%1==0) {
                if (coefficients[deg]==1) {output = "x^" + deg;}
                else output = (int) coefficients[deg] + "x^" + deg;
            }
            else output = coefficients[deg] + "x^" + deg;
        }

        if (this.deg == 1){
            if (coefficients[1]%1==0) {
                if (coefficients[1] == 1) {
                    output = "x";
                } else output = (int) coefficients[1] + "x";
            }
            if (coefficients[0]!=0) {
                if (coefficients[0]%1==0) output = output + " + " + (int)coefficients[0];
                else output = output + " + " + coefficients[0];
            }
            return output;

        }

        if (this.deg == 0) {
            if (coefficients[0]%1==0) output = output + (int)coefficients[0];
            else output = output + coefficients[0];
        }



        for (int i = deg-1;i>1;i--){
            if(coefficients[i]!=0){
                if (coefficients[i]%1==0) {
                    if (coefficients[i]==1) {output = output + " + " + "x^" + i;}
                    else output = output + " + " + (int) coefficients[i] + "x^" + i;
                }
                else output = output + " + " + coefficients[i] + "x^" + i;
            }
        }

        if(this.deg != 1 && coefficients[1]!=0){
            if (coefficients[1]%1==0) {
                if (coefficients[1]==1) {output = output + " + " + "x";}
                else output = output + " + " + (int) coefficients[1] + "x";
            }
            else output = output + " + " + coefficients[1] + "x";
        }

        if (this.deg != 0 && coefficients[0]!=0) {
            if (coefficients[0]%1==0) output = output + " + " + (int)coefficients[0];
            else output = output + " + " + coefficients[0];
        }
        return output;
    }

    public void verifyDegree() {
        while (this.deg >= 0 && this.coefficients[deg]==0)
            this.deg--;
    }

    public void add(Polynomial p) {
        this.deg=Integer.max(this.deg, p.deg);
        for (int i=0;i<=this.deg;i++) {
            this.coefficients[i]+=p.coefficients[i];
        }

        this.verifyDegree();
    }

    public void subtract(Polynomial p) {
        for (int i=0;i<=10;i++) {
            this.coefficients[i]-=p.coefficients[i];
        }

        this.deg=10;
        this.verifyDegree();
    }

    public void multiply(Polynomial p) {
        assert this.deg + p.deg <= 10 : "degree must be smaller than 10";
        float [] newCoefficients = new float[11];
        for (int i=0;i<=p.deg;i++) {
            for (int j=0;j<=this.deg;j++){
                newCoefficients[i+j]+=this.coefficients[j]*p.coefficients[i];
            }
        }

        this.coefficients = newCoefficients;
        this.deg+=p.deg;
        this.verifyDegree();
    }

    public void divide(Polynomial p) {
        assert this.deg >= p.deg : "polynomial has to have higher degree than it's divisor";
        assert p.deg != -1 : "dividing by zero is illegal";

        float [] newCoefficients = new float[11];
        int newDeg = this.deg - p.deg;
        for (int i=this.deg;i>=p.deg;i--) {
                newCoefficients[i-p.deg]=this.coefficients[i]/p.coefficients[p.deg];
                Polynomial newPolynomial = new Polynomial(i-p.deg, newCoefficients[i-p.deg]);
                newPolynomial.multiply(p);
                this.subtract(newPolynomial);
        }

        assert this.deg == -1 : "polynomial is not dividable";
        this.deg=newDeg;
        this.coefficients=newCoefficients;
        this.verifyDegree();
    }

    public float value(float x) {
        float result = 0;
        for (int i=this.deg;i>0;i--) {
            result += this.coefficients[i];
            result*=x;
        }
        result+=this.coefficients[0];
        return result;
    }

    public Polynomial derivative() {
        Polynomial result = new Polynomial();
        result.deg=this.deg-1;
        for (int i=0;i<=result.deg;i++) {
            result.coefficients[i]=this.coefficients[i+1]*(i+1);
        }
        return result;
    }

    public Polynomial integral() {
        Polynomial result = new Polynomial();
        result.deg=this.deg+1;
        for (int i=1;i<=result.deg;i++) {
            result.coefficients[i]=this.coefficients[i-1]/i;
        }
        return result;
    }
}
