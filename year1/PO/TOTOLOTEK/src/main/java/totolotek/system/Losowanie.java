package totolotek.system;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ThreadLocalRandom;

public class Losowanie {
    private static int licznikGlobalny = 1;
    public final int numerPorzadkowy;
    public boolean odbyloSie = false;
    public Zaklad wynikLosowania;
    public Map<Zaklad, Integer> zaklady = new HashMap<>();
    public long wplaty = 0;
    public long kumulacja;
    public long pula;
    public long[] nagroda = new long[6];
    public int[] ileWygranych = new int[6];
    public Centrala centrala;

    public Losowanie(Centrala centrala) {
        this.numerPorzadkowy = licznikGlobalny++;
        this.centrala = centrala;
    }

    public void losuj(){
        int i=0;
        int[] liczby = new int[6];
        Map<Integer, Boolean> mapa = new HashMap<>();
        while (i < 6) {
            int x = ThreadLocalRandom.current().nextInt(1, 50);
            if (!mapa.containsKey(x)) {
                mapa.put(x, true);
                liczby[i] = x;
                i++;
            }
        }
        this.wynikLosowania = new Zaklad(liczby);
        this.odbyloSie = true;
    }

    public void dodajZaklad(Zaklad zaklad) {
        wplaty+=240;
        zaklady.put(zaklad, zaklady.getOrDefault(zaklad, 0) + 1);
    }

    public void wyliczNagrody(){

        //ile wygranych danego stopnia - indeks to ile liczb sie zgadza-1
        for(Map.Entry<Zaklad, Integer> entry : zaklady.entrySet() ) {
            if(wynikLosowania.porownaj(entry.getKey())>=1) ileWygranych[wynikLosowania.porownaj(entry.getKey())-1]+=entry.getValue();
        }

        pula = wplaty/100;
        pula*=51;

        //obliczenia puli na nagrody konkretnych stopni - indekt to lizba zgadzajacych sie liczb-1
        nagroda[2]=2400;
        nagroda[5]+=(pula/100)*44;
        nagroda[4] = (pula/100)*8;
        nagroda[3] = pula - nagroda[5] - nagroda[4] - nagroda[2]*ileWygranych[2];
        if (nagroda[5]<200000000) nagroda[5]=200000000;

        //obliczenie kwot poszczegolnych wygranych
        if (ileWygranych[5]!=0) nagroda[5]/=ileWygranych[5];
        else kumulacja=nagroda[5];

        if(ileWygranych[4]!=0) nagroda[4]/=ileWygranych[4];
        if(ileWygranych[3]!=0) nagroda[3]/=ileWygranych[3];

        if (nagroda[3]<3600) nagroda[3]=3600;

    }

    public long nagroda(Zaklad zaklad){
        if (wynikLosowania.porownaj(zaklad)>=1) return nagroda[wynikLosowania.porownaj(zaklad)-1];
        else return 0;
    }


    @Override
    public String toString() {
        return "Losowanie nr " + numerPorzadkowy + "\nWyniki: " + wynikLosowania;
    }
}
