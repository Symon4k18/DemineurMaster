/**
 * @file   main.c, JeuMatrice
 * @author Simon Jourdenais
 * @date   25 Septembre 2019
 * @brief  Le programme est en fait un jeu de stratégie qui consiste à determiner
 *  l'emplacement des mines et de les isoler en découvrant toutes les cases autre
 *  que celles minées. L'utilisateur peut choisir de mettre un drapeau sur les cases 
 * de son choix, en occurence celles qu'il croit minées.
 * 
 * Le jeu place  m_niveau
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
/****************** VARIABLES GLOBALES ****************************************/


char m_tabMines[NB_LIGNE][NB_COL+1]; //Tableau des caractères affichés au LCD
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
    /******** début du code********************/

    initialisation();   //initialise différents registres
    init_serie();       //initialise d'autre registres pour activer le port série
    lcd_init();         //initialise le LCD
    menuAccueil();      //affiche le menuAccueil (# Labo et nom)

    initTabVue();      //initialise le tableau m_tabVue ( le rempli de tuiles )
    rempliMines(nbMines);   // place le nombre de mines voulu dans m_tabMines
    metToucheCombien();    // mets le chiffre correspondant au nombre de mines qu'une tuile touche pour chaque tuile s'il y a lieu    
    afficheTabVue();        //fait apparaitre sur le lcd le tableau m_tabVue
    lcd_montreCurseur();    //fait apparaitre le curseur
    while(1)
    {
        deplace(&posX,&posY);   //effectue un déplacement au coordonées x et y
        
        if(PORT_SW)         // Si le bouton est appuyé
        {
            finPartie=gagne(&nbMines);  //verifie si le joueur gagné et placela réponse dans finPartie
            if((!demine(posX,posY))||(finPartie==true)) // vérifie si le joueur a perdu ou a gagné
            {
                afficheTabMines();     //montre où sont les mines
                while(PORT_SW);     //reste sur l'écran des mines jusqu'à ce que le joueur appuie sur le bouton
                initTabVue();       // met des tuiles dans les tableau m_tabVue a nouveau
                rempliMines(nbMines);   // rempli le tableau m_tabMines du nombre de nbMines mines
                metToucheCombien(); // mets le chiffre correspondant au nombre de mines que la tuile touche pour chaque tuile s'il y a lieu
                afficheTabVue();    //fait apparaitre sur le lcd le tableau m_tabVue
            }
        }
        else if(!BUILTIN_SW)    // si le joueur appuie sur le bouton sur la carte de développement 
        {
            if(m_tabVue[posY-1][posX-1]==TUILE) // Si la case est une tuile, un drapeau apparait aux coordonées 
            {
                m_tabVue[posY-1][posX-1]=DRAPEAU;   //le drapeau est aussi écrit dans le tableau m_tabVue
                lcd_ecritChar(DRAPEAU);     
            }
            else if(m_tabVue[posY-1][posX-1]==DRAPEAU)// Si la case est un drapeau, une tuile apparait aux coordonées
            {
                m_tabVue[posY-1][posX-1]=TUILE;     //le tuile est aussi écrit dans le tableau m_tabVue
                lcd_ecritChar(TUILE);
            }
            __delay_ms(25);     //Anti-rebond
        }
        __delay_ms(100);    
    }
}

/**
 * @brief  Affichage de l'écran d'accueil ; Affiche le numéro du laboratoire 
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
    __delay_ms(1000);       //reste sur l'écran affiché pendant 1 seconde
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
        for(unsigned char j=0;j<NB_COL;j++)     //mets des tuiles dans toutes les espaces de m_tabvue
        {
            m_tabVue[i][j]=TUILE;
        }
    }
    for(unsigned char k=0;k<NB_LIGNE;k++)       //rajoute un '\0' a la fin des 4 tableau de 20 char, utile pour lcd_putMessage
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
    for(unsigned char i =0; i<NB_LIGNE;++i) //fait afficher le tableau m_tabVue sur le LCD
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
    for(unsigned char i =0; i<NB_LIGNE;++i) //fait afficher le tableau m_tabMines sur le LCD
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
        for(unsigned char j=0;j<NB_COL;j++)     // les deux boucles servent à parcourir toutes les deux dimensions du tableau tabMines et les remplir d'espaces
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
    char mineReste=nb;  // initialise le décompteur au nombre de mines totales
    
    videMines();        // rempli le tableau m_tabMines d'espaces pour vider le tableau de ses mines 
    
    while(mineReste!=0) //pendant qu'il reste encore des mines à placer
    {
        x=rand()%NB_LIGNE;  // trouve une position aléatoire pour la variable x
        y=rand()%NB_COL;    // trouve une position aléatoire pour la variable y
        if(m_tabMines[x][y]!=MINE)  // si le tableau n'a pas déjà une mine au coordonées trouvées aléatoirement
        {
            m_tabMines[x][y]=MINE;  // on place la mine au coordonées
            mineReste--;    // on enlève une mine a placer au décompteur
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
    char nombreTouche='0';      //initialise le compte du nombre de mine que la tuile au coordonées  touche a zéro ( valeur ASCII de zéro)
    
    if(m_tabMines[ligne][colonne]!=MINE)    // si il n'y a pas de mine dans le tableau m_tabMines au coordonées (ligne,colonne)
    {
        if(ligne!=3)        // si la ligne choisie n'est pas la derniere (la plus basse)
        {
            if(m_tabMines[ligne+1][colonne]==MINE)  //ajoute 1 a nombreTouche si la case au dessus de la case choisie est une mine
                nombreTouche++;     
        }
        if(ligne!=0)     // si la ligne choisie n'est pas la premiere (la plus haute)
        {
            if(m_tabMines[ligne-1][colonne]==MINE)  //ajoute 1 a nombreTouche si la case sous de la case choisie est une mine
                nombreTouche++;
        }
        if(ligne!=3&&colonne!=0)     // si la ligne choisie n'est pas la derniere (la plus basse) et que la colonne choisie n'est pas la première (extrême gauche) ; Pourrait être dans la première condition ( le premier if(ligne!=3) mais cause plus de confusion).Meme chose pour les conditions suivantes
        {
            if(m_tabMines[ligne+1][colonne-1]==MINE)    //ajoute 1 a nombreTouche si la case en haut et à gauche de la case choisie est une mine
                nombreTouche++;
        }
        if(ligne!=3&&colonne!=19)   // si la ligne choisie n'est pas la derniere (la plus basse) et que la colonne choisie n'est pas la dernière (extrême droite) 
        {
            if(m_tabMines[ligne+1][colonne+1]==MINE)    //ajoute 1 a nombreTouche si la case en haut et à droite de la case choisie est une mine
                nombreTouche++;
        }
        if(ligne!=0&&colonne!=0)     // si la ligne choisie n'est pas la premiere (la plus haute) et que la colonne choisie n'est pas la première (extrême gauche)
        {
            if(m_tabMines[ligne-1][colonne-1]==MINE)    //ajoute 1 a nombreTouche si la case en bas et à gauche de la case choisie est une mine
                nombreTouche++;
        }
        if(ligne!=0&&colonne!=19)    // si la ligne choisie n'est pas la premiere (la plus haute) et que la colonne choisie n'est pas la dernière (extrême droite) 
        {
            if(m_tabMines[ligne-1][colonne+1]==MINE)    //ajoute 1 a nombreTouche si la case en bas et à droite de la case choisie est une mine
                nombreTouche++;
        }
        if(colonne!=19)     // si la colonne choisie n'est pas la dernière (extrême droite)
        {
            if(m_tabMines[ligne][colonne+1]==MINE)  //ajoute 1 a nombreTouche si la case en dessous et à droite de la case choisie est une mine
                nombreTouche++;
        }
        if(colonne!=0)  // si la colonne choisie n'est pas la première (extrême gauche)
        {
            if(m_tabMines[ligne][colonne-1]==MINE)  //ajoute 1 a nombreTouche si la case en dessous à gauche de la case choisie est une mine
                nombreTouche++;
        }
    }
    return nombreTouche;    //retourne le nombre de mines touchées
}


/**
 * @brief Si la manette est vers la droite ou la gauche, on déplace le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    if(getAnalog(AXE_X)>180)       //si la valeur lue du potentiomètre de l'axe X est de plus de 180 on effectue un déplacement vers la gauche. Pour un meilleur contrôle,on laisse un jeu d'environ 50 autour de la valeur millieu (128)
    {
        (*x)=(*x)-1;        //déplace le curseur de 1 vers la gauche
        if((*x)<1)  // si le curseur "sort" de l'écran vers la gauche , il réapparait de la droite
            (*x)=20;
    }
    else if(getAnalog(AXE_X)<80)    //si la valeur lue du potentiomètre de l'axe X est de moins de 80 on effectue un déplacement vers la droite.
    {
        (*x)=(*x)+1;    //déplace le curseur de 1 vers la droite
        if((*x)>20)     // si le curseur "sort" de l'écran vers la droite , il réapparait de la gauche
            (*x)=1;
    }
    if(getAnalog(AXE_Y)<80)       //si la valeur lue du potentiomètre de l'axe Y est de moins de 80 on effectue un déplacement vers le haut.
    {
        (*y)=(*y)-1;    //déplace le curseur de 1 vers le haut
        if((*y)<1)      // si le curseur "sort" de l'écran vers le haut , il réapparait du bas
            (*y)=4;
    }
    else if(getAnalog(AXE_Y)>180) //si la valeur lue du potentiomètre de l'axe Y est de plus de 180 on effectue un déplacement vers le haut.
    {
        (*y)=(*y)+1;    //déplace le curseur de 1 vers le bas
        if((*y)>4)      // si le curseur "sort" de l'écran vers le bas , il réapparait du haut
            (*y)=1;
    }
    lcd_gotoXY((*x),(*y));      //déplace le curseur aux nouvelles coordonées
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
    if(m_tabVue[y-1][x-1]!=DRAPEAU) // si la case choisie n'est pas un drapeau
    {
        if(m_tabMines[y-1][x-1]==MINE) //si la case choisie est une mine
            return false;   // signale aux programme que le joueur a perdu 
        else if(m_tabMines[y-1][x-1]!=MINE && m_tabMines[y-1][x-1]!=' ')    // si la case choisie n'est pas une mine ni un espace
        {
            m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];  //mets dans m_tabVue le vrai contenu de la case  
            lcd_ecritChar(m_tabVue[y-1][x-1]);      //affiche le case dévoilée dans m_tabVue
        }
        else        //si la case choisie est un espace
            enleveTuilesAutour(x,y);    //affiche les tuiles adjacentes à la case
    }
    return true;        //signale au programme que le joueur n'a pas perdu
}

/*
 * @brief Dévoile les cases non minées autour de la tuile reçue en paramètre.
 * Cette méthode est appelée par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    unsigned char xTabCol=x-1;      //initialisation des coordonnées en X pour le tableau ( change de 1-20 à 0-19)
    unsigned char yTabLigne=y-1;     //initialisation des coordonnées en Y pour le tableau ( change de 1-4 à 0-3)


    for(signed char j=-1;j<=1;j++)  // valeur additionnée a la coordonée y pour vérifier les cases à gauche et à droite de la case choisie
    {
        if(yTabLigne==0&&j==-1) //Evalue si la ligne est la premiere ( 0 ) et part le compteur j à zero de facon à ne pas ecrire dans la ligne -1
            j=0;            
        for(signed char i=-1;i<=1;i++)   // valeur additionnée a la coordonée x pour vérifier les cases au dessus et en dessous de la case choisie
        {
            if(xTabCol==0&&i==-1)   //Evalue si la colonne est la premiere ( 0 ) et part le compteur i à zero de facon à ne pas ecrire dans la colonne -1
                i=0;
            
            if(m_tabVue[yTabLigne+j][xTabCol+i]==TUILE)       //verifie si la case a déja été révélée ou contient un drapeau 
            {
                lcd_gotoXY(x+i,y+j);
                m_tabVue[yTabLigne+j][xTabCol+i]=m_tabMines[yTabLigne+j][xTabCol+i];
                lcd_ecritChar(m_tabVue[yTabLigne+j][xTabCol+i]);
            }
            if(xTabCol==19&&i==0)   //Evalue si la colonne est la derniere ( 19 ) et termine le compteur i à zero de facon à ne pas ecrire dans la colonne 20 ( qui n'existe pas dans m_tabVue et m_tabMines))
                i=1;
            
        }
        if(yTabLigne==3&&j==0)  //Evalue si la ligne est la derniere ( 3 ) et termine le compteur j à zero de facon à ne pas ecrire dans la ligne 4 ( qui n'existe pas dans m_tabVue et m_tabMines))
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
    unsigned char compteurBombe=0;      //initialisation du compteur de mines isolées ( soit avec drapeau ou non)   //initialisation du compteur de tuiles qui n'est pas sur une mine restante 

    for(unsigned char i=0;i<NB_LIGNE;i++)         
    {
        for(unsigned char j=0;j<NB_COL;j++)
        {
            if((m_tabVue[i][j]==TUILE||m_tabVue[i][j]==DRAPEAU))    //parcours toutes les tuiles et compte le nombre de tuiles et drapeaux au total
            {
                compteurBombe++;
            }
        }
    }
    if(compteurBombe==(*pMines))        //si le compte total de tuiles et drapeau sont équivalent au nombre de mines totales, les mines sont tous trouvées
    {
        afficheGagne();     //affiche le message du gagnant 
        (*pMines)++;        //augmente le nombre de mines de 1
        return true;        //retourne vrai pour signaler que le joueur a gagné et qu'une nouvelle partie devra commencer
    }
    else 
        return false;   //retourne faux s'il reste des tuiles a enlever
}
/*
* @brief Affiche le message du gagnant
* @param Aucun
* @return Rien
*/
void afficheGagne(void)
{
    lcd_effaceAffichage();      
    lcd_gotoXY(5,2);
    lcd_putMessage("YOU WIN!");     //affiche le string "VOUS AVEZ GAGNEZ" en Anglais
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
