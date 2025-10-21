package tablice;

import tablice.exceptions.BledneArgumenty;

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
    public abstract Tablica suma(Tablica tablica) throws BledneArgumenty;
    public abstract void przemnoz(Tablica tablica) throws BledneArgumenty;
    public abstract Tablica iloczyn(Tablica tablica) throws BledneArgumenty;
    public abstract void zaneguj();
    public abstract Tablica negacja();
    public abstract void przypisz(Tablica tablica) throws BledneArgumenty;
    public abstract Tablica wycinek(int... wspolrzedne) throws BledneArgumenty;
    public abstract Tablica kopia();

    public void ustaw(double x, int... wspolrzedne) throws BledneArgumenty {
        if (wspolrzedne.length != 2) {throw new BledneArgumenty("Zła liczba wspolrzednych");}

        if (this.poziome) this.values[wspolrzedne[0]][wspolrzedne[1]].set(x);
        else this.values[wspolrzedne[1]][wspolrzedne[0]].set(x);
    }

    public double daj(int... wspolrzedne) throws BledneArgumenty{
        if (wspolrzedne.length != 2 && wspolrzedne.length!=wymiar) {throw new BledneArgumenty("Zła liczba wspolrzednych");}

    
        if (wspolrzedne.length==2){
            if(this.poziome ) return this.values[wspolrzedne[0]][wspolrzedne[1]].get();
        else return this.values[wspolrzedne[1]][wspolrzedne[0]].get();
        }

        if (wymiar==0) return this.values[0][0].get();
        if (wymiar==1) return this.values[0][wspolrzedne[0]].get();
        
        return 0;
    }

    public int[] ksztalt(){
        if (wymiar==0) return new int[0];
        if (wymiar==1) return new int[]{m};
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

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Tablica)) return false;

        Tablica tablica = (Tablica) o;

        if (poziome != tablica.poziome) return false;
        if (wymiar != tablica.wymiar) return false;
        if (n != tablica.n || m != tablica.m) return false;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (values[i][j].get() != tablica.values[i][j].get()) return false;
            }
        }
        return true;
    }


}
