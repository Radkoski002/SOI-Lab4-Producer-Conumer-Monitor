# SOI - Lab 4 - Monitory
Zadanie polegało na zaimplementowaniu problemu konsumenta i producenta za pomocą monitorów.
Mamy magazyn o danej pojemności i daną ilość producentów i konsumentów.
Każdy z nich produkuje/konsumuje losową ilość produktów z danego przedziału.
Wszystkie operacje (produkcja, konsumpcja i bieżący stan magazynu) są zapisywane do plików z logami.
Aby program działał należy przekazać mu następujące argumenty w dokładnie tej kolejności:
- pojemność magazynu
- ilość producentów
- ilość konsumentów
- minimalna ilość produktów którą może wyprodukować producent
- maksymalna ilość produktów którą może wyprodukować producent
- minimalna ilość produktów którą może zabrać konsument
- maksymalna ilość produktów którą może zabrać konsument

Program jest kompilowany za pomocą narzędzia ```CMake```