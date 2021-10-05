# Memory management

## Zad 1. Allocation of a list with pointers to the blocks of memory containing a list of pointers

Zaprojektuj i przygotuj zestaw funkcji (bibliotekę) do zarządzania tablicą bloków, w których to blokach pamięci zapisywane są rezultaty operacji porównywania mergowania dwóch plików wiersz po wierszu (round robin: 1 wiersz pliku A, 1 wiersz pliku B itd.) sekwencji par plików — sekwencja
ich nazw jest parametrem funkcji.

Biblioteka powinna umożliwiać:
- Utworzenie tablicy wskaźników (tablicy głównej) — w tej tablicy będą przechowywane wskaźniki na wiersze zmergowanych plików — pierwszy element tablicy głównej zawiera wykaz wierszy pierwszej pary zmergowanych plików, drugi element dla drugiej pary, itd. Pojedynczy blok wierszy(element wskazywany z tablicy głównej), to tablica wskaźników na poszczególne wiersze w zmergowanym pliku
- Definiowanie sekwencji par plików
- Przeprowadzenie zmergowania (dla każdego elementu sekwencji) oraz zapisanie wyniku zmergowania do pliku tymczasowego
- Utworzenie, na podstawie zawartość pliku tymczasowego, bloku wierszy — tablicy wskaźników na wiersze, ustawienie w tablicy głównej (wskaźników) wskazania na ten blok; na końcu, funkcja powinna zwrócić indeks elementu tablicy (głównej), który zawiera wskazanie na utworzony blok
- Zwrócenie informacji o ilości wierszy w danym bloku wierszy
- Usunięcie, z pamięci, bloku (wierszy) o zadanym indeksie
- Usunięcie, z pamięci, określonego wiersza dla podanego bloku wierszy
- Wypisanie zmergowanych plików, z uwzględnieniem wcześniejszych usuniętych bloków wierszy / wierszy

Przykład — załóżmy, że sekwencja nazw plików zawiera tylko jedną parę ('a.txt', 'b.txt').
Zawartość pliku a.txt:
Litwo, Ojczyzno moja!
ty jesteś jak zdrowie
Ile cię trzeba cenić

Zawartość pliku b.txt:
Lorem ipsum dolor sit ame
Consectetur adipiscing elit
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua

Wynik wykonania zmergowania plików a.txt i b.txtLitwo, Ojczyzno moja!
Lorem ipsum dolor sit ame
ty jesteś jak zdrowie
Consectetur adipiscing elit
Ile cię trzeba cenić
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua
