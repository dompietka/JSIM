package totolotek;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import totolotek.system.Zaklad;

class ZakladTest {
    static Zaklad zakladGlowny;
    @BeforeAll
    static void setUp() {
        zakladGlowny = new Zaklad(28,6, 5, 17, 23, 31);
    }
    @Test
    void test0(){
        Zaklad zaklad0 = new Zaklad(3, 2, 8, 12, 20, 35);
        Assertions.assertEquals(0, zakladGlowny.porownaj(zaklad0));
    }

    @Test
    void test1(){
        Zaklad zaklad1 = new Zaklad(3, 2, 8, 12, 23, 35);
        Assertions.assertEquals(1, zakladGlowny.porownaj(zaklad1));
    }

    @Test
    void test2(){
        Zaklad zaklad2 = new Zaklad(3, 5, 8, 17, 20, 35);
        Assertions.assertEquals(2, zakladGlowny.porownaj(zaklad2));
    }

    @Test
    void test3(){
        Zaklad zaklad3 = new Zaklad(3, 5, 8, 17, 20, 31);
        Assertions.assertEquals(3, zakladGlowny.porownaj(zaklad3));
    }

    @Test
    void test4(){
        Zaklad zaklad4 = new Zaklad(23, 5, 8, 17, 20, 31);
        Assertions.assertEquals(4, zakladGlowny.porownaj(zaklad4));
    }

    @Test
    void test5(){
        Zaklad zaklad5 = new Zaklad(23, 5, 6, 17, 20, 31);
        Assertions.assertEquals(5, zakladGlowny.porownaj(zaklad5));
    }

    @Test
    void test6(){
        Zaklad zaklad6 = new Zaklad(23, 5, 6, 17, 28, 31);
        Assertions.assertEquals(6, zakladGlowny.porownaj(zaklad6));
    }

    @Test
    void test7(){
        System.out.println(zakladGlowny);
    }
}