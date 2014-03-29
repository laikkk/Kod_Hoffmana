/* 
 * File:   main.c
 * Author: kamil
 * Sposob wywowalnia:
 * ./nazwaProgramu nazwaPliku.txt
 * pomocne:
 * http://edu.i-lo.tarnow.pl/inf/utils/002_roz/ol016.php
 * http://huffman.ooz.ie/?text=EAEACDGDGEGGFGDFGHGAGGBEEEDGHH
 * Created on 24 marzec 2014, 19:30
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ILOSC_ZNAKOW 128
#define SZER_EKR 140  /* szerokosc ekranu */
#define IL_POZ   8   /* ilosc poziomow drzewa, ktore beda wydrukowane   */
/* gwiazdka bedzie sygnalizowac istnienie nizszych  poziomow */
int miejsceWKolejce; // ta zmienna jest wskaznikiem na nowy wezel w kolejce

/* struktury danych do reprezentowania drzewa */
typedef struct wezel *Wskwezla; /* wskaznik na wezel drzewa  */

typedef struct wezel
{
    int znak;
    Wskwezla left, right, p;
    int f;
} Twezla; /* typ wezla */
/* drzewa z wartownikami: wezel wskazywany przez "nil" jest wartownikiem 
   zastepujacym NULL; dla korzenia pole "p" ma wartosc "nil"; 
   pole nil->p musi byc ustawione odpowiednio w przy usuwaniu 
 */
Wskwezla nil;

typedef struct Kody // Struktura trzyma jaki znak ma jaki kod Hoffmana
{
    char znak;
    char kod[16];
} Kody;


// <editor-fold defaultstate="collapsed" desc="Funkcje od drukowania etc">

void nilInit(void)
{
    /* funkcja inicjujaca nil; musi byc wywolana przed budowaniem drzewa */
    nil = (Wskwezla)malloc(sizeof (Twezla));
    nil->p = NULL;
    nil->left = nil->right = NULL;
}

void drukuj(Wskwezla w);
/* funkcja drukujaca drzewo binarne na ekranie (tutaj tylko deklaracja) */
/* funkcja drukuje drzewo o korzeniu "w"                                */

void drukujDot(Wskwezla r);
// generuje w plikach drzewo0.gv,  drzewo1.gv ...
// opis drzew o korzeniu r do wydrukowania przez program dot
// zlecenie "dot -Tpdf -O drzewo1.gv" utworzy plik "drzewo1.gv.pdf" 

/* ------------  implementacja ------------------------------------- */
char wydruk[IL_POZ+1][SZER_EKR];

void drukujost(Wskwezla w, int l, int p, int poziom)
{
    int srodek = (l+p)/2;
    if (w==nil) return;
    wydruk[poziom][srodek] = '*';
}

void drukujwew(Wskwezla w, int l, int p, int poziom)
{
    int srodek = (l+p)/2;
    int i, dl;
    char s[19];
    if (w==nil) return;
    //	        dl=sprintf(s,"\e[31m%+d\e[0m",w->klucz);
    dl = sprintf(s, "%c", (char)w->znak);
    for (i = 0; i<dl; i++)
        wydruk[poziom][srodek-dl/2+i] = s[i];
    if (++poziom<IL_POZ)
    {
        drukujwew(w->left, l, srodek, poziom);
        drukujwew(w->right, srodek+1, p, poziom);
    }
    else
    {
        drukujost(w->left, l, srodek, poziom);
        drukujost(w->right, srodek+1, p, poziom);
    }
}

void drukuj(Wskwezla w)
{
    int j, i;
    for (i = 0; i<=IL_POZ; i++)
        for (j = 0; j<SZER_EKR; j++)
            wydruk[i][j] = ' ';
    drukujwew(w, 0, SZER_EKR, 0);
    for (i = 0; i<=IL_POZ; i++)
    {
        for (j = 0; j<SZER_EKR; j++)
            putchar(wydruk[i][j]);
        printf("\n");
    }
}

void drukujKrawedz(FILE *plikwy, Wskwezla r, int z, Wskwezla syn, int zs)
{
    // wezel r o numerze z jest juz wydrukowany
    // teraz drukujemy jego syna "syn" o numerze zs oraz krawedz miedzy nimi
    if (syn==nil)
    {
        fprintf(plikwy, "%d [shape=circle, style=invisible, label=\"", zs);
        fprintf(plikwy, "%d ", 0);
        fprintf(plikwy, "\"]\n");
        fprintf(plikwy, "%d -- %d [style=invis];\n ", z, zs);
    }
    else
    {

        fprintf(plikwy, "%d [shape=circle, color=black, label=\"", zs);
        fprintf(plikwy, "%c ", (char)syn->znak);
        fprintf(plikwy, "\"]\n");
        fprintf(plikwy, "%d -- %d ;\n", z, zs);
    }
}

int rekDrukujDot(Wskwezla r, int z, FILE *plikwy)
{
    // drukuje drzewo o korzeniu r 
    // z  - numer wezla r
    // zwraca najwiekszy numer wezla w poddrzewie, ktorego jest korzeniem
    // zakladamy, ze korzen r jest juz wydrukowany
    int nz;
    if (r==nil) return z;
    else
    {
        drukujKrawedz(plikwy, r, z, r->left, z+1);
        nz = rekDrukujDot(r->left, z+1, plikwy);
        drukujKrawedz(plikwy, r, z, r->right, nz+1);
        nz = rekDrukujDot(r->right, nz+1, plikwy);
        return nz;
    }
}

void drukujDot(Wskwezla r)
{
    // generuje w plikach drzewo0.gv, dzrewo1.gv, ... 
    // opis drzewa o korzeniu r do wydrukowania przez program dot
    // zlecenie "dot -Tpdf -O drzewo1.gv" utworzy plik "drzewo1.gv.pdf" 
    static int gdzie = 0;
    char numer[10];
    char nazwap[20] = "drzewo";
    FILE *plikwy;
    snprintf(numer, 9, "%d", gdzie);
    strcat(nazwap, numer);
    strcat(nazwap, ".gv");
    plikwy = fopen(nazwap, "w");
    gdzie++;
    fprintf(plikwy, "graph drzewo{\n");
    fprintf(plikwy, "size = \"2,20\"");
    if (r!=nil)
    {
        fprintf(plikwy, "%d [shape=circle, color=black, label=\"", 0);
        fprintf(plikwy, "%c ", (char)r->znak);
        fprintf(plikwy, "\"]\n");
        rekDrukujDot(r, 0, plikwy);
    }
    fprintf(plikwy, "}\n");
    fclose(plikwy);
    printf("wydrukowane %s\n", nazwap);
}
// </editor-fold>

void liczWystapienia(char *nazwaPliku, int* wystapienia)
{
    FILE* plik;
    if ((plik = fopen(nazwaPliku, "r"))==NULL)//argv[1]
    {
        printf("Nie mogę otworzyć pliku %s\n", nazwaPliku);
        exit(1);
    }
    int i;
    for (i = 0; i<ILOSC_ZNAKOW; i++)
        wystapienia[i] = 0;
    int znak = getc(plik);
    while (znak!=EOF) //pętla odczytująca po jednym znaku z pliku
    {
        wystapienia[znak]++;
        //printf("%c", znak); //do napotkania znaku końca pliku EOF
        znak = getc(plik);
    }
    fclose(plik);
}

void drukujWystapienia(int* wystapienia)
{
    printf("Znak i liczba jego wystapien w tekscie\n");
    int i;
    for (i = 0; i<ILOSC_ZNAKOW; i++)
        if (wystapienia[i]!=0)
            printf("'%c' - %d\n", (char)i, wystapienia[i]);
}

Wskwezla ExtractMin(Wskwezla kolejka[], int rozmiar_kolejki)
{
    Wskwezla najmniejszy = (Wskwezla)malloc(sizeof (Twezla));
    int i;
    int doUsuniecia;
    for (i = 0; i<rozmiar_kolejki; i++)
        if (kolejka[i]!=nil)
        {
            najmniejszy = kolejka[i];
            doUsuniecia = i;
            break;
        }
    //   printf("I '%c' \n",(char)najmniejszy->znak);
    for (i = 0; i<rozmiar_kolejki; i++)
    {
        if (kolejka[i]!=nil)
            if ((najmniejszy->f)>(kolejka[i]->f))
            {
                najmniejszy = kolejka[i];
                doUsuniecia = i;
            }
    }
    //   printf("II '%c' \n",(char)najmniejszy->znak);
    kolejka[doUsuniecia] = nil; //usunac z kolejki//TODO poprawic wyszukiwania najmniejszego.
    return najmniejszy;
}

void drukujDostepne(Wskwezla kolejka[], int rozmiar_kolejki)
{
    int i;
    for (i = 0; i<rozmiar_kolejki; i++)
    {
        if (kolejka[i]!=nil)
            printf(" ['%c':%2d] ", kolejka[i]->znak, kolejka[i]->f);
    }
    printf("\n");
}

void DrukujKody(Kody kody[], int rozmiar)
{

    int i;
    for (i = 0; i<rozmiar; i++)
        printf("'%c' = %s\n", kody[i].znak, kody[i].kod);
}

void wypisz(int d, char kod[], int pozycja)
{
    if (d>0)
        wypisz(d/2, kod, pozycja+1);
    if (d>0)
    {
        kod[pozycja] = (char)(d%2)+48;
        // printf("%d", d%2);
    }
}

void dec2bin(long decimal, char *binary)
{
    int k = 0, n = 0;
    int neg_flag = 0;
    int remain;
    char temp[80];
    // take care of negative input
    if (decimal<0)
    {
        decimal = -decimal;
        neg_flag = 1;
    }
    do
    {
        remain = decimal%2;
        // whittle down the decimal number
        decimal = decimal/2;
        temp[k++] = remain+'0';
    }
    while (decimal>0);
    if (neg_flag)
        temp[k++] = '-'; // add - sign
    // reverse the spelling
    while (k>=0)
        binary[n++] = temp[--k];
    binary[n-1] = 0; // end with NULL
}

void KodyOrginalne(int ilosc_znakow_w_tekscie, Kody kody[], int wystapienia[])
{
    // printf("Kody 'orginalne %d'\n", ilosc_znakow_w_tekscie);
    int dlugoscKody = 1, iloscBitow = 0, i;
    while (ilosc_znakow_w_tekscie>dlugoscKody)
    {
        dlugoscKody *= 2;
        iloscBitow++;
    }
    char kod[15] = "";
    for (i = 0; i<iloscBitow; i++)
        strcat(kod, "0");

    int koddec = 0, j = 0, x;
    for (i = 0; i<ILOSC_ZNAKOW; i++)

        if (wystapienia[i]!=0)
        {

            char nowy[15] = "";
            strcpy(nowy, kod);
            char bin[15] = "";
            // strcpy(bin, kod);
            dec2bin(koddec, bin);
            //wypisz(koddec, bin, 0);
            // printf("stary = %s\n", bin);
            int dlstatego = strlen(bin)-1;
            for (x = iloscBitow-1; x>=0; x--)
            {
                if (dlstatego<0) break;
                if ((int)bin[dlstatego]==49)
                    nowy[x] = bin[dlstatego];
                dlstatego--;
            }
            // printf("stary = '%s' nowy = %s\n",bin, nowy);
            kody[j].znak = (char)i;
            strcpy(kody[j++].kod, nowy);
            koddec++;
        }
}
int wierszPodajKody = 0;

void PodajKody(Wskwezla W, Kody kody[], char kodznaku[], int ktoraPozycjawStringu)
{
    printf("wszedlem do funkcji W->znak = %c kod = %s\n", (char)W->znak, kodznaku);
    if (W->left==nil&&W->right==nil)
    {
        printf("zapisuje kod dla znaku = '%c' kod = %s wiersz = %d\n", W->znak, kodznaku, wierszPodajKody);
        kody[wierszPodajKody].znak = (char)W->znak;
        strcpy(kody[wierszPodajKody++].kod, kodznaku);
    }
    if (W->left!=nil)
    {
        printf("Ide w lewo\n");
        kodznaku[ktoraPozycjawStringu] = '0';
        PodajKody(W->left, kody, kodznaku, ktoraPozycjawStringu+1);
    }
    if (W->right!=nil)
    {
        printf("Ide w prawo\n");
        kodznaku[ktoraPozycjawStringu] = '1';
        PodajKody(W->right, kody, kodznaku, ktoraPozycjawStringu+1);
    }
}

void ustaw_Kody_Hoff(Wskwezla W, Kody kody[], char kodznaku[])
{
    if (W!=nil)
    {
        char kodlewy[15];
        strcpy(kodlewy, kodznaku);
        strcat(kodlewy, "0");
        ustaw_Kody_Hoff(W->left, kody, kodlewy);

        char kodprawy[15];
        strcpy(kodprawy, kodznaku);
        strcat(kodprawy, "1");
        ustaw_Kody_Hoff(W->right, kody, kodprawy);
        if (W->left==nil&&W->right==nil)
        {
            kody[wierszPodajKody].znak = (char)W->znak;
            strcpy(kody[wierszPodajKody++].kod, kodznaku);
        }
    }
}

int ilosc_znakow_w_tekscie(int wystapienia[])
{// zlicza ilosc znakow ktore sa w tabeli(tekscie)
    int i, ilosc_znakow_w_txt = 0;

    for (i = 32; i<ILOSC_ZNAKOW; i++)
        if (wystapienia[i]!=0)
            ilosc_znakow_w_txt++;
    return ilosc_znakow_w_txt;
}

/*Przygotowujemy znaki i ilosci ich wystapien. Wkladamy je do węzłów i umieszczamy w kolejce*/
void dodaj_do_kolejki(int _wystapienia[], Wskwezla kolejka[])
{
    int i, j = 0;
    for (i = 32; i<ILOSC_ZNAKOW; i++)
        if (_wystapienia[i]!=0)
        {
            Wskwezla w = (Wskwezla)malloc(sizeof (Twezla));
            w->p = nil;
            w->znak = i;
            w->f = _wystapienia[i];
            w->left = w->right = nil;
            kolejka[j++] = w;
        }
}

void algorytm_Hoffmana(int rozmiar_kolejki, Wskwezla kolejka[])
{
    /*Algorytm Hoffmana*/
    int i;
    for (i = 1; i<rozmiar_kolejki/2; i++)
    {
        drukujDostepne(kolejka, rozmiar_kolejki);
        Wskwezla x = ExtractMin(kolejka, rozmiar_kolejki);
        Wskwezla y = ExtractMin(kolejka, rozmiar_kolejki);
        //printf("\tx = '%c'\ty = '%c' \n", (char)x->znak, (char)y->znak);
        Wskwezla w = (Wskwezla)malloc(sizeof (Twezla));
        w->p = nil;
        w->znak = 35;
        w->f = x->f+y->f;
        w->left = x;
        w->right = y;
        kolejka[miejsceWKolejce++] = w; //Dodanie tego nowego wezla do kolejki      
    }
}

void kodowanie_pliku(char* zrodlo,char *cel,int ilosc_znakow_w_txt,Kody kody[]){
     FILE *zrodloF, *celF;
    if ((zrodloF = fopen(zrodlo, "r"))==NULL)//argv[1]
    {
        printf("Nie mogę otworzyć pliku %s\n", zrodlo);
        exit(1);
    }
    if ((celF = fopen(cel, "w"))==NULL)//argv[1]
    {
        printf("Nie mogę otworzyć pliku %s\n", cel);
        exit(1);
    }
    int i,znak = getc(zrodloF);
    while (znak!=EOF) //pętla odczytująca po jednym znaku z pliku
    {
        for (i = 0; i<ilosc_znakow_w_txt; i++)
            if (kody[i].znak==(char)znak)       
                fprintf(celF, "%s", kody[i].kod);

        znak = getc(zrodloF);
    }
    printf("Zapisalem plik %s\n",cel);
    fclose(zrodloF);
    fclose(celF);
    }

int main(int argc, char** argv)
{
    nilInit();
    int _wystapienia[ILOSC_ZNAKOW];
    liczWystapienia(argv[1], _wystapienia); //text.txt
    //drukujWystapienia(_wystapienia);
    int i, ilosc_znakow_w_txt = ilosc_znakow_w_tekscie(_wystapienia);
    Kody kody[ilosc_znakow_w_txt]; // Tu bd trzymana znaki i odpowiadajace im kody Hoffmana
    Kody Orginalne[ilosc_znakow_w_txt];//Tu bd trzymane kody do 'Orginalnych' znakow o stalej dlugosci
    KodyOrginalne(ilosc_znakow_w_txt, Orginalne, _wystapienia);//uzupelnia Tablice struktur 'Orginalnymi'/'Normalnymi' kodami

    int rozmiar_kolejki = ilosc_znakow_w_txt*2; // rozmiar kolejki 2 razy wiekszy bo od ilosc_znakow_w_txt beda znajdowac sie wezly zlaczone
    miejsceWKolejce = ilosc_znakow_w_txt; //wskazuje na wolne miejsce w kolejsce ,tam gdzie skonczyly dodawac sie wezly pojedynczych liter beda dodawac sie wezly zlozonych liter 
    Wskwezla kolejka[rozmiar_kolejki]; // kolejka trzymająca wezly

    for (i = 0; i<rozmiar_kolejki; i++) //'wyzerowanie' kolejki
        kolejka[i] = nil;

    dodaj_do_kolejki(_wystapienia, kolejka);

    algorytm_Hoffmana(rozmiar_kolejki, kolejka);

    printf("Po algorytmie\n");

    char kodznaku[15] = "";
    //Po algorytmie w kolejce powinnien znajdowac sie tylko jeden wezel<- korzen drzewa -> 
    //w petli sa tak naprwde powinien byc tylko jedno przejscie ifa, gdy if(true) do zmiennej <kody> funkcja <ustaw_Kody_Hoff> wpisze kody znakow
    for (i = 0; i<rozmiar_kolejki; i++)
        if (kolejka[i]!=nil)
        {
            printf("%d. '%c' = %d \n", i, (char)kolejka[i]->znak, kolejka[i]->f);
            drukuj(kolejka[i]); // drukuje drzewo 
            ustaw_Kody_Hoff(kolejka[i], kody, kodznaku);
        }
    
    printf("Kody Hoffmana\n");
    DrukujKody(kody, ilosc_znakow_w_txt);
    printf("Kody Orginalne\n");
    DrukujKody(Orginalne, ilosc_znakow_w_txt);

    // w plikach sa zakodowane zanki z odpowiedniej grupy kodow. Mozna porownać dlugosci!
    kodowanie_pliku(argv[1],"kodHoffmana",ilosc_znakow_w_txt,kody);
    kodowanie_pliku(argv[1],"kodOrginalne",ilosc_znakow_w_txt,Orginalne);

    return (EXIT_SUCCESS);
}

