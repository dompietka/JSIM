package totolotek.system;

import totolotek.BudzetPanstwa;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class Centrala {
    public long budzet;
    public BudzetPanstwa budzetPanstwa;
    int najblizszeLosowanie = 1;
    public Map<Integer, Losowanie> wszystkieLosowania= new HashMap<>();
    public ArrayList<Kolektura> wszystkieKolektury = new ArrayList<>();

    public Centrala(int budzet, BudzetPanstwa budzetPanstwa) {
        this.budzet = budzet;
        this.budzetPanstwa = budzetPanstwa;
        for(int i=0;i<10;i++){
            wszystkieLosowania.put(i+1, new Losowanie(this));
        }
    }

    public void nowaKolektura(int nr){
        Kolektura k = new Kolektura(nr, najblizszeLosowanie, this.wszystkieLosowania, this, this.budzetPanstwa);
        wszystkieKolektury.add(k);
    }

    public void wyplacNagrode(long nagroda){
        if (nagroda <= budzet){
            budzet -= nagroda;
        }
        else {
            budzet = 0;
            budzetPanstwa.dajSubwencje(nagroda-budzet);
        }
    }

    public void przeprowadzLosowanie(int nrLosowania) {
        wszystkieLosowania.put(nrLosowania+10, new Losowanie(this));

        Losowanie obecneLosowanie = wszystkieLosowania.get(nrLosowania);
        budzet+=((obecneLosowanie.wplaty/100)*49);
        if (nrLosowania>1) obecneLosowanie.nagroda[5]+=wszystkieLosowania.get(nrLosowania-1).kumulacja;
        obecneLosowanie.losuj();
        obecneLosowanie.wyliczNagrody();

        for (Kolektura k : wszystkieKolektury) {
            k.najblizszeLosowanie++;
        }

    }

    @Override
    public String toString() {
        String s = "Centrala [budzet=" + budzet + "]\nLOSOWANIA:";
        for (Losowanie losowanie : wszystkieLosowania.values()) {
            if (losowanie.odbyloSie){
                s += losowanie + "\nPodsumowanie nagród:\n";
                for (int i=2;i<6;i++){
                    s+= "nagrodę stopnia " + (6-i) + " równą " + losowanie.nagroda[i]/100 + " zł " + losowanie.nagroda[i]%100
                            + " gr wygrało " + losowanie.ileWygranych[i] + " osób\n";
                }

            }

        }
        return s;

    }




}
