package totolotek.system;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ThreadLocalRandom;

public class Zaklad {
    public final int[] liczby;

    public Zaklad(int... dane) {
        this.liczby = new int[6];
        for (int i = 0; i < 6; i++) {
            this.liczby[i] = dane[i];
        }
        Arrays.sort(this.liczby);
    }

    public Zaklad(){ //generuje losowy zaklad
        int i = 0;
        int[] l = new int[6];
        Map<Integer, Boolean> mapa = new HashMap<>();
        while (i < 6) {
            int x = ThreadLocalRandom.current().nextInt(1, 50);
            if (!mapa.containsKey(x)) {
                mapa.put(x, true);
                l[i] = x;
                i++;
            }
        }
        Arrays.sort(l);
        this.liczby = l;
    }

    public int porownaj(Zaklad zaklad) {
        int odp = 0;
        int i = 0;
        int j = 0;
        while (i < 6 && j < 6) {
            if (this.liczby[i] == zaklad.liczby[j]) {odp++; i++; j++;}
            else if (this.liczby[i] > zaklad.liczby[j]) {j++;}
            else if (this.liczby[i] < zaklad.liczby[j]) {i++;}
        }
        return odp;
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof Zaklad)) return false;
        Zaklad zaklad = (Zaklad) o;
        for (int i = 0; i < 6; i++) {
            if (this.liczby[i] != zaklad.liczby[i]) return false;
        }
        return true;
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(this.liczby);
    }

    @Override
    public String toString() {
        String s = "";
        for (int i = 0; i < liczby.length; i++) {
            if (liczby[i] < 10) s+=" ";
            s+=liczby[i]+" ";
        }
        s+=" ";
        return s;
    }
}
