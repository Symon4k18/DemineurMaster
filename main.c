/**
 * @file   main.c, JeuMatrice
 * @author Simon Jourdenais
 * @date   25 Septembre 2019
 * @brief  Le programme est en fait un jeu de strat�gie qui consiste � determiner
 *  l'emplacement des mines et de les isoler en d�couvrant toutes les cases autre
 *  que celles min�es. L'utilisateur peut choisir de mettre un drapeau sur les cases 
 * de son choix, en occurence celles qu'il croit min�es.
 * 
 * Le jeu place  m_niveau
 * 
 * 
 * 
 * @version 1.0 : Premi�re version
 * Environnement de d�veloppement: MPLAB X IDE (version 5.25)
 * Compilateur: XC8 (version 2.10)
 * Mat�riel: PicKIT4
 */

 /****************** Liste des INCLUDES ****************************************/
#include <xc.h>
#include <stdbool.h>  // pour l'utilisation du type bool
#include "serie.h"
#include "Lcd4Lignes.h"
#include <conio.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/********************** CONSTANTES *******************************************/
#define _XTAL_FREQ 1000000 //Constante utilis�e par __delay_ms(x). Doit = fr�q interne du uC
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define BUILTIN_SW PORTBbits.RB0 
#define TUILE 1 //caract�re cgram d'une tuile
#define MINE 2 //caract�re cgram d'une mine
#define DRAPEAU 3 // caract�re cgram d'un drapeau
#define MAXCASES 76
/********************** PROTOTYPES *******************************************/
void initialisation(void);
void menuAccueil(void);
void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
char calculToucheCombien(int ligne, int colonne);
void deplace(char* x, char* y);
bool demine(char x, char y);
void enleveTuilesAutour(char x, char y);
void videMines(void);
bool gagne(int* pMines);
char getAnalog(char canal);
void afficheTabVue(void);
void afficheTabMines(void);
void afficheGagne(void);
void openSpaces(int ligneTab, int colTab);
void verifieToucheEspace(int *tabVerif, int ligne, int colonne, unsigned char iCount);
void enleveEspaceAutour(char x, char y);

/****************** VARIABLES GLOBALES ****************************************/


char m_tabMines[NB_LIGNE][NB_COL+1]; //Tableau des caract�res affich�s au LCD
char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau contenant les mines, les espaces et les chiffres
/*               ***** PROGRAMME PRINCPAL *****                             */

/**
 * @brief  
 * @param  Aucun
 * @return Aucun
 */
void main(void)
{
    /*********variables locales du main********/
    unsigned char posY =NB_LIGNE/2;
    unsigned char posX =NB_COL/2;
    int nbMines=1;
    bool finPartie=false;
    /******** d�but du code********************/

    initialisation();   //initialise diff�rents registres
    init_serie();       //initialise d'autre registres pour activer le port s�rie
    lcd_init();         //initialise le LCD
    menuAccueil();      //affiche le menuAccueil (# Labo et nom)

    initTabVue();      //initialise le tableau m_tabVue ( le rempli de tuiles )
    rempliMines(nbMines);   // place le nombre de mines voulu dans m_tabMines
    metToucheCombien();     //place les chiffres autour des mines indiquant le nombre de mines auquelles elles touchent
    afficheTabVue();        //fait apparaitre sur le lcd le tableau m_tabVue
    lcd_montreCurseur();    //fait apparaitre le curseur
    while(1)
    {
        deplace(&posX,&posY);
        
        if(PORT_SW)
        {
            finPartie=gagne(&nbMines);
            if((!demine(posX,posY))||(finPartie==true))
            {
                afficheTabMines();     //Show m_tabMines
                while(!PORT_SW);
                initTabVue();
                rempliMines(nbMines);
                metToucheCombien();
                afficheTabVue();
            }
        }
        else if(!BUILTIN_SW)
        {
            if(m_tabVue[posY-1][posX-1]==TUILE)
            {
                m_tabVue[posY-1][posX-1]=DRAPEAU;
                lcd_ecritChar(DRAPEAU);
            }
            else if(m_tabVue[posY-1][posX-1]==DRAPEAU)
            {
                m_tabVue[posY-1][posX-1]=TUILE;
                lcd_ecritChar(TUILE);
            }
            __delay_ms(25);     //Anti-rebond
        }
        __delay_ms(100);
    }
}


/**
 * @brief  Affichage de l'�cran d'accueil ; Affiche le num�ro du laboratoire 
 * ainsi que mon nom
 * @param  Aucun
 * @return Aucun
 */
void menuAccueil(void)
{
    
    const char menuMsgs[2][20] = {"Laboratoire 6 ","Simon Jourdenais"};
    lcd_effaceAffichage();
    lcd_putMessage(menuMsgs[0]);    //affiche le premier tableau de char ( premier string ) 
    lcd_gotoXY(1,3);
    lcd_putMessage(menuMsgs[1]);     //affiche le deuxieme tableau de char ( deuxieme string ) 
    lcd_curseurHome();
    lcd_cacheCurseur();
    __delay_ms(1000);       //reste sur l'�cran affich� pendant 1 seconde
    lcd_effaceAffichage();
}

/*
 * @brief Rempli le tableau m_tabVue avec le caract�re sp�cial (d�finie en CGRAM
 *  du LCD) TUILE. Met un '\0' � la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    for(unsigned char i=0;i<NB_LIGNE;i++)
    {
        for(unsigned char j=0;j<NB_COL;j++)     //mets des tuiles dans toutes les espaces de m_tabvue
        {
            m_tabVue[i][j]=TUILE;
        }
    }
    for(unsigned char k=0;k<NB_LIGNE;k++)       //rajoute un \0 a la fin des 4 tableau de 20 char, utile pour lcd_putMessage
    {
        m_tabVue[k][20]='\0';
    }
}
 /**
 * @brief  Affiche les 4 tableaux de 20 de m_tabVue
 * @param  Aucun
 * @return Aucun
 */
void afficheTabVue(void)
{
    for(unsigned char i =0; i<NB_LIGNE;++i)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabVue[i]);
    }
}

/**
 * @brief  Affiche les 4 tableaux de 20 de m_tabMines
 * @param  Aucun
 * @return Aucun
 */
void afficheTabMines(void)
{
    for(unsigned char i =0; i<NB_LIGNE;++i)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabMines[i]);
    }
}
/**
 * @brief  Efface le tableau m_tabMines (rempli avec des espaces)
 * @param  Aucun
 * @return Aucun
 */
void videMines(void)
{
    for(unsigned char i=0;i<NB_LIGNE;i++)
    {
        for(unsigned char j=0;j<NB_COL;j++)
        {
            m_tabMines[i][j]=' ';
        }
    }
}
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caract�re MINE d�fini en CGRAM.
 * @param int nb, le nombre de mines � mettre dans le tableau 
 * @return rien
 */
void rempliMines(int nb)
{
    unsigned char x,y;
    char mineReste=nb;
    
    videMines(); 
    
    while(mineReste!=0)
    {
        x=rand()%NB_LIGNE;
        y=rand()%NB_COL;
        if(m_tabMines[x][y]!=MINE)
        {
            m_tabMines[x][y]=MINE;
            mineReste--;
        }
    }
}

/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche � 3 mines, alors la m�thode place le code ascii de 3 dans
 * le tableau. Si la case ne touche � aucune mine, la m�thode met le code
 * ascii d'un espace.
 * Cette m�thode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{ 
    char minesTouche=0; 
    
    for(unsigned char i=0;i<NB_LIGNE;i++)
    {
        for(unsigned char j=0;j<NB_COL;j++)
        {
            minesTouche=calculToucheCombien(i,j);
            if(minesTouche!='0'&&m_tabMines[i][j]!=MINE)
                m_tabMines[i][j]=minesTouche;
            else if(m_tabMines[i][j]!=MINE)
                m_tabMines[i][j]=' ';
        }
    }
        for(unsigned char k=0;k<NB_LIGNE;k++)
    {
        m_tabMines[k][20]='\0';
    }
}
 
/*
 * @brief Calcul � combien de mines touche la case.
 * @param int ligne, int colonne La position dans le tableau m_tabMines a v�rifier
 * @return char nombre. Le nombre de mines touch�es par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    char nombreTouche='0';
    if(m_tabMines[ligne][colonne]!=MINE)
    {
        if(ligne!=3)
        {
            if(m_tabMines[ligne+1][colonne]==MINE)
                nombreTouche++;
        }
        if(ligne!=0)
        {
            if(m_tabMines[ligne-1][colonne]==MINE)
                nombreTouche++;
        }
        if(ligne!=3&&colonne!=0)
        {
            if(m_tabMines[ligne+1][colonne-1]==MINE)
                nombreTouche++;
        }
        if(ligne!=3&&colonne!=19)
        {
            if(m_tabMines[ligne+1][colonne+1]==MINE)
                nombreTouche++;
        }
        if(ligne!=0&&colonne!=0)
        {
            if(m_tabMines[ligne-1][colonne-1]==MINE)
                nombreTouche++;
        }
        if(ligne!=0&&colonne!=19)
        {
            if(m_tabMines[ligne-1][colonne+1]==MINE)
                nombreTouche++;
        }
        if(colonne!=19)
        {
            if(m_tabMines[ligne][colonne+1]==MINE)
                nombreTouche++;
        }
        if(colonne!=0)
        {
            if(m_tabMines[ligne][colonne-1]==MINE)
                nombreTouche++;
        }
    }
    return nombreTouche;
}


/**
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    if(getAnalog(AXE_X)>180)       // On laisse un jeu de +- 50 (Treshold d'environ 39 %)
    {
        (*x)=(*x)-1;
        if((*x)<1)
            (*x)=20;
    }
    else if(getAnalog(AXE_X)<80)
    {
        (*x)=(*x)+1;
        if((*x)>20)
            (*x)=1;
    }
    if(getAnalog(AXE_Y)<80)       // On laisse un jeu de +- 50 (Treshold d'environ 39 %)
    {
        (*y)=(*y)-1;
        if((*y)<1)
            (*y)=4;
    }
    else if(getAnalog(AXE_Y)>180)
    {
        (*y)=(*y)+1;
        if((*y)>4)
            (*y)=1;
    }
    lcd_gotoXY((*x),(*y));
}
 
/*
 * @brief D�voile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derri�re les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    if(m_tabVue[y-1][x-1]!=DRAPEAU)
    {
        if(m_tabMines[y-1][x-1]==MINE&&m_tabVue[y-1][x-1]!=DRAPEAU)
            return false;
        else if(m_tabMines[y-1][x-1]!=MINE && m_tabMines[y-1][x-1]!=' ')
        {
            m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];
            lcd_ecritChar(m_tabVue[y-1][x-1]);
        }
        else
            enleveTuilesAutour(x,y);   
    }
    return true;
}

/*
 * @brief D�voile les cases non min�es autour de la tuile re�ue en param�tre.
 * Cette m�thode est appel�e par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    unsigned char xTabCol=x-1;
    unsigned char yTabLigne=y-1;    


    for(signed char j=-1;j<=1;j++)
    {
        if(yTabLigne==0&&j==-1)
            j=0;            //Evalue si la ligne est la premiere ( 0 ) et part le compteur i a zero de facon a ne pas ecrire dans la ligne -1
        for(signed char i=-1;i<=1;i++)
        {
            if(xTabCol==0&&i==-1) 
                i=0;
            
            if(m_tabMines[yTabLigne+j][xTabCol+i]!=MINE&&m_tabVue[yTabLigne+j][xTabCol+i]==TUILE)
            {
                lcd_gotoXY(x+i,y+j);
                m_tabVue[yTabLigne+j][xTabCol+i]=m_tabMines[yTabLigne+j][xTabCol+i];
                lcd_ecritChar(m_tabVue[yTabLigne+j][xTabCol+i]);
            }
            if(xTabCol==19&&i==0)
                i=1;
            
        }
        if(yTabLigne==3&&j==0)
            j=1;
    }

}

 
/*
 * @brief V�rifie si gagn�. On a gagn� quand le nombre de tuiles non d�voil�es
 * est �gal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagn�.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagn�, faux sinon
 */
bool gagne(int* pMines)
{
    unsigned char compteurBombe=0;
    unsigned char resteTuiles=0;
    unsigned char i=0;
    unsigned char j=0;
    while((i<4)||(j<20))
    {
        for(i;i<NB_LIGNE;i++)
        {
            for(j=0;j<NB_COL;j++)
            {
                if((m_tabVue[i][j]==TUILE||m_tabVue[i][j]==DRAPEAU)&&m_tabMines[i][j]==MINE)
                {
                    compteurBombe++;
                }
                else if((m_tabVue[i][j]==TUILE||m_tabVue[i][j]==DRAPEAU)&&m_tabMines[i][j]!=MINE)
                    resteTuiles++;
            }
        }
    }
    if(compteurBombe==(*pMines)&&(!resteTuiles))
    {
        afficheGagne();
        (*pMines)++;
        return true;
    }
    else 
        return false;
}
 
void afficheGagne(void)
{
    lcd_effaceAffichage();
    lcd_gotoXY(5,2);
    lcd_putMessage("YOU WIN!");
    lcd_gotoXY(6,3);
    lcd_putMessage("+1 Mine");
    __delay_ms(2500);
}
/*
 * @brief Lit le port analogique. 
 * @param Le no du port � lire
 * @return La valeur des 8 bits de poids forts du port analogique
 */

char getAnalog(char canal)
{ 
    ADCON0bits.CHS = canal;
    __delay_us(1);  
    ADCON0bits.GO_DONE = 1;  //lance une conversion
    while (ADCON0bits.GO_DONE == 1) //attend fin de la conversion
        ;
    return  ADRESH; //retourne seulement les 8 MSB. On laisse tomber les 2 LSB de ADRESL
}
 
 
 
/*
 * @brief Fait l'initialisation des diff�rents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 � RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN � on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement � gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB � gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fr�quence pour la conversion la plus longue possible)
 
}
