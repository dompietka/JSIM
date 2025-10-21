package totolotek.gracze;

import totolotek.system.Blankiet;
import totolotek.system.Kolektura;
import totolotek.system.Kupon;
import totolotek.system.Zaklad;

import java.util.ArrayList;

public class Staloliczbowy extends Gracz {
    private int[] liczby = new int[6];
    private Kolektura[] kolektury;
    private int nrKolektury = 0;
    public Staloliczbowy(String imie, String nazwisko, String PESEL, long srodki, int[] ulubioneLiczby, Kolektura[] ulubioneKolektury) {
        super(imie, nazwisko, PESEL, srodki);
        for (int i=0;i<6;i++)
            this.liczby[i] = ulubioneLiczby[i];

        this.kolektury = ulubioneKolektury;
    }

    @Override
    public Kupon generujKupon(Kolektura kolektura) {
        return new Kupon(kolektura, generujBlankiet());
    }

    @Override
    public void kupKupon(){
        kupKupon(kolektury[nrKolektury%kolektury.length]);
        nrKolektury++;
    }

    @Override
    public void kupKupon(Kolektura kolektura) {
        if (kupony.size()>0 && kupony.get(kupony.size()-1).nrLosowania+10<kolektura.najblizszeLosowanie)
            super.kupKupon(kolektura);
    }

    public Blankiet generujBlankiet() {
        ArrayList<Zaklad> zaklady = new ArrayList<Zaklad>();
        for (int i=0;i<8;i++)
            zaklady.add(new Zaklad(liczby));

        return new Blankiet(10,8,zaklady);
    }
}
