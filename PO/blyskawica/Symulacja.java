public class Symulacja{
    private Gracze gracze;
    private int stawka;

   public Symulacja(Gracze gracze, int stawka){
        this.gracze=gracze;
        this.stawka=stawka;
    }

    public void symuluj(){
        Gra gra = new Gra(this.gracze, this.stawka);

        while (gra.czyTrwa()){
            gra.kolejneRozdanie();
        }
    }

}