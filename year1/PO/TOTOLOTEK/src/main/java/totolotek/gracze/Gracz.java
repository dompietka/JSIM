package totolotek.gracze;

import totolotek.system.Kolektura;
import totolotek.system.Kupon;

import java.util.ArrayList;

public abstract class Gracz {
    private final String imie;
    private final String nazwisko;
    private final String PESEL;
    protected ArrayList<Kupon> kupony = new ArrayList<>();
    private long srodki;

    public Gracz(String imie, String nazwisko, String PESEL, long srodki) {
        this.imie = imie;
        this.nazwisko = nazwisko;
        this.PESEL = PESEL;
        this.srodki = srodki;
    }

    public abstract void kupKupon();

    public void kupKupon(Kolektura kolektura) {
        Kupon kupon = generujKupon(kolektura);
        assert(kupon.cena <= srodki):"za mało środków żeby kupić ten kupon";
        kolektura.sprzedaj(kupon);
        kupony.add(kupon);
        srodki-=kupon.cena;
    }
    public abstract Kupon generujKupon(Kolektura kolektura);

    public void odbierzKupon(Kupon kupon, Kolektura kolektura) {
        srodki+=kolektura.wyplacNagrode(kupon);
        kupony.remove(kupon);
    }

    public void odbyloSieLosowanie(int nrLosowania) {
        for (int i=0;i<kupony.size();i++) {
            Kupon kupon = kupony.get(i);
            if (nrLosowania == kupon.nrLosowania+ kupon.ileLosowan){
                odbierzKupon(kupon, kupon.kolektura);
            }
        }
    }

    @Override
    public String toString() {
        String s = imie + " " + nazwisko + " " + PESEL + "\n" + "Posiadane środki: " + srodki/100 + "zl" + srodki%100 + "gr" + "\nPosiadane kupony:";
        if (kupony.size() > 0) {
            for (Kupon k : kupony) {
                s += " " + k.toString();
            }
        }
        else s+= " brak kuponów";
        return s;
    }



}
