package totolotek;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import totolotek.system.Centrala;
import totolotek.system.Losowanie;
import totolotek.system.Zaklad;

import java.util.Map;

class LosowanieTest {

    static Losowanie l;
    static Centrala c;

    @BeforeAll
    static void setUp(){
        BudzetPanstwa budzetPanstwa = new BudzetPanstwa();
        c = new Centrala(100, budzetPanstwa);
        l =c.wszystkieLosowania.get(1);

        for (int i=0;i<100;i++){
            l.dodajZaklad(new Zaklad(2, 6, 12, 25, (i+5)%49+1, (i+12)%49+1));
        }

        for (int i=0;i<10;i++){
            l.dodajZaklad(new Zaklad(2, 6, 12, 25, 36, (i+12)%49+1));
        }

        for (int i=0;i<100;i++){
            l.dodajZaklad(new Zaklad(2, 3, 10, 25, (i+5)%49+1, (i+12)%49+1));
        }

        l.dodajZaklad(new Zaklad(2, 6, 12, 25, 16, 42));

        l.losuj();
        l.wynikLosowania = new Zaklad(2,6,12,25,16,42);
        l.wyliczNagrody();

    }

    @Test
    void test1(){
        Assertions.assertEquals(true, l.odbyloSie);

    }

    @Test
    void test2(){
        Assertions.assertEquals(240*211, l.wplaty);
        System.out.println(l.wplaty);
    }

    @Test
    void test3(){
        int[] ilewygranych = new int[6];
        for(Map.Entry<Zaklad, Integer> entry : l.zaklady.entrySet() ) {
            ilewygranych[l.wynikLosowania.porownaj(entry.getKey())-1]+=entry.getValue();
        }

        for (int i=0;i<ilewygranych.length;i++){
            System.out.println(ilewygranych[i]);
        }

    }

    @Test
    void test4(){
       Assertions.assertEquals(200000000, l.nagroda[5]);
    }

    @Test
    void test5(){
        Assertions.assertEquals(2400, l.nagroda[2]);
    }

    @Test
    void test6(){
        assert(l.nagroda[3]>=3600);
    }

    @Test
    void test7(){
        System.out.println(l);
    }

    @Test
    void testWypisywaniaCentrali(){
        System.out.println(c);
    }

}