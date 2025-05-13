public class Gra{
    private Gracze gracze;
    private int stawka;

    public Gra(Gracze gracze, int stawka){
        this.gracze=gracze;
        this.stawka=stawka;
    }

    public Boolean czyTrwa(){
        return gracze.czyMogaGracDalej(stawka);
    }

    public void kolejneRozdanie(){

    }
}