package totolotek.system;

import java.util.ArrayList;

public class Kupon {
    private boolean zrealizowany = false;
    public Kolektura kolektura;
    private NumerPorzadkowy nrPorzadkowy;
    public String identyfikator;
    public final int liczbaZakladow;
    public ArrayList<Zaklad> zaklady = new ArrayList<>();
    public int nrLosowania;
    public final int ileLosowan;
    public final long cena;
    public final long podatek;

    public Kupon(Kolektura kolektura, int ileLosowan, ArrayList<Zaklad> zaklady, int liczbaZakladow) {
        this.kolektura = kolektura;
        this.nrPorzadkowy = new NumerPorzadkowy(kolektura.numer);
        this.identyfikator = nrPorzadkowy.get();
        this.ileLosowan = ileLosowan;
        this.zaklady = zaklady;
        this.liczbaZakladow = liczbaZakladow;
        this.cena = 300*liczbaZakladow*ileLosowan;
        this.podatek = 60 * liczbaZakladow * ileLosowan;
        assert(this.zaklady.size() == liczbaZakladow):"dane się nie zgadzają - liczba zakładów inna niz dana";
        assert(liczbaZakladow<=10):"za dużo zakładów";
    }

    public Kupon(Kolektura kolektura, Blankiet blankiet) {
        this(kolektura, blankiet.ileLosowan, blankiet.zaklady, blankiet.liczbaZakladow);
    }

    public void odbierz(){
        zrealizowany = true;
    }

    @Override
    public String toString() {
        String s = "KUPON NR " + this.identyfikator;
        for (int i = 0; i < this.zaklady.size(); i++) {
            s+="\n" + (i+1) + ": ";
            for (int j = 0; j < 6; j++) {
                if (this.zaklady.get(i).liczby[j] < 10) {s+=" ";}
                s+=this.zaklady.get(i).liczby[j]+" ";
            }
        }
        s+="\nLICZBA LOSOWAN: " +   this.ileLosowan;
        s+="\nNUMERY LOSOWAN:\n";
        for (int i = 0; i < this.ileLosowan; i++) {
            s+=(this.nrLosowania+i) + " ";
        }
        s+="\nCENA: " + this.cena/100 + "zl" + this.cena%100 + "gr";
        return s;
    }
}
