package tablice;

import exceptions.*;

public class Macierz extends Tablica{
    public Macierz(double [][] values) {
        super(true, 2, values.length, values[0].length);
        for (int i = 0; i < values.length; i++) {
            for (int j = 0; j < values[i].length; j++) {
                this.values[i][j]=new ModDouble(values[i][j]);
            }
        }
    }

    public Macierz(int n, int m){
        super(true, 2, n, m);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                values[i][j] = new ModDouble(0);
            }
        }
    }

    @Override
    public void dodaj(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar==0){
            for(int i=0;i<values.length;i++){
                for(int j=0;j<values[i].length;j++){
                    try {
                        this.values[i][j].set(this.values[i][j].get()+tablica.daj());
                    } catch (BledneArgumenty e) {
                        System.err.println("Nieoczekiwany błąd wewnetrzny");
                        System.exit(2);
                    }
                }
            }
        }
        if (tablica.wymiar==1){
            if (tablica.poziome==this.poziome){
                if (this.m!=tablica.m) throw new BledneArgumenty("zly ksztalt argumentu");
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        this.values[i][j].set(tablica.values[0][j].get()+this.values[i][j].get());
                    }
                }
            }
            if (tablica.poziome!=this.poziome){
                if (this.n != tablica.m) throw new BledneArgumenty("zly ksztalt argumentu");
                for (int i = 0; i < this.m; i++) {
                    for (int j = 0; j < this.n; j++) {
                        this.values[j][i].set(this.values[i][j].get()+tablica.values[0][j].get());
                    }
                }
            }
            return;
        }
        if (tablica.wymiar==2){
            if (tablica.poziome!=this.poziome && this.m == tablica.n && this.n == tablica.m ){
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        this.values[i][j].set(this.values[i][j].get()+tablica.values[j][i].get());
                    }
                }
                return;
            }
            if (tablica.poziome==this.poziome && this.n == tablica.n && this.m == tablica.m){
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        this.values[i][j].set(this.values[i][j].get()+tablica.values[i][j].get());
                    }
                }
                return;
            }
            throw new BledneArgumenty("zly ksztalt argumentu");
        }
    }

    @Override
    public Tablica suma(Tablica tablica) throws BledneArgumenty {
        Tablica m = this.kopia();
        try {
            m.dodaj(tablica);
        }
        catch (BledneArgumenty e) {
            throw e;
        }
        return m;
    }

    @Override
    public void przemnoz(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar==0){
            for(int i=0;i<values.length;i++){
                for(int j=0;j<values[i].length;j++){
                    try {
                        this.values[i][j].set(this.values[i][j].get()*tablica.daj());
                    } catch (BledneArgumenty e) {
                        System.err.println("Nieoczekiwany błąd wewnetrzny");
                        System.exit(2);
                    }
                }
            }
        }
        if (tablica.wymiar==1){throw new BledneArgumenty("zly ksztalt argumentu, nie da sie pomnozyc przez wektor bez zmiany ksztaltu");}
        if (tablica.wymiar==2){
            if (tablica.n!=tablica.m || this.n != this.m || this.n !=tablica.n)
                throw new BledneArgumenty("zly ksztalt argumentu, zeby nie zmienic ksztaltu obie musza byc tak samo kwadratowe");

            Tablica m = new Macierz(this.n, this.n);
            for (int i = 0; i < this.n; i++) {
                for (int j = 0; j < this.n; j++) {
                    double x = 0;
                    for(int k=0;k<this.n;k++){
                        x+=this.daj(i,j)*tablica.daj(j,i);
                    }
                    m.values[i][j].set(x);
                }
            }
            this.przypisz(m);
        }
    }

    @Override
    public Tablica iloczyn(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar==0){
            Tablica m = this.kopia();
            m.przemnoz(tablica);
            return m;
        }

        if (tablica.wymiar==2){
            int szer = this.m;
            int wys2 = this.n;
            if (!this.poziome) {szer=this.n; wys2=this.m;}
            int wys = tablica.n;
            int szer2=tablica.m;
            if (!tablica.poziome) {wys=tablica.m; szer2=tablica.n;}

            if (szer!=wys) throw new BledneArgumenty("zly ksztalt argumentu");

            Tablica m = new Macierz(wys2, szer2);
            for (int i = 0; i < wys2; i++) {
                for (int j = 0; j < szer2; j++) {
                    double x = 0;
                    for(int k=0;k<szer;k++){
                        x+=this.daj(i,j)*tablica.daj(j,i);
                    }
                    m.values[i][j].set(x);
                }
            }
            return m;
        }

        if (tablica.wymiar==1){
            if (tablica.poziome){
                if (this.n!=1 || this.m!=1) throw new BledneArgumenty("zly ksztalt argumentu");
                Tablica w = new Wektor (tablica.m);
                for (int i = 0; i < w.m; i++) {
                    w.values[0][i].set(this.values[0][0].get()*tablica.values[0][i].get());
                }
                return w;
            }
            if (!tablica.poziome){
                int szer = this.m;
                int wys2 = this.n;
                if (!this.poziome) {szer=this.n; wys2=this.m;}

                if (szer!=tablica.m) throw new BledneArgumenty("zly ksztalt argumentu");
                Tablica w = new Wektor(wys2);
                w.poziome=false;

                for(int i=0;i<wys2;i++){
                    double x=0;
                    for (int j=0;j<szer;j++){
                        x+=this.daj(i, j)*tablica.values[0][j].get();
                    }
                    w.values[0][i].set(x);
                }
                return w;
            }
        }

        throw new BledneArgumenty("zly ksztalt argumentu");
    }

    @Override
    public void zaneguj(){
        for (int i=0; i < n; i++){
            for (int j=0; j < m; j++){
                values[i][j].set(values[i][j].get()*(-1));
            }
        }
    }

    @Override
    public Tablica negacja(){
        Tablica m = this.kopia();
        m.zaneguj();
        return m;
    }

    @Override
    public void przypisz(Tablica tablica) throws BledneArgumenty {
        if (tablica.wymiar==0){
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    try {
                        values[i][j].set(tablica.daj());
                    } catch (BledneArgumenty e) {
                        System.err.println("Nieoczekiwany błąd wewnetrzny");
                        System.exit(2);
                    }
                }
            }
            return;
        }
        if (tablica.wymiar==2){
            if (tablica.poziome!=this.poziome && this.m == tablica.n && this.n == tablica.m ){
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        this.values[i][j].set(tablica.values[j][i].get());
                    }
                }
                return;
            }
            if (tablica.poziome==this.poziome && this.n == tablica.n && this.m == tablica.m){
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        this.values[i][j].set(tablica.values[i][j].get());
                    }
                }
                return;
            }
            throw new BledneArgumenty("zly ksztalt argumentu");
        }
        if (tablica.poziome==this.poziome){
            if (this.m!=tablica.m) throw new BledneArgumenty("zly ksztalt argumentu");
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    this.values[i][j].set(tablica.values[0][j].get());
                }
            }
        }
        if (tablica.poziome!=this.poziome){
            if (this.n != tablica.m) throw new BledneArgumenty("zly ksztalt argumentu");
            for (int i = 0; i < this.m; i++) {
                for (int j = 0; j < this.n; j++) {
                    this.values[j][i].set(tablica.values[0][j].get());
                }
            }
        }
    }

    @Override
    public Tablica wycinek(int... wspolrzedne) throws BledneArgumenty{
        if (wspolrzedne.length != 4) {throw new BledneArgumenty("za malo wspolrzednych");}

        if (!this.poziome){
            int pom1=wspolrzedne[0];
            int pom2=wspolrzedne[1];
            wspolrzedne[0]=wspolrzedne[2];
            wspolrzedne[1]=wspolrzedne[3];
            wspolrzedne[2]=pom1;
            wspolrzedne[3]=pom2;
        }
        Macierz m = new Macierz(wspolrzedne[1]-wspolrzedne[0]+1, wspolrzedne[3]-wspolrzedne[2]+1 );
        m.poziome=this.poziome;

        for (int i = 0; i < wspolrzedne[1]-wspolrzedne[0]+1; i++) {
            for (int j = 0; j < wspolrzedne[3]-wspolrzedne[2]+1; j++) {
                m.values[i][j]=this.values[wspolrzedne[0]+i][wspolrzedne[2]+j];
            }
        }

        return m;
    }

    @Override
    public Tablica kopia(){
        Macierz m = new Macierz(this.n,this.m);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                m.values[i][j].set(this.values[i][j].get());
            }
        }
        m.poziome=this.poziome;
        return m;
    }

    @Override
    public String toString() {
        String s="";
        int wys = this.n;
        int szer = this.m;
        if (!this.poziome){wys=this.m; szer = this.n;}
        for (int i = 0; i < wys; i++) {
            s+="[ ";
            for (int j = 0; j < szer; j++) {
                try {
                    s+=this.daj(i, j)+" ";
                } catch (BledneArgumenty e) {
                    System.err.println("Nieoczekiwany błąd wewnetrzny");
                    System.exit(2);
                }
            }
            s+="]\n";
        }
        return s;
    }

}
