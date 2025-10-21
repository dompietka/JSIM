package totolotek.gracze;

import totolotek.system.Kolektura;
import totolotek.system.Kupon;
import totolotek.system.Zaklad;

import java.util.ArrayList;
import java.util.concurrent.ThreadLocalRandom;

public class Losowy extends Gracz {
    private Kolektura[] kolektury;

    public Losowy(String imie, String nazwisko, String PESEL, Kolektura[] kolektury) {
        super(imie, nazwisko, PESEL,  ThreadLocalRandom.current().nextInt(0, 100000000));
        this.kolektury = kolektury;
    }

    @Override
    public void kupKupon(Kolektura kolektura) {
        int liczbaKuponow = ThreadLocalRandom.current().nextInt(0, 101);
        for (int i = 0; i < liczbaKuponow; i++) {
            super.kupKupon(kolektura);
        }
    }

    @Override
    public void kupKupon() {
        int liczbaKuponow = ThreadLocalRandom.current().nextInt(0, 101);
        for (int i = 0; i < liczbaKuponow; i++) {
            super.kupKupon(kolektury[ ThreadLocalRandom.current().nextInt(0, kolektury.length)]);
        }
    }

    @Override
    public Kupon generujKupon(Kolektura kolektura) {
        int liczbaZakladow = ThreadLocalRandom.current().nextInt(1, 9);
        int liczbaLosowan = ThreadLocalRandom.current().nextInt(1, 11);
        ArrayList<Zaklad> zaklady = new ArrayList<>();
        for (int i = 0; i < liczbaZakladow; i++) {
            Zaklad z = new Zaklad();
            zaklady.add(z);
        }
        return new Kupon(kolektura, liczbaLosowan, zaklady, liczbaZakladow);
    }

    public Kupon generujKupon() {
        return generujKupon(kolektury[ ThreadLocalRandom.current().nextInt(0, kolektury.length)]);
    }




}
