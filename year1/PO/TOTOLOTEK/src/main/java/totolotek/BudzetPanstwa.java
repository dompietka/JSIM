package totolotek;

public class BudzetPanstwa {
    private long przekazaneSubwencje;
    private long pobranePodatki;

    public BudzetPanstwa() {
        this.przekazaneSubwencje = 0;
        this.pobranePodatki = 0;
    }
    public void dajSubwencje(long kwota){
        przekazaneSubwencje +=kwota;
    }
    public void pobierzPodatek(long kwota){
        pobranePodatki += kwota;
    }

    @Override
    public String toString() {
        String s = "PRZEKAZANE SUBWENCJE: " + przekazaneSubwencje/100 + "zl" + przekazaneSubwencje%100 + "gr"  +
                "\nPOBRANE PODATKI: " + pobranePodatki/100 + "zl" + pobranePodatki%100 + "gr";
        return s;
    }
}
