package totolotek.system;

import totolotek.BudzetPanstwa;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class Kolektura {
    public final int numer;
    private ArrayList<Kupon> sprzedaneKupony = new ArrayList<>();
    private Map<Integer, Losowanie> wszystkieLosowania= new HashMap<>();
    public BudzetPanstwa budzetPanstwa;
    public Centrala centrala;
    public int najblizszeLosowanie;

    public Kolektura(int numer, int najblizszeLosowanie, Map<Integer,Losowanie> wszystkieLosowania, Centrala centrala, BudzetPanstwa budzetPanstwa) {
        this.numer = numer;
        this.najblizszeLosowanie = najblizszeLosowanie;
        this.wszystkieLosowania = wszystkieLosowania;
        this.budzetPanstwa = budzetPanstwa;
        this.centrala = centrala;
    }

    public void sprzedaj(Kupon kupon) {
        assert(kupon.kolektura==this):"na kuponie jest wpisana inna kolektura";
        kupon.nrLosowania=najblizszeLosowanie;
        sprzedaneKupony.add(kupon);
        budzetPanstwa.pobierzPodatek(kupon.podatek);
        centrala.budzet+=kupon.cena- kupon.podatek;
        for (int i = 0;i<kupon.ileLosowan;i++){
            for (Zaklad z : kupon.zaklady)
                wszystkieLosowania.get(kupon.nrLosowania+i).dodajZaklad(z);
        }
    }

    public long wyplacNagrode(Kupon kupon) {
        assert (kupon.kolektura==this):"kupon został zakupiony w innej kolekturze";
        kupon.odbierz();
        long nagroda = 0;
        for (int i = 0; i < kupon.ileLosowan;i++){
            if (wszystkieLosowania.get(kupon.nrLosowania+i).odbyloSie){
                for (Zaklad z : kupon.zaklady){
                    nagroda += wszystkieLosowania.get(kupon.nrLosowania+i).nagroda(z);
                }
            }
        }
        centrala.wyplacNagrode(nagroda);
        return nagroda;
    }
}
