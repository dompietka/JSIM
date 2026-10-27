package tablice;
import java.util.Arrays;

import tablice.exceptions.BledneArgumenty;
public class Main {
    public static void main(String[] args) throws BledneArgumenty {
        // Example usage of Skalar class
        Tablica m1 = new Macierz(new double[][]{
            {1, 1, 1},
            {2, 2, 2}
        });
        Macierz m2 = new Macierz(new double[][]{
            {7, 8, 9},
            {10, 11, 12},
            {0, 2, 4}
        });
        Wektor w1 = new Wektor(new double[]{1, 2, 3});
        Skalar s1 = new Skalar(2.0);

        System.out.println(m1 + " " + Arrays.toString(m1.ksztalt()) + " " + m1.wymiar() + " " + m1.daj(0, 1));
        System.out.println(m2 + " " + Arrays.toString(m2.ksztalt()) + " " + m2.wymiar() + " " + m2.daj(1, 2));

        m2.dodaj(w1);
        m2.transponuj();
        Tablica m3=m1.iloczyn(m2);
        System.out.println(m1);
        System.out.println(m2);
        System.out.println(m3);
       
    }
}
