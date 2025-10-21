package tablice;

import exceptions.BledneArgumenty;

public abstract class Tablica {
    protected boolean poziome;
    protected int wymiar;
    protected int n, m;
    protected ModDouble[][] values;

    public Tablica(boolean poziome, int wymiar, int n, int m) {
        this.poziome = poziome;
        this.wymiar = wymiar;
        this.n = n;
        this.m = m;
        values = new ModDouble[n][m];
    }

    public abstract void dodaj (Tablica tablica) throws BledneArgumenty ;
    public Tablica suma(Tablica tablica) throws BledneArgumenty{};
    public abstract void przemnoz(Tablica tablica) throws BledneArgumenty;
    public abstract Tablica iloczyn(Tablica tablica) throws BledneArgumenty;
    public abstract void zaneguj();
    public abstract Tablica negacja();
    public abstract void przypisz(Tablica tablica) throws Exception;
    public abstract Tablica wycinek(int... wspolrzedne) throws Exception;
    public abstract Tablica kopia();

    public void ustaw(double x, int... wspolrzedne) throws BledneArgumenty {
        if (wspolrzedne.length != 2) {throw new BledneArgumenty("Zła liczba wspolrzednych");}

        if (this.poziome) this.values[wspolrzedne[0]][wspolrzedne[1]].set(x);
        else this.values[wspolrzedne[1]][wspolrzedne[0]].set(x);
    }

    public double daj(int... wspolrzedne) throws BledneArgumenty{
        if (wspolrzedne.length != 2) {throw new BledneArgumenty("Zła liczba wspolrzednych");}

        if (this.poziome) return this.values[wspolrzedne[0]][wspolrzedne[1]].get();
        else return this.values[wspolrzedne[1]][wspolrzedne[0]].get();
    }

    public int[] ksztalt(){
        if (wymiar==0) return new int[]{};
        if (wymiar==1) return new int[]{n};
        if (wymiar==2 && poziome) return new int[]{n, m};
        else return new int[]{m, n};
    }

    public int wymiar(){
        return wymiar;
    }

    public int liczba_elementow(){
        return n*m;
    }

    public void transponuj(){
        poziome = !poziome;
    }

    public void swap(){
        int pom = this.n;
        this.n = this.m;
        this.m = pom;
    }

}
