package tablice;

import exceptions.BledneArgumenty;

public class Wektor extends Tablica{
    public Wektor(double[] vals) {
        super(true, 1, 1, vals.length);
        for (int i = 0; i < vals.length; i++) {
            this.values[0][i] = new ModDouble(vals[i]);
        }
    }

    public Wektor(int m){
        super(true, 1, 1, m);
        for (int i = 0; i < m; i++) {
            this.values[0][i] = new ModDouble(0);
        }
    }


    @Override
    public void ustaw(double x, int... wspolrzedne) throws BledneArgumenty {
        if (wspolrzedne.length != 1) {throw new BledneArgumenty("Zła liczba wspolrzednych");}
        try {
            super.ustaw(x, 0, wspolrzedne[0]);
        } catch (BledneArgumenty e) {
            System.err.println("Nieoczekiwany błąd wewnetrzny");
            System.exit(2);
        }
    }

    @Override
    public double daj(int... wspolrzedne) throws BledneArgumenty {
        if (wspolrzedne.length != 1) {throw new BledneArgumenty("Zła liczba wspolrzednych");}
        try {
            return super.daj(0, wspolrzedne[0]);
        } catch (BledneArgumenty e) {
            System.err.println("Nieoczekiwany błąd wewnetrzny");
            System.exit(2);
        }
        return 0;
    }

    @Override
    public void dodaj(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar == 2) throw new BledneArgumenty("zly wymiar argumentu");
        if (tablica.wymiar == 1 && tablica.m != this.m) throw new BledneArgumenty("zly ksztalt argumentu");
        if (tablica.wymiar ==1 && tablica.poziome != this.poziome) throw new BledneArgumenty("zla orientacja argumentu");

        if (tablica.wymiar==0){
            for (int i=1;i<m;i++){
                values[0][i].set(values[0][i].get()+tablica.daj());
            }
        }
        else {
            for (int i=1;i<m;i++){
                this.values[0][i].set(this.values[0][i].get()+tablica.values[0][i].get());
            }
        }
    }



    @Override
    public Tablica suma(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar <= this.wymiar){
            Tablica w = this.kopia();
            try{
                w.dodaj(tablica);
            }
            catch (BledneArgumenty e){
                throw e;
            }
            return w;
        }
        return tablica.suma(this);
    }

    @Override
    public void przemnoz(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar == 0) {
            for (int i=1;i<m;i++){
                values[0][i].set(values[0][i].get()*tablica.daj());
            }
            return;
        }
        throw new BledneArgumenty("nie da sie przemnozyc - ksztat wyniku inny niz obecny");
    }

    @Override
    public Tablica iloczyn(Tablica tablica) throws BledneArgumenty{
        if (tablica.wymiar == 0) {
            Tablica w = this.kopia();
            w.przemnoz(tablica);
            return w;
        }
        if (tablica.wymiar==1){
            if (this.m!=tablica.m) throw new BledneArgumenty("zly ksztalt argumentu");
            if (this.poziome){
                double x = 0;
                for (int i=1;i<m;i++){
                    x+=tablica.values[0][i].get()*this.values[0][i].get();
                }
                if (tablica.poziome)
                    return new Skalar (x);
                else
                    return new Macierz(new double[][]{{x}});
            }
            Macierz m = new Macierz(this.m, this.m);
            for (int i=0;i<this.m;i++){
                for (int j=0;j<this.m;j++){
                    m.values[i][j].set(this.values[0][i].get()*tablica.values[0][j].get());
                }
            }
            return m;
        }
        int n=tablica.n;
        int m=tablica.m;
        if(!tablica.poziome){n=tablica.m; m=tablica.n;}
        if(!this.poziome && this.m == m) return tablica.iloczyn(this);
        if(this.m!=n) throw new BledneArgumenty("zly ksztalt argumentu");
        Tablica w = new Wektor(m);
        for (int i=0;i<m;i++){
            double x = 0;
            for (int j=0;j<this.m;j++){
                x+=tablica.daj(j, i)*this.values[0][j].get();
            }
            w.values[0][i].set(x);
        }
        return w;
    }



    @Override
    public void zaneguj(){
        for (int i=1;i<m;i++){
            values[0][i].set(values[0][i].get()*(-1));
        }
    }

    @Override
    public Tablica negacja(){
        Wektor w = new Wektor(this.m);
        w.poziome=this.poziome;

        for (int i=1;i<m;i++){
            w.values[0][i].set(this.values[0][i].get()*(-1));
        }
        return w;
    }

    @Override
    public void przypisz(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar > this.wymiar) throw new BledneArgumenty("Za duzy wymiar argumentu");
        if (tablica.wymiar==1 && tablica.m != this.m) throw new BledneArgumenty("niepoprawny kształt argumentu");
        if (tablica.wymiar==0){
            for (int i=0;i<m;i++)
                values[0][i].set(tablica.daj());
        }
        else {
            for (int i=0;i<m;i++)
                values[0][i].set(tablica.daj(i));
        }
    }



    @Override
    public Tablica kopia(){
        Wektor w;
        w = new Wektor(this.m);
        w.poziome=this.poziome;

        for(int i=0;i<this.m;i++){
            w.values[0][i].set(this.values[0][i].get());
        }

        return w;
    }

    @Override
    public Tablica wycinek(int... wspolrzedne) throws BledneArgumenty {
        if (wspolrzedne.length != 2) throw new BledneArgumenty("zła liczba wspolrzednych");

        Wektor w;
        w = new Wektor(wspolrzedne[1]-wspolrzedne[0]+1);
        w.poziome=this.poziome;

        for(int i=0;i<this.m;i++){
            w.values[0][i]=this.values[0][wspolrzedne[0]+i];
        }

        return w;
    }




    @Override
    public String toString() {
        String s;
        if (poziome) s="poziomo: ";
        else s="pionowo: ";
        s += "[ ";
        for (int i=0;i<this.m;i++){
            s+=values[0][i].get()+" ";
        }
        s+="]";
        return s;
    }
}
