#include <iostream>
#include <random>
#include <bitset>
#include <iomanip>

using namespace std;

const float ZAKRES = 40.0f;
const int LICZEBNOSC_POPULACJI = 20;
const int LICZBA_CHROMOSOMOW = sizeof(float) * 8;
const float FUNKCJA[3] = {0.5, 6.09, -85.0};
const int MAKS_ITERACJI = 1000000;
const float DOKLADNOSC = 0.01f;
const float ODLEGLOSC_MIEDZY_PIERWIASTKAMI = 10.0f;


struct Osobnik {
    int nrOsobnika;
    float argument;
    int reprezentacja_bitowa[LICZBA_CHROMOSOMOW];
    float wartosc_funkcji;
    float wartoscFunkcji();
    Osobnik() {
        fill(reprezentacja_bitowa,reprezentacja_bitowa+LICZBA_CHROMOSOMOW,0); //zerowanie bitów, żeby było czysto
    }
};

Osobnik Populacja[LICZEBNOSC_POPULACJI];

float Osobnik::wartoscFunkcji() {
    wartosc_funkcji = FUNKCJA[0] * pow(argument, 2) + FUNKCJA[1] * argument + FUNKCJA[2];
}

void losowaniePopulacji(std::mt19937 random_engine) {
    uniform_real_distribution<float> dist(-ZAKRES, ZAKRES);
    for (int i = 0; i < LICZEBNOSC_POPULACJI; ++i) {
        Populacja[i].nrOsobnika = i;
        Populacja[i].argument = dist(random_engine);
        Populacja[i].wartoscFunkcji();
    }
}

void floatToByte(int nrOsobnika) {
    union {
        float input; // przyjmuje sizeof(float) == sizeof(int)
        int output;
    } data;
    data.input = Populacja[nrOsobnika].argument;
    bitset<sizeof(float) * CHAR_BIT> bits(data.output);
    for (int i = 0; i < LICZBA_CHROMOSOMOW; ++i) {
        Populacja[nrOsobnika].reprezentacja_bitowa[i] = bits[31 - i];
    }
}

float byteToFloat(int nrOsobnika) {
    unsigned int znak = Populacja[nrOsobnika].reprezentacja_bitowa[0];
    float mantysa = 0;
    int cecha = 0;
    const int BIAS = 127;
    for (int i = 1; i < 9; ++i) {
        cecha = cecha + Populacja[nrOsobnika].reprezentacja_bitowa[i] * pow(2, 8 - i);
    }
    for (int i = 9; i < 32; ++i) {
        mantysa = mantysa + Populacja[nrOsobnika].reprezentacja_bitowa[i] * pow(0.5f, i - 8);
    }
    cecha = cecha - BIAS;
    mantysa = 1.0f + mantysa;
    return pow(-1, znak) * mantysa * pow(2.0f, cecha);
} //Zwraca float, nie zapisuje nic

void swap(Osobnik *xp, Osobnik *yp) {
    Osobnik temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void sortowanie() {
    int n = sizeof(Populacja) / sizeof(Populacja[0]);
    int i, j;
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if (fabs(Populacja[j].wartosc_funkcji) > fabs(Populacja[j + 1].wartosc_funkcji))
                swap(&Populacja[j], &Populacja[j + 1]);
} //Bubble Sort, względem bliskości Populacja[].wartosc_funkcji do zera. Im bliżej tym niższy indeks w Populacja[];

void krzyzowanie(int pozycja1, int pozycja2) {
    int pozycja = rand()%(LICZBA_CHROMOSOMOW-1)+1; //+1 bo omijamy bit znaku... żeby jakos się trzymać przy tym, czego szukamy
    for (int i = pozycja; i < LICZBA_CHROMOSOMOW; i++) { // od losowego miejsca zamieniamy miejscami chromosomy osobników
        swap(Populacja[pozycja1].reprezentacja_bitowa[i], Populacja[pozycja2].reprezentacja_bitowa[i]);
    }
}

void mutacja(mt19937 random_engine) {
    uniform_int_distribution<int> dist_szansa(0, 100);
    uniform_int_distribution<int> dist_pop(0, (LICZEBNOSC_POPULACJI - 1));
    if (dist_szansa(random_engine) <= 5) {
        uniform_int_distribution<int> dist(0, LICZBA_CHROMOSOMOW - 1);
        int pozycja = dist(random_engine);
        int a = dist_pop(random_engine);
        if (Populacja[a].reprezentacja_bitowa[pozycja] == 0) {
            Populacja[a].reprezentacja_bitowa[pozycja] == 1;
        } else {
            Populacja[a].reprezentacja_bitowa[pozycja] == 0;
        }
    }
}

void Main_Loop(mt19937 random_engine) {
    mutacja(random_engine);
    sortowanie();

    int do_krzyzowania[LICZEBNOSC_POPULACJI/2];
    int czy_krzyzowane[LICZEBNOSC_POPULACJI];
    fill(do_krzyzowania,do_krzyzowania+LICZEBNOSC_POPULACJI/2,0);
    fill(czy_krzyzowane,czy_krzyzowane+LICZEBNOSC_POPULACJI,0);

    for (int i = 0; i < LICZEBNOSC_POPULACJI / 2; i++)
    {
        bool warunek=true;
        while (warunek)
        {
            int los = (rand()%LICZEBNOSC_POPULACJI/2);
            if (czy_krzyzowane[los]==0) {
                czy_krzyzowane[los]=1;
                do_krzyzowania[i]=los;
                warunek=false;
            }
        }
    }

    for (int i = 0; i < LICZEBNOSC_POPULACJI / 2; i++) { //pętla krzyżowania
        krzyzowanie(i, do_krzyzowania[i]);
    }

    for (int k = 0; k < LICZEBNOSC_POPULACJI; ++k) {
        Populacja[k].argument = byteToFloat(k);
        Populacja[k].wartoscFunkcji();
    }
}

bool check(int n, float *ARG1, float *ARG2, float *WAR1, float *WAR2) {
    //cout << fabs(Populacja[0].wartosc_funkcji) << endl;
        if (fabs(Populacja[0].wartosc_funkcji) < DOKLADNOSC) {
            *ARG1 = Populacja[0].argument;
            *WAR1 = Populacja[0].wartosc_funkcji;
            if (*WAR2==100.0) {
                *ARG2 = Populacja[0].argument;
                *WAR2 = Populacja[0].wartosc_funkcji;
                *ARG1 = 100.0;
                *WAR1 = 100.0;
                return true;
            }
            else if (fabs(*ARG1 - *ARG2) > ODLEGLOSC_MIEDZY_PIERWIASTKAMI) {
                    cout << "\nOto one, dwa wspaniale pierwiastki:\n";
                    return false;
            }
            else if (n < MAKS_ITERACJI) {
                /*if (fabs(*WAR1)<fabs(*WAR2)) //kod na podmienianie jednego pierwiastka na dokladniejszy, jezeli znaleziony
                {
                    *ARG2 = Populacja[0].argument;
                    *WAR2 = Populacja[0].wartosc_funkcji;
                }*/
                *ARG1 = 100.0;
                *WAR1 = 100.0;
                return true;
            }
        }
        else if (n>=MAKS_ITERACJI) {
            //cout << "Nie udalo sie znalezc pierwiastkow" << endl;
            return false;
        }
    return true;
}


int main() {
    cout << setprecision(9);
    mt19937 random_engine;
    random_engine.seed(time(NULL));
    srand(time(NULL));
    losowaniePopulacji(random_engine);
    cout << "Tablica poczatkowa:\n";
    for (int j = 0; j < LICZEBNOSC_POPULACJI; ++j) {
        floatToByte(j);
        cout << "Argument: " << Populacja[j].argument << "\t";
        cout << "Wartosc: " << Populacja[j].wartosc_funkcji << "\t";
        //cout << "Sprawdzenie byteToFloat: " << byteToFloat(j) << endl;
        for (int i = 0; i < LICZBA_CHROMOSOMOW; ++i) {
            cout << Populacja[j].reprezentacja_bitowa[i];
        }
        cout << endl;
    }
    int n = 0;
    bool warunek = true;
    float ARG1 = 100.0, WAR1 = 100.0;
    float ARG2 = 100.0, WAR2 = 100.0;
    while (warunek) {
        Main_Loop(random_engine);
        warunek = check(n, &ARG1, &ARG2, &WAR1, &WAR2);
        n++;
    }
    cout << "Najblizszy argument 1: " << ARG1 << endl;
    cout << "Wartosc: " << WAR1 << endl;
    cout << "Najblizszy argument 2: " << ARG2 << endl;
    cout << "Wartosc: " << WAR2 << endl;
    cout << "Iteracje: " << n;
    cout << endl;
    /*cout << endl << "Tablica koncowa: \n";
    for (int j = 0; j < LICZEBNOSC_POPULACJI; ++j) {
        cout << "Argument: " << Populacja[j].argument << "\t";
        cout << "Wartosc: " << Populacja[j].wartosc_funkcji << "\t";
        //cout << "Sprawdzenie byteToFloat: " << byteToFloat(j) << endl;
        for (int i = 0; i < LICZBA_CHROMOSOMOW; ++i) {
            cout << Populacja[j].reprezentacja_bitowa[i];
        }
        cout << endl;
    }*/
    cout << "\nNacisnij Enter, by kontynuowac...";
    cin.get();
    return 0;
}
