package totolotek.system;

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import totolotek.BudzetPanstwa;

import static org.junit.jupiter.api.Assertions.*;

class KolekturaTest {
    static Losowanie l;
    static Centrala c;
    static Kolektura[] kolektury = new Kolektura[10];



    @BeforeAll
    static void setUp() {
        BudzetPanstwa budzetPanstwa = new BudzetPanstwa();
        c = new Centrala(100, budzetPanstwa);
        l = c.wszystkieLosowania.get(1);
        for (int i = 0; i < 10; i++) {
            c.nowaKolektura(101 + i);
            kolektury[i] = c.wszystkieKolektury.get(i);
        }
    }

    @Test
    void testWypisywania() {
        for (int i = 0; i < 10; i++) {
            System.out.println(c.wszystkieKolektury.get(i).toString());
        }
    }
}