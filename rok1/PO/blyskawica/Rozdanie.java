public class Rozdanie{
    public static int nrRozdania = 0;
    private Gracze gracze;
    private int stawka;
    private int liczbaGraczy;
    private Talia talia;

    public Rozdanie(Gracze gracze, int stawka){
        this.nrRozdania++;
        this.gracze=gracze;
        this.liczbaGraczy = gracze.getLiczbaGraczy();
        this.stawka=stawka;
        this.talia = new Talia();
    }

    public String rozegraj(){
        for (int i=0;i<liczbaGraczy;i++)
            gracze.gracz(i).wplacDoPuli(stawka);

        for (int i=0;i<5;i++){
             talia.tasuj();
        }
       
        gracze.rozdaj(talia);
        Karta[] pomStol = new Karta[liczbaGraczy];
        for (int i=0;i<liczbaGraczy;i++){
            pomStol[i]=gracze.gracz(i).zglosKarte();
        }
        Stol stol = new Stol(pomStol, liczbaGraczy);
        int zwyciezca = stol.nrNajwyzszej();
        gracze.gracz(zwyciezca).zgarnijPule(liczbaGraczy*stawka);
        return this.toString();
    }

    @Override
    public String toString(){
        return "Rozdanie " + nrRozdania + gracze.toString(); 
    }


}