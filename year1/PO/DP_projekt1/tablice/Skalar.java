package tablice;

import tablice.exceptions.BledneArgumenty;

public class Skalar extends Tablica{
    public Skalar(double x) {
        super(true, 0, 1,1);
        this.values[0][0] = new ModDouble(x);
    }

    @Override
    public void dodaj(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar()!=0) throw new BledneArgumenty("argument powinien być skalarem");
        this.values[0][0].set(this.values[0][0].get()+tablica.values[0][0].get());
    }

    @Override
    public Tablica suma(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar()==0){
            Skalar s = new Skalar(this.values[0][0].get()+tablica.values[0][0].get());
            return s;
        }
        return tablica.suma(this);
    }



    @Override
    public void przemnoz(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar()!=0) throw new BledneArgumenty("wymiar sie nie zgadza");
        this.values[0][0].set(this.values[0][0].get()*tablica.values[0][0].get());
    }

    @Override
    public Tablica iloczyn(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar()==0){
            Skalar s = new Skalar(this.values[0][0].get()*tablica.values[0][0].get());
            return s;
        }
        return tablica.iloczyn(this);
    }

    @Override
    public void zaneguj(){
        this.values[0][0].set(this.values[0][0].get()*(-1));
    }

    @Override
    public Tablica negacja(){
        Skalar s = new Skalar(this.values[0][0].get()*(-1));
        return s;
    }

    @Override
    public void przypisz(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar() != 0) throw new BledneArgumenty("wymiar sie zgadza");
        this.values[0][0].set(tablica.values[0][0].get());
    }

    @Override
    public Tablica wycinek(int... wspolrzedne) throws BledneArgumenty {
        if (wspolrzedne.length != 0) throw new BledneArgumenty("za duzo wspolrzednych");
        return this;
    }

    @Override
    public Tablica kopia(){
        Skalar s = new Skalar(this.values[0][0].get());
        return s;
    }

    public void ustaw(double x){
        try {
            super.ustaw(x, 0, 0);
        } catch (BledneArgumenty e) {
            System.err.println("Nieoczekiwany błąd wewnetrzny");
            System.exit(2);
        }
    }

    public double daj(){
        try {
            return super.daj(0,0);
        } catch (BledneArgumenty e) {
            System.err.println("Nieoczekiwany błąd wewnetrzny");
            System.exit(2);
        }
        return 0;
    }

    @Override
    public String toString() {
        return "[ " + this.values[0][0].get() + " ]";
    }

}
