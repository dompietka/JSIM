public class Gracze {
    private Gracz[] gracze;
    private int liczbaGraczy;

    public Gracze(int liczbaGraczy, Gracz[] gracze){
        this.liczbaGraczy=liczbaGraczy;
        this.gracze=gracze;
    }

    public int getLiczbaGraczy(){
        return liczbaGraczy;
    }

    public Boolean czyMogaGracDalej(int stawka){
        for (int i=0;i<liczbaGraczy;i++)
        {
            if (gracze[i].getBudzet() < stawka) return false;
        }

        return true;
    }

    public void rozdaj(Talia talia){
        for (int i=0;i<liczbaGraczy;i++){
            Karta[] pomReka = new Karta[5];
            for (int j=0;j<5;j++)
                pomReka[j]=talia.dajKarte(i+(j*liczbaGraczy));
            gracze[i].nowaReka(pomReka);
        }
    }

    public Gracz gracz (int i){
        return gracze[i];
    }

    @Override
    public String toString(){
        String ans = "";
        for (int i=0;i<liczbaGraczy;i++){
            ans += "karty gracza " + i + ": " + gracze[i].getReka().toString(); 
        }
        return ans;
    }
}