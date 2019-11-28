/**
 * @file   main.c, JeuMatrice
 * @author Simon Jourdenais
 * @date   25 Septembre 2019
 * @brief  Le programme est en fait un jeu d'adresse sur une matrice de DEL. 
 * 
 * 
 * 
 * @version 1.0 : Première version
 * Environnement de développement: MPLAB X IDE (version 5.25)
 * Compilateur: XC8 (version 2.10)
 * Matériel: PicKIT4
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
#define _XTAL_FREQ 1000000 //Constante utilisée par __delay_ms(x). Doit = fréq interne du uC
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define BUILTIN_SW PORTBbits.RB0 
#define TUILE 1 //caractère cgram d'une tuile
#define MINE 2 //caractère cgram d'une mine
#define DRAPEAU 3 // caractère cgram d'un drapeau
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


char m_tabMines[NB_LIGNE][NB_COL+1]; //Tableau des caractères affichés au LCD
char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau contenant les mines, les espaces et les chiffres
/*               ***** PROGRAMME PRINCPAL *****                             */
enum direction
{
    haut=0,
    bas,
    droite,
    gauche
};
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
    /******** début du code********************/

    initialisation();
    init_serie();
    lcd_init();
    menuAccueil();
    lcd_montreCurseur();
    
    initTabVue();
    rempliMines(nbMines);
    metToucheCombien();
    afficheTabVue();
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
 * @brief  
 * @param  Aucun
 * @return Aucun
 */
void menuAccueil(void)
{
    
    const char menuMsgs[2][20] = {"Laboratoire 6 ","Simon Jourdenais"};
    lcd_effaceAffichage();
    lcd_putMessage(menuMsgs[0]);
    lcd_gotoXY(1,3);
    lcd_putMessage(menuMsgs[1]);
    lcd_curseurHome();
    lcd_cacheCurseur();
    __delay_ms(1000);
    lcd_effaceAffichage();
}

/*
 * @brief Rempli le tableau m_tabVue avec le caractère spécial (définie en CGRAM
 *  du LCD) TUILE. Met un '\0' à la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    for(unsigned char i=0;i<NB_LIGNE;i++)
    {
        for(unsigned char j=0;j<NB_COL;j++)
        {
            m_tabVue[i][j]=TUILE;
        }
    }
    for(unsigned char k=0;k<NB_LIGNE;k++)
    {
        m_tabVue[k][20]='\0';
    }
}
 
void afficheTabVue(void)
{
    for(unsigned char i =0; i<NB_LIGNE;++i)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabVue[i]);
    }
}


void afficheTabMines(void)
{
    for(unsigned char i =0; i<NB_LIGNE;++i)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabMines[i]);
    }
}
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
 *  mine contiendront le caractère MINE défini en CGRAM.
 * @param int nb, le nombre de mines à mettre dans le tableau 
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
 * Si une case touche à 3 mines, alors la méthode place le code ascii de 3 dans
 * le tableau. Si la case ne touche à aucune mine, la méthode met le code
 * ascii d'un espace.
 * Cette méthode utilise calculToucheCombien(). 
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
 * @brief Calcul à combien de mines touche la case.
 * @param int ligne, int colonne La position dans le tableau m_tabMines a vérifier
 * @return char nombre. Le nombre de mines touchées par la case
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
void verifieToucheEspace(int *tabVerif, int ligne, int colonne, unsigned char iCount)
{
    unsigned char i=0;
    while(tabVerif[i]!='\0')
        i++;
    
    if(ligne!=3)
    {
        if(m_tabMines[ligne+1][colonne]==' '&&m_tabVue[ligne+1][colonne]!=' ')
        {
            tabVerif[i]=(((ligne+1)*100)+colonne);
            i++;
        }
    }
    if(ligne!=0)
    {
        if(m_tabMines[ligne-1][colonne]==' '&&m_tabVue[ligne-1][colonne]!=' ')
        {
            tabVerif[i]=(((ligne-1)*100)+colonne);
            i++;
        }
    }
    if(colonne!=19)
    {
        if(m_tabMines[ligne][colonne+1]==' '&&m_tabVue[ligne][colonne+1]!=' ')
        {
            tabVerif[i]=((ligne*100)+(colonne+1));
            i++;
        }
    }
    if(colonne!=0)
    {
        if(m_tabMines[ligne][colonne-1]==' '&&m_tabVue[ligne][colonne-1]!=' ')
        {
            tabVerif[i]=((ligne*100)+(colonne-1));
            i++;
        }
    }
        if(ligne!=3&&colonne!=0)
        {
            if(m_tabMines[ligne+1][colonne-1]==' '&&m_tabVue[ligne+1][colonne-1]!=' ')
            {
                tabVerif[i]=(((ligne+1)*100)+(colonne-1));
                i++;
            }
        }
        if(ligne!=3&&colonne!=19)
        {
            if(m_tabMines[ligne+1][colonne+1]==' '&&m_tabVue[ligne+1][colonne+1]!=' ')
            {
                tabVerif[i]=(((ligne+1)*100)+(colonne+1));
                i++;
            }        
        }
        if(ligne!=0&&colonne!=0)
        {
            if(m_tabMines[ligne-1][colonne-1]==' '&&m_tabVue[ligne-1][colonne-1]!=' ')
            {
                tabVerif[i]=(((ligne-1)*100)+(colonne-1));
                i++;
            }        
        }
        if(ligne!=0&&colonne!=19)
        {
            if(m_tabMines[ligne-1][colonne+1]==' '&&m_tabVue[ligne-1][colonne+1]!=' ')
            {
                tabVerif[i]=(((ligne-1)*100)+(colonne+1));
                i++;
                
            }        
        }
}
 void openSpaces(int ligne, int colonne)                //les valeurs sont celles du tableau (0-3,0-19)
{
    int caseVide[MAXCASES];
    int i=0;
    unsigned char xCoord;
    unsigned char yCoord;
    for(unsigned char j=0;j<MAXCASES;j++)
        caseVide[j]='\0';
    
    verifieToucheEspace(caseVide, ligne, colonne, i);   // Mets les Coordonnées des cases Vides autour de la case clické dans caseVide
    
    while(caseVide[i]!='\0')
    {
        
        xCoord=caseVide[i]/100;
        yCoord=caseVide[i]%100;
        verifieToucheEspace(caseVide, xCoord, yCoord, i);// Mets les Coordonnées des cases Vides autour de la case au coordonnées xCoord yCoord dans caseVide
        /*
        if(m_tabMines[xCoord][yCoord]!=MINE && m_tabVue[xCoord][yCoord]!=' ')
        {
            m_tabVue[xCoord][yCoord] = m_tabMines[xCoord][yCoord];
            lcd_ecritChar(m_tabVue[xCoord][yCoord]);
        }
        else
            enleveEspaceAutour(xCoord,yCoord); */ 
        i++;
    }
   
}
/**
 * @brief Si la manette est vers la droite ou la gauche, on déplace le curseur 
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
 * @brief Dévoile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derrière les tuiles (m_tabMines).
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
 * @brief Dévoile les cases non minées autour de la tuile reçue en paramètre.
 * Cette méthode est appelée par demine().
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
                if(m_tabMines[yTabLigne+j][xTabCol+i]==' ')
                    openSpaces((yTabLigne+j),(xTabCol+i));
                lcd_ecritChar(m_tabVue[yTabLigne+j][xTabCol+i]);
            }
            if(xTabCol==19&&i==0)
                i=1;
            
        }
        if(yTabLigne==3&&j==0)
            j=1;
    }

}
void enleveEspaceAutour(char x, char y)
{
    unsigned char xTabCol=x;
    unsigned char yTabLigne=y;    


    for(signed char j=-1;j<=1;j++)
    {
        if(y==0&&j==-1)
            j=0;            //Evalue si la ligne est la premiere ( 0 ) et part le compteur i a zero de facon a ne pas ecrire dans la ligne -1
        for(signed char i=-1;i<=1;i++)
        {
            if(x==0&&i==-1) 
                i=0;
            
            if(m_tabMines[y+j][x+i]!=MINE&&m_tabVue[y+j][x+i]==TUILE)
            {
                lcd_gotoXY((x+1+i),(y+1+j));
                m_tabVue[y+j][x+i]=m_tabMines[y+j][x+i];
                lcd_ecritChar(m_tabVue[y+j][x+i]);
            }
            if(x==19&&i==0)
                i=1;
            
        }
        if(y==3&&j==0)
            j=1;
    }

}
 
/*
 * @brief Vérifie si gagné. On a gagné quand le nombre de tuiles non dévoilées
 * est égal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagné.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagné, faux sinon
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
 * @param Le no du port à lire
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
 * @brief Fait l'initialisation des différents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 à RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN à on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement à gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB à gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fréquence pour la conversion la plus longue possible)
 
}
