public class ZnajomosciOsobistosc {

    private static int znajdzOsobistosc(int n, boolean[][] zna) {
        int kandydat = 0;

        //znalezienie kandydata
        for (int i = 1; i < n; i++) {
            if (zna[kandydat][i]) { //kandydat zna i
                kandydat=i;
            }
        }

        //sprawdzenie czy kandydat jest osobistością
        for (int i = 0; i < n; i++) {
            if (i != kandydat && (zna[kandydat][i] || !zna[i][kandydat])) {
                return -1;
            }
        }

        return kandydat;
    }

    public static void main(String[] args) {
        int n = Integer.parseInt(args[0]);
        if (args.length != 1+n*n) {
            System.err.println("Błąd: za mało danych");
            return;
        }

        boolean[][] zna = new boolean[n][n];

        for (int i = 0; i < args.length-1; i++) {
            zna[i/n][i%n]=args[i+1].equals("1");
        }

        int osobistosc = znajdzOsobistosc(n, zna);
        if (osobistosc == -1) {
            System.out.println("nie ma osobistości");
        }
        else {
            System.out.println("osobistość to osoba nr " + osobistosc);
        }


    }
}