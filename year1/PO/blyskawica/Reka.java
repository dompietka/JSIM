public class Reka extends Karty {
    public Reka(){
        super(5);
    }

    public void ustawReke(Karta[] karty){
        this.karty=karty;
    }

    @Override
    public String toString(){
        return karty[0].toString() + " " +  karty[1].toString() + " " + karty[2].toString() + " " + karty[3].toString() + " " + karty[4].toString() + " ";
    }

}