package lab5;

public class Student {
    private String imie;
    private String nazwisko;
    private int rok;
    private int nrIndeksu;
    private String [] przedmioty;
    private int ile;

    Student(String imie, String nazwisko, int rok, int nrIndeksu, String[] przedmioty) {
        assert przedmioty.length <= 50 :"za dużo przedmiotów";
        assert rok >= 1 && rok <= 5 :"za dlugo studiuje";
        assert imie != null : "nie pytaja cie o imie";
        assert nazwisko != null : "nie pytaja cie o nazwisko";

        this.imie = imie;
        this.nazwisko = nazwisko;
        this.rok = rok;
        this.nrIndeksu = nrIndeksu;


        //przepisujemy przedmioty do tablicy size 50
    }

    Student(String imie, String nazwisko, int rok, int nrIndeksu) {
        this(imie, nazwisko, rok, nrIndeksu, new String[] {"PO"});
    }

    public void dodajPrzedmiot(String przedmiot) {
        assert ile<=49;
        przedmioty[ile++] = przedmiot;
    }

    public void usunPrzedmiot(String przedmiot) {
        assert przedmiot != null;
        przedmioty.removeAt(przedmioty.find(przedmiot)) = przedmiot;

        //
    }
}


@override ???? 