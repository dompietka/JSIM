import java.util.Scanner;


//delikatny chaos przy wypisywaniu, ale :runTests działa bez problemu - pozostałe funkcje nie do końca
//w toString też jest lekki chaos - wszytsko działa, ale kod nie jest piękny - wynika to z próby wypisywania faktycznie user friendly, nie jest konieczne

public class Main {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        //System.out.println("input format: string to define operation, new line, then degree of polynomial then coefficients from lowers power, then next polynomial in new line if required");
        //System.out.println("possible operations: add, substract, multiply, divide, derivative, integral");
        System.out.println("to run few system tests, type :runTests");
        System.out.println("to quit, type :quit");

        String input = scanner.nextLine();


        while (!input.equals(":quit")) {
            if(input.equals(":runTests")) {
                float[] coefficients = new float[] {0, 2, 6, 3, 8, 0, 0, 0, 0, 0, 0};
                float[] coefficients2 = new float[] {5, 8, 3, 4, 0, 0, 0, 0, 0, 0, 0};

                Polynomial p = new Polynomial(coefficients);
                Polynomial p2 = new Polynomial(coefficients2);
                Polynomial zero = new Polynomial();

                System.out.println("p: " + p.toString() + "stopień to " + p.deg );
                System.out.println("p2: " + p2.toString() + "stopień to " + p2.deg);

                p.add(p2);
                //p.add(zero);

                System.out.println("p+=p2: " + p.toString() + "stopień to " + p.deg );

                p.subtract(p2);
                p2.subtract(p);

                System.out.println("p-=p2; p2-=p: " + p2.toString() + "stopień to " + p2.deg );

                p2.multiply(p);

                System.out.println("p2*=p: " + p2.toString() + "stopień to " + p2.deg );
                //p.multiply(zero);
                //System.out.println("p: " + p.toString() + "stopień to " + p.deg );

                p2.divide(p);
                System.out.println("p2/=p: " + p2.toString() + "stopień to " + p2.deg );
                //p.divide(p2);
                //System.out.println("p: " + p.toString() + "stopień to " + p.deg );

                System.out.println("integral p: " + p.integral().toString() + "stopień to " + p.integral().deg );
                System.out.println("derivative p2: " + p2.derivative().toString() + "stopień to " + p2.derivative().deg );


            }
            /*if (input.equals("add")) {
                int d1, d2;
                float[] coefficients = new float[11];
                float[] coefficients2 = new float[11];
                d1 = scanner.nextInt();
                for (int i=0;i<=d1;i++) {
                    coefficients[i] = scanner.nextFloat();
                }
                d2 = scanner.nextInt();
                for (int i=0;i<=d2;i++) {
                    coefficients2[i] = scanner.nextFloat();
                }
                Polynomial p = new Polynomial(coefficients);
                Polynomial p2 = new Polynomial(coefficients2);
                p.add(p2);
                System.out.println(p.toString());
            }
            if (input.equals("substract")) {
                int d1, d2;
                float[] coefficients = new float[11];
                float[] coefficients2 = new float[11];
                d1 = scanner.nextInt();
                for (int i=0;i<=d1;i++) {
                    coefficients[i] = scanner.nextFloat();
                }
                d2 = scanner.nextInt();
                for (int i=0;i<=d2;i++) {
                    coefficients2[i] = scanner.nextFloat();
                }
                Polynomial p = new Polynomial(coefficients);
                Polynomial p2 = new Polynomial(coefficients2);
                p.subtract(p2);
                System.out.println(p.toString());
            }
            if (input.equals("multiply")) {
                int d1, d2;
                float[] coefficients = new float[11];
                float[] coefficients2 = new float[11];
                d1 = scanner.nextInt();
                for (int i=0;i<=d1;i++) {
                    coefficients[i] = scanner.nextFloat();
                }
                d2 = scanner.nextInt();
                for (int i=0;i<=d2;i++) {
                    coefficients2[i] = scanner.nextFloat();
                }
                Polynomial p = new Polynomial(coefficients);
                Polynomial p2 = new Polynomial(coefficients2);
                p.multiply(p2);
                System.out.println(p.toString());
            }
            if (input.equals("divide")) {
                int d1, d2;
                float[] coefficients = new float[11];
                float[] coefficients2 = new float[11];
                d1 = scanner.nextInt();
                for (int i=0;i<=d1;i++) {
                    coefficients[i] = scanner.nextFloat();
                }
                d2 = scanner.nextInt();
                for (int i=0;i<=d2;i++) {
                    coefficients2[i] = scanner.nextFloat();
                }
                Polynomial p = new Polynomial(coefficients);
                Polynomial p2 = new Polynomial(coefficients2);
                p.divide(p2);
                System.out.println(p.toString());
            }
            if (input.equals("derivative")) {
                int d1, d2;
                float[] coefficients = new float[11];
                float[] coefficients2 = new float[11];
                d1 = scanner.nextInt();
                for (int i=0;i<=d1;i++) {
                    coefficients[i] = scanner.nextFloat();
                }
                Polynomial p = new Polynomial(coefficients);
                System.out.println(p.toString() + " " + p.derivative().toString());
            }
            if (input.equals("integral")) {
                int d1, d2;
                float[] coefficients = new float[11];
                float[] coefficients2 = new float[11];
                d1 = scanner.nextInt();
                for (int i=0;i<=d1;i++) {
                    coefficients[i] = scanner.nextFloat();
                }
                Polynomial p = new Polynomial(coefficients);
                System.out.println(p.integral().toString());
            }*/
            input = scanner.nextLine();
        }


        System.out.println("goodbye");
    }

}