package totolotek;

import totolotek.gracze.*;
import totolotek.system.*;

import java.util.ArrayList;
import java.util.concurrent.ThreadLocalRandom;

public class Main {
    public static void main(String[] args) {
        BudzetPanstwa budzetPanstwa = new BudzetPanstwa();
        Centrala centrala = new Centrala(10000000, budzetPanstwa);
        Kolektura[] kolektury = new Kolektura[10];

        for (int i = 0; i < 10; i++) {
            centrala.nowaKolektura(101+i);
            kolektury[i] = centrala.wszystkieKolektury.get(i);
        }


        //generowanie graczy
        Gracz[] gracze = new Gracz[800];

        for (int i = 0; i < 200; i++) {
            gracze[i] = new Minimalista("imie"+i, "nazwisko"+i,"randomPESEL",  ThreadLocalRandom.current().nextInt(200, 500), kolektury[i%10]);
        }

        for (int i = 0; i < 200; i++) {
            gracze[200+i] = new Losowy("imie" + (200+i), "nazwisko"+(200+i), "randomPESEL", kolektury);
        }

        for (int i = 0; i < 200; i++) {

            int[] ulubione = new int[6];
            Zaklad z = new Zaklad();
            ulubione=z.liczby;

            Kolektura[] ulubioneK = new Kolektura[2];
            ulubioneK[0]=kolektury[i%10];
            ulubioneK[1]=kolektury[(i+3)%10];

            gracze[400+i] = new Staloliczbowy("imie"+(400+i), "nazwisko"+(400+i), "randomPESEL",
                    ThreadLocalRandom.current().nextInt(200, 500), ulubione, ulubioneK);
        }

        for (int i = 0; i < 200; i++) {
            Kolektura[] ulubioneK = new Kolektura[2];
            ulubioneK[0]=kolektury[i%10];
            ulubioneK[1]=kolektury[(i+3)%10];

            int liczbaZakladow = ThreadLocalRandom.current().nextInt(1, 9);
            int liczbaLosowan = ThreadLocalRandom.current().nextInt(1, 11);
            ArrayList<Zaklad> zaklady = new ArrayList<>();
            for (int j = 0; j < liczbaZakladow; j++) {
                Zaklad z = new Zaklad();
                zaklady.add(z);
            }
            Blankiet blankiet = new Blankiet(liczbaLosowan, liczbaZakladow, zaklady);
            gracze[600+i] = new Staloblankietowy("imie"+(600+i), "nazwisko"+(600+i), "randomPESEL",
                    ThreadLocalRandom.current().nextInt(200, 500), blankiet, ulubioneK,
                    ThreadLocalRandom.current().nextInt(2, 11));


        }

        //przeprowadzenie losowan
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 800; j++) {
                gracze[j].kupKupon();
            }

            centrala.przeprowadzLosowanie(i+1);

            for (int j = 0; j < 800; j++) {
                gracze[j].odbyloSieLosowanie(i);
            }
        }

        System.out.println(centrala);


    }
}
