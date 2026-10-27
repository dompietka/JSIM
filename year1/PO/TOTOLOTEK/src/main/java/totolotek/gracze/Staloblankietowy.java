package totolotek.gracze;

import totolotek.system.Blankiet;
import totolotek.system.Kolektura;
import totolotek.system.Kupon;

public class Staloblankietowy extends Gracz {
    private Blankiet blankiet;
    private Kolektura[] kolektury;
    private int nrKolektury = 0;
    private int coIleLosowan;
    public Staloblankietowy(String imie, String nazwisko, String PESEL, long srodki, Blankiet blankiet, Kolektura[] kolektury, int coIleLosowan) {
        super(imie, nazwisko, PESEL, srodki);
        this.blankiet = blankiet;
        this.kolektury = kolektury;
        this.coIleLosowan = coIleLosowan;
    }

    @Override
    public Kupon generujKupon(Kolektura kolektura) {
        return new Kupon(kolektura, blankiet);
    }

    @Override
    public void kupKupon(){
        kupKupon(kolektury[nrKolektury%kolektury.length]);
        nrKolektury++;
    }

    @Override
    public void kupKupon(Kolektura kolektura) {
        if (kolektura.najblizszeLosowanie%coIleLosowan==0) {
            super.kupKupon(kolektura);
        }

    }

}
