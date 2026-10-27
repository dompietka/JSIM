public class Karta {
    private int wartosc; // 2, 3, 4, 5, 6, 7, 8, 9, 10, 11-J, 12-Q, 13-K, 14-A
    private int kolor; // 0-trefl, 1-karo, 2-kier, 3-pik

    public Karta(int wartosc, int kolor) {
        this.wartosc = wartosc;
        this.kolor = kolor;
    }

    public Boolean porownaj(Karta k){ //zwraca 1 jesli karta dla ktorej jest wywolana funckja jets wieksza od argumnenty
        if (this.wartosc > k.wartosc) {
            return true;
        } 
        if (this.wartosc < k.wartosc) {
            return false;
        }
        if (this.kolor > k.kolor) {
            return true;
        }
        else {
            return false;
        }
    }

    @Override
    public String toString() {
        String[] wartosci = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
        String[] kolory = {"trefl", "karo", "kier", "pik"};
        return wartosci[this.wartosc] + kolory[this.kolor];
    }


    
}