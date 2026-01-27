import time
import pandas as pd
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.chrome.options import Options as ChromeOptions
import os

# ========== USTAWIENIA ==========
CSV_FILE = "grades.csv"
MOODLE_URL = "https://moodle.mimuw.edu.pl/mod/assign/view.php?id=166323&action=grading&status=submitted"
LOGIN_WAIT = 20

# ========= CSV =========
try:
    df = pd.read_csv(CSV_FILE)
    
    # Sprawdzenie i przygotowanie danych
    if 'Email' not in df.columns or 'Punkty' not in df.columns or 'Komentarz' not in df.columns:
        print("BŁĄD: Nagłówki 'Email', 'Punkty' lub 'Komentarz' nie zostały poprawnie odczytane.")
        print(f"Znalezione kolumny: {list(df.columns)}")
        raise KeyError("Niepoprawne kolumny w pliku CSV.")

    df['Email'] = df['Email'].astype(str).str.strip()
    df['Punkty'] = df['Punkty'].astype(str).str.strip().str.replace(',', '.', regex=False)
    df['Komentarz'] = df['Komentarz'].astype(str)
    
    try:
        df['Punkty_float'] = pd.to_numeric(df['Punkty'])
        print("Format liczbowy w kolumnie 'Punkty' jest poprawny (używa kropki).")
        df = df.drop(columns=['Punkty_float']) # Usuń kolumnę testową
    except ValueError:
        print("OSTRZEŻENIE: Po konwersji, kolumna 'Punkty' nadal zawierała nie-liczbowe dane.")
    
    data = df.set_index("Email").to_dict("index")
    print(f"✅ Pomyślnie wczytano {len(df)} wierszy z pliku {CSV_FILE}.")
    print(df.head())

except Exception as e:
    print(f"❌ Krytyczny błąd odczytu CSV. Sprawdź, czy używasz separatora przecinka (','): {e}")
    exit()

try:
    print("⬇️ Pobieranie/Sprawdzanie sterownika ChromeDriver...")
    # Używamy chrome_type="google-chrome" (jeśli zainstalowałaś Chrome przez .deb)
    service = Service(ChromeDriverManager(chrome_type="google-chrome").install())
    
    print("🚀 Uruchamianie przeglądarki Chrome...")

    chrome_options = ChromeOptions()
    
    # 💡 Nieagresywne, ale niezbędne flagi dla stabilności na Linuxie:
    chrome_options.add_argument("--no-sandbox")            # Wymagane, gdy proces jest uruchamiany przez Selenium/system
    chrome_options.add_argument("--disable-dev-shm-usage") # Rozwiązuje problem z brakiem pamięci w kontenerach/systemach Linux
    

    # Dodatkowa flaga, która jest często zalecana, ale nie jest agresywna:
    chrome_options.add_argument("--disable-extensions")    # Wyłącza rozszerzenia, które mogą blokować skrypt
    
    
    driver = webdriver.Chrome(service=service, options=chrome_options)
    
    # Możemy bezpiecznie przywrócić maximize_window() jako polecenie Selenium, 
    # zamiast używać agresywnej flagi startowej.
    driver.maximize_window() 
    
    driver.get(MOODLE_URL)
    # ========= LOGOWANIE MANUALNE =========
    print(f"=========================================")
    print(f"👉 ZALOGUJ SIĘ RĘCZNIE w otwartym oknie Chrome.")
    input(f" po zalogowaniu wróć tutaj i wciśnij Enter...")

except Exception as e:
    # ... (Obsługa błędu)
    print("\n" + "="*40)
    print("❌ BŁĄD. Sprawdź, czy nowa instalacja Chrome jest dostępna z terminala ('google-chrome').")
    print(e)
    print("="*40)
    exit()
    
    
znaleziono_licznik = 0    
for email, item in data.items():
    punkty = item['Punkty']
    komentarz = item['Komentarz']

    if not email or email.lower() == 'nan':
        continue

    try:
        # 1. Znajdź wiersz (TR) zawierający unikalny email studenta
        row_xpath = f"//tr[contains(., '{email}')]"
        student_rows = driver.find_elements(By.XPATH, row_xpath)
        
        if len(student_rows) == 0:
            print(f" [POMINIĘTO] Nie znaleziono studenta na aktualnej stronie: {email}")
            continue

        student_row = student_rows[0]

        # 2. Wpisanie OCENY
        grade_input = student_row.find_element(By.XPATH, ".//input[starts-with(@name, 'quickgrade_') and not(contains(@name, 'comments'))]")
        grade_input.clear()
        grade_input.send_keys(punkty)

        # 3. Wpisanie KOMENTARZA
        comment_input = student_row.find_element(By.XPATH, ".//textarea[starts-with(@name, 'quickgrade_comments_')]")
        
        if komentarz and komentarz.lower() not in ('nan', ''):
            komentarz_czysty = komentarz.strip().strip('"') 
            comment_input.clear()
            comment_input.send_keys(komentarz_czysty)

        print(f" [OK] Wpisano dla {email}: {punkty} pkt")
        znaleziono_licznik += 1
        time.sleep(0.1) 

    except Exception as e:
        print(f" [BŁĄD W PĘTLI] Problem przy studencie {email}: {e}")

# ========= ZAKOŃCZENIE I SPRZĄTANIE =========
print("\n" + "="*50)
print(f"✅ Zakończono wpisywanie. Uzupełniono dane dla {znaleziono_licznik} studentów.")
print("👉 WAŻNE: Przejrzyj oceny w przeglądarce i KLIKNIJ RĘCZNIE 'Zapisz wszystkie szybkie oceny'.")
print("====================================")

# Wstrzymanie skryptu, aby okno pozostało otwarte
input("Naciśnij ENTER w konsoli, aby ZAMKNĄĆ okno przeglądarki...")
driver.quit()