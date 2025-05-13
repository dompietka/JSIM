public class Talia extends Karty {
    
    public Talia() {
        super(52);
        for (int i=0;i<13;i++) {
            for (int j=0;j<=4;j++) {
                karty[4*i+j] = new Karta(i+2,j);
            }
        }
    }

    public void tasuj() { 
        Karta[] pom = new Karta[52];
        for (int i=0;i<26;i++)
        {
            pom[i]=karty[i];
            pom[i+1]=karty[26+i];
        }

        int x = (int)(1+(50*Math.random()));

        for (int i=x;i<52;i++)
        karty[i-x]=pom[i];

        for (int i=0;i<x;i++)
        karty[52-x+i]=pom[i];

    }

    public Karta dajKarte(int i){
        return karty[i];
    }
}