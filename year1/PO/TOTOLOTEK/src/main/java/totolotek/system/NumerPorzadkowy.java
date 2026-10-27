package totolotek.system;

import java.util.concurrent.ThreadLocalRandom;

public class NumerPorzadkowy {
    private int numer;
    private int numerKolektury;
    private String losowyNumer = "";
    private int sumaKontrolna = 0;
    private static int licznik = 1;

    public NumerPorzadkowy(int NumerKolektury)  {
        this.numerKolektury = NumerKolektury;
        this.numer = licznik++;
        for (int i = 0; i < 9; i++) {
            losowyNumer+= ThreadLocalRandom.current().nextInt(0, 10);
        }
        sumaKontrolna = sumacyfr(numer) + sumacyfr(numerKolektury) + sumacyfr(losowyNumer);
        sumaKontrolna%=100;
    }

    private int sumacyfr(int x){
        int ans = 0;
        while (x>=10){
            ans+=x%10;
            x=x/10;
        }
        ans+=x;
        return ans;
    }

    private int sumacyfr(String x){
        int ans = 0;
        for (int i=0;i<x.length();i++){
            ans+=Character.getNumericValue(x.charAt(0));
        }
        return ans;
    }

    public String get() {
        String s = "";
        s+=numer + "-" + numerKolektury + "-" + losowyNumer + "-";
        if (sumaKontrolna < 10) { s+="0";}
        s+=sumaKontrolna;
        return s;
    }
}
