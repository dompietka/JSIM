public class Gracz{
    private int budzet;
    private Reka reka;

    public Gracz(int budzet){
        this.budzet=budzet;
        reka = new Reka();
    }

    public void nowaReka(Karta[] karty){
        this.reka.ustawReke(karty);
    }

    public Reka getReka(){
        return this.reka;
    }

    public void wplacDoPuli(int kwota){
        this.budzet-=kwota;
    }

    public void zgarnijPule(int kwota){
        this.budzet+=kwota;
    }

    public void zrezygnujDobrowolnie(){

    }

    public int getBudzet(){
        return budzet;
    }

    public Karta zglosKarte(){
        return reka.najwyzsza();
    }

    
}