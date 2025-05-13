public abstract class Karty{
    protected Karta[] karty;
    protected int liczbaKart;

    public Karty(int liczbaKart){
        this.liczbaKart=liczbaKart;
        karty = new Karta[liczbaKart];
    }

    public int nrNajwyzszej(){
        int ans = 0;
        for (int i=1;i<liczbaKart;i++){
            if (karty[i].porownaj(karty[ans])) ans = i;
        }
        return ans;
    }
    
    public Karta najwyzsza(){
        return karty[this.nrNajwyzszej()];
    }

    @Override
    public String toString(){
        String s = "";
        for (int i=0;i<liczbaKart;i++)
            s+= karty[i].toString() + " ";
        return s;
    }

}