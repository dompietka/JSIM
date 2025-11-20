import argparse
import sys
from pathlib import Path

# wypisuje wygenerowane ścieżki
def print_paths():
    print("Wygenerowane ścieżki:")
    for path in paths:
        print(path)
        
def generate_data(path):
    with open(path, 'w') as f:
        f.write("To jest przykładowa zawartość pliku.\n")

def get_data(path):
    print(f"Zawartość pliku {path}:")
    

# generuje ścieżkę; niezależnie od trybu, tworzy katalogi i pliki tylko gdy nie istnieją
def generate_paths(mies, dzien, pora, typ):
    
    paths = []
    for i in range(len(mies)):
        folder_path = Path("./dane") / mies[i] / dzien[i] / pora[i]
        folder_path.mkdir(parents=True, exist_ok=True)
        full_path = folder_path / f"dane.{typ[i]}"
        full_path.touch(exist_ok=True)
        paths.append(full_path)
        
    return paths

# wczytuje parametry programu do zmiennych
def obtain_paths():
    
    # definicje parametrów i flag
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--tworz", action='store_true')
    parser.add_argument("-o", "--odczyt", action='store_true')
    parser.add_argument("paths")

    # sprawdza, czy jest dokladnie jedna flaga spośród -t, -o
    args = parser.parse_args()
    if args.tworz and args.odczyt:
        print("Jednoczesne tworzenie i czytanie nie jest dostepne")
        return -1
    if not args.tworz and not args.odczyt:
        print("Potrzebna flaga -t lub -o")
        return -1

    # podział głównego parametru na miesiące, dni i pory
    paths = args.paths
    months = paths[paths.find("[")+1:paths.find("]")]
    paths = paths[paths.find("]")+1:]
    days = paths[paths.find("[")+1:paths.find("]")]
    paths = paths[paths.find("]")+1:]
    times = paths[paths.find("[")+1:paths.find("]")]

    # tablice z wartościami w kompaktowej formie
    monthtable = []
    daytable = []
    timetable = []

    # wypełnienie kompaktowych tablic
    while months.find(",") != -1:
        monthtable.append(months[:months.find(",")])
        months = months[months.find(",")+1:]
        if months[:1] == " ":
            months = months[1:]
    monthtable.append(months)

    while days.find(",") != -1:
        daytable.append(days[:days.find(",")])
        days = days[days.find(",")+1:]
        if days[:1] == " ":
            days = days[1:]
    daytable.append(days)

    while times.find(",") != -1:
        timetable.append(times[:times.find(",")])
        times = times[times.find(",")+1:]
        if times[:1] == " ":
            times = times[1:]
    timetable.append(times)

    # tablice z pełnym rozwinięciem
    mies = []
    dzien = []
    tydzien = ['pn', 'wt', 'śr', 'czw', 'pt', 'sb','nd'] # lista dni tygodnia

    # wypełnienie rozwiniętych tablic
    for index in range(len(monthtable)):

        # więcej niż jeden dzień tygodnia
        if daytable[index].find('-') != -1:
            start = daytable[index][:daytable[index].find('-')]
            end = daytable[index][daytable[index].find('-')+1:]
            try:
                start = tydzien.index(start)
                end = tydzien.index(end)
            except ValueError:
                print("nieznany dzień tygodnia")
                return -1
            while start%7 != end:
                mies.append(monthtable[index])
                dzien.append(tydzien[start])
                start = start+1
            mies.append(monthtable[index])
            dzien.append(tydzien[end])
        # tylko jeden dzień tygodnia
        else:
            mies.append(monthtable[index])
            if daytable[index] in tydzien:
                dzien.append(daytable[index])
            else:
                print("nieznany dzień tygodnia")
                return -1
            
    # uzupełnia/obcina timetable[] do odpowiedniej dlugosci, by użyć go jako pora[]
    if len(timetable) < len(mies):
        timetable.extend(['r',]*(len(mies) - len(timetable)))
    if len(timetable) > len(mies):
        timetable = timetable[:len(mies)]


    tryb = 1 if args.tworz else 0
    #zwraca tablice z danymi o ścieżkach
    return [mies, dzien, timetable]

#glowna tresc programu
tryb = 0 #usawiane na 0 jesli odczyt, 1 jesli tworzenie
sciezki = obtain_paths()
if sciezki == -1:
    sys.exit()
paths = generate_paths(*sciezki, ['txt']*len(sciezki[0]))

print_paths()

if (tryb == 0):
    print("\nodczytane pliki:")
    for path in paths:
        get_data(path)
else:
    for path in paths:
        generate_data(path)
        