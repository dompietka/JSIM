package totolotek.gracze;

import totolotek.system.Kolektura;
import totolotek.system.Kupon;
import totolotek.system.Zaklad;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ThreadLocalRandom;

public class Minimalista extends Gracz {
    Kolektura ulubiona;
    public Minimalista(String imie, String nazwisko, String PESEL, long srodki, Kolektura ulubiona) {
        super(imie, nazwisko, PESEL, srodki);
        this.ulubiona = ulubiona;
    }

    @Override
    public void kupKupon(){
        super.kupKupon(ulubiona);
    }

    @Override
    public Kupon generujKupon(Kolektura kolektura) {
        Zaklad z = new Zaklad();
        ArrayList<Zaklad> lista = new ArrayList<>();
        lista.add(z);
        Kupon k = new Kupon(kolektura, 1, lista, 1);
        return k;
    }

    public Kupon generujKupon(){
        return generujKupon(ulubiona);
    }
}
