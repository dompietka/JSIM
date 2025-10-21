package totolotek.system;

import java.util.ArrayList;

public class Blankiet {
    public int ileLosowan;
    public int liczbaZakladow;
    public ArrayList<Zaklad> zaklady;
    public boolean[][] pola = new boolean[8][50]; //50 pole to anuluj
    public boolean[] losowania = new boolean[10]; //traktujemy jako 9 wiersz przy zaznaczaniu

    public Blankiet(int ileLosowan, int liczbaZakladow, ArrayList<Zaklad> zaklady) {
        this.ileLosowan = ileLosowan;
        this.liczbaZakladow = liczbaZakladow;
        this.zaklady = zaklady;
    }

    public Blankiet(){
    }

    public void zaznacz(int i, int j){
        assert(i<=9 && i>0 && j>0):"nie ma takiego pola na blankiecie";
        if (i==9){
            assert(j<=10):"nie ma takiego pola na blankiecie";
            losowania[j-1]=true;
        }
        else {
            assert(j<=50):"nie ma takiego pola na blankiecie";
            pola[i-1][j-1]=true;
        }
    }

    public void odczytaj(){
        ileLosowan=1;
        for (int i = 0; i < 10; i++) {
            if (losowania[i]) {ileLosowan=i+1;}
        }

        liczbaZakladow=0;
        for (int i = 0; i < 8; i++) {
            if (pola[i][49]) continue;
            int licznik = 0;
            int[] liczby = new int[6];
            for (int j = 0; j < 49; j++) {
                if (pola[i][j]) {liczby[licznik]=j+1; licznik++; }
            }
            if (licznik==6){
                liczbaZakladow++;
                zaklady.add(new Zaklad(liczby));
            }
        }
    }

    @Override
    public String toString() {
        String s = "";
        for(int i=0;i<8;i++){
            s+=(i+1)+"\n";
            for (int j=0;j<9;j++){
                if (pola[i][j]) {
                    s+="[ -- ] ";
                }
                else {
                    s+="[  " + (j+1) + " ] ";
                }
            }

            if (pola[i][9]) {
                s+="[ -- ] ";
            }
            else {
                s+="[ " + 10 + " ] ";
            }
            s+="\n";

            for (int j=10;j<20;j++){
                if (pola[i][j]) {
                    s+="[ -- ] ";
                }
                else {
                    s+="[ " + (j+1) + " ] ";
                }
            }
            s+="\n";

            for (int j=20;j<30;j++){
                if (pola[i][j]) {
                    s+="[ -- ] ";
                }
                else {
                    s+="[ " + (j+1) + " ] ";
                }
            }
            s+="\n";

            for (int j=30;j<40;j++){
                if (pola[i][j]) {
                    s+="[ -- ] ";
                }
                else {
                    s+="[ " + (j+1) + " ] ";
                }
            }
            s+="\n";

            for (int j=40;j<49;j++){
                if (pola[i][j]) {
                    s+="[ -- ] ";
                }
                else {
                    s+="[ " + (j+1) + " ] ";
                }
            }
            s+="\n";

            if (pola[i][49]) s+="[ -- ]";
            else s+="[   ]";

            s+=" anuluj\n";
        }

        return s;
    }
}
