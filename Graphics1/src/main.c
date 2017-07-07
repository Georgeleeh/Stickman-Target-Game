/* BUGS */
/* BUG: -nan() freakout when wind causes projectile to move backwards */
/* temp fix: setting windAcc to always be 0 or above*/

/* A Stickman target game which gives you three times  */

 #include <graphics_lib.h>
 #include <math.h>
 #include <time.h>
 #include <string.h>

/* ---------------------- DEFINE CONSTANTS ---------------------- */

 /* window dimensions */
 #define YWINDOW 400
 #define XWINDOW 700
 /* floor height = yWindow - yFloor */
 #define YFLOOR 300
 /* determines target size */
 #define BLUERADIUS 60
 /* set number of allowed shots */
 #define SHOTS 5
 /* set how many top scores are listed */
 #define HISCORES 5

/* --------------------- DEFINE STRUCTURES ---------------------- */

/* player data */
typedef struct
{
    /* holds the player's name */
    char name[4];
    /* holds current total score */
    int score_total;
} PlayerData_t;

/* coordinates */
typedef struct
{
    /* x and y coordinate */
    int x,y;
} Coordinates_t;

/* --------------------- DEFINE SUBROUTINES --------------------- */

/* returns a '1' if the score is a new hi score */
 int CheckHiScore(int scores[],int scoretotal)
 {
    FILE *fp;
    PlayerData_t filePlayers[HISCORES];
    int i,boolean = 0;

    /* open the file for appending in case it doesn't exist */
    fp = fopen("hiscores.txt","a");
    fclose(fp);

    /* open file for reading */
    fp = fopen("hiscores.txt","r");
    /* loop hi scores to read names and scores to arrays */
    for (i = 0; i < HISCORES; i++)
    {
        fscanf(fp, "%3s,%d", filePlayers[i].name,&filePlayers[i].score_total);
    }
    /* check if player's total score is a new hi score */
    for (i = 0; i < HISCORES; i++)
    {
        /* return a 1 if user score is new hi score */
        if (scoretotal > filePlayers[i].score_total)
        {
            boolean = 1;
        }
    }
    fclose(fp);

    return boolean;
 }

/* draws the hi score table on screen */
 int DrawHiScores(int scores[])
 {
    FILE *fp;
    PlayerData_t filePlayers[HISCORES];
    int i;
    int linespace = 20;
    char strScreenText[40];

    /* draw rectangle for sky */
	GFX_DrawFilledRectangle(0,YFLOOR,XWINDOW,0,LIGHTCYAN);
	/* draw yFloor */
	GFX_DrawFilledRectangle(0,YFLOOR,XWINDOW,YWINDOW,GREEN);

	GFX_DrawText(XWINDOW/2 - 40,YWINDOW-YFLOOR/2-100-linespace,"HI SCORES");

    /* open file for reading */
    fp = fopen("hiscores.txt","r");
    /* give error message and quit if opening file failed */
    if (fp == NULL)
    {
        printf("I'm sorry, Dave. I'm afraid I can't read the file.");
        exit(0);
    }
    /* loop hi scores to read names and scores to arrays */
    for (i = 0; i < HISCORES; i++)
    {
        fscanf(fp, "%3s,%d", filePlayers[i].name,&filePlayers[i].score_total);
        /* printf("name: %3s, score: %d\n",strNames[i],filescore[i]); */
        snprintf(strScreenText,40,"%d - %3s: %d",i+1,filePlayers[i].name,filePlayers[i].score_total);
        GFX_DrawText(XWINDOW/2 - 50,YWINDOW-YFLOOR/2-100+i*linespace,strScreenText);
    }

    fclose(fp);
    GFX_UpdateDisplay();
    return 0;
 }

/* initialise the window looking nicely */
int DrawGameBackground(int tries,int wind)
{
    /* a string for writing windspeed onscreen */
    char strWindSpeed[9];

    /* draw a yFloor 100 pixels from the bottom of the window */
	GFX_MoveTo(0,YFLOOR);
	/* draw rectangle for sky */
	GFX_DrawFilledRectangle(0,YFLOOR,XWINDOW,0,LIGHTCYAN);
	/* draw yFloor */
	GFX_DrawFilledRectangle(0,YFLOOR,XWINDOW,YWINDOW,GREEN);
    /* draw an instruction on the first fire */
    if (tries == 0)
    {
        /* draw an instruction on screen */
        GFX_DrawText(222,25, "Click anywhere to fire the bow");
        GFX_DrawText(217,45, "Use A and D to move the stickman");
        GFX_DrawText(175,65, "Watch for economic winds affecting your aim!");
    }
    /* write windspeed */
    GFX_SetColour(BLUE);
    snprintf(strWindSpeed,9,"Wind: %d",wind);
    GFX_DrawText(XWINDOW - 90,25,strWindSpeed);

    GFX_UpdateDisplay();
    return 0;
}

/* draw the path of a projectile thrown by a stick man and return cursor to stickhand */
int DrawProjectilePath(Coordinates_t coords,int xTarget,int wind)
{
    /* constant for scaling down magnitude */
    const int magscale = 3;
    /* define gravity */
    const float gravity = 9.81;
    /* SUVAT variables */
    float mag,ang,Ux,Vx,Vy,T;
    /* coordinates */
    Coordinates_t initPCoords,PCoords,clickCoords;
    /* boolean */
    int done;
    /* distances */
    int distance,absdistance;
    /* misc */
    int xAppleHitbox;
    char strDistance[26];

    /* takes stickmans left foot coords and converts to right hand coords */
	PCoords.x = coords.x + 60;
    PCoords.y = coords.y - 70;
    /* set initial projectile position to stickmans hand */
    initPCoords.x = PCoords.x;
    initPCoords.y = PCoords.y;
    /* get hitbox upperleft coord */
    /* hitbox extends 20 right and down from that point */
    xAppleHitbox = xTarget + BLUERADIUS + 40;

    /* move to stickmans hand */
    GFX_MoveTo(PCoords.x,PCoords.y);

    GFX_SetColour(RED);

    /* get coordinates from click */
    GFX_GetMouseCoordinates(&clickCoords.x,&clickCoords.y);
    GFX_DrawCircle(clickCoords.x,clickCoords.y,2,2);

    /* ---------- PROJECTILE MOTION ---------- */

    /* use PCoords.ythagoras to turn mouse coords and stickman hand into magnitude */
    mag = sqrt(pow((clickCoords.x - initPCoords.x),2) + pow((clickCoords.y - initPCoords.y),2));
    /* scale down magnitude for playability */
    mag /= magscale;
    /* use trig to get launch angle in radians */
    ang = atan( fabs(clickCoords.y - initPCoords.y) / fabs(clickCoords.x - initPCoords.x));
    /* convert magnitude and angle to x and y velocity components */

    Vx = mag * cos(ang);
    Vy = mag * sin(ang);

    done = 0;
    do
    {
        /* T = (PCoords.x-initPCoords.x) / Vx; */
        /* calculate initial velocity before time */
        Ux = sqrt( Vx*Vx + 2*wind*(PCoords.x-initPCoords.x) );
        /* calculate time at each column of pixels */
        T = (PCoords.x-initPCoords.x) / ((Ux + Vx) / 2);
        /* use time to find projectiles height on the column */
        PCoords.y = initPCoords.y - (Vy * T) + (gravity * T*T)/2;

        /* draw small connected line segments to generate arc of motion */
        GFX_DrawLineTo(PCoords.x,PCoords.y,1);
        /* increment to evaluate next column of pixels */
        PCoords.x++;

        /* -------- PROJECTILE HIT CHECKS -------- */

        /* floor hit */
        if (PCoords.y > YFLOOR)
        {
            /* if the object would go below the yFloor, set its height to the yFloor height */
            PCoords.y = YFLOOR;
            done = 1;
        }
        /* edge of screen hit */
        if (PCoords.x > XWINDOW)
        {
            done = 1;
        }
        /* stickman hit / killed */
        /* the box is a rectangle the stickmans size */
        if (xTarget+BLUERADIUS+30 <= PCoords.x && PCoords.x <= xTarget+BLUERADIUS+70 && YFLOOR - 130 <= PCoords.y && PCoords.y <= YFLOOR)
        {
            done = 1;
        }
        /* red box on stickman head hit */
        /* check if projectile is inside the box */
        if (xAppleHitbox <= PCoords.x && PCoords.x <= (xAppleHitbox+20) && PCoords.y > YWINDOW - 250)
        {
            done = 1;
        }

        GFX_UpdateDisplay();
    /* loop untill the projectile hits a hitbox, the floor or a window boundary */
    } while (!done);

    /* get distance from projectile to target */
    distance = PCoords.x - xTarget;
    /* print absolute distance */
    absdistance = abs(distance);

    /* add distance value to string */
    snprintf(strDistance,26,"Distance from target: %d",absdistance);
    GFX_SetColour(BLUE);
    /* print distance on screen */
    GFX_DrawText(20,100,strDistance);

    /* use non absolute distance to avoid +/- duplicates of distances */
    return distance;
}

/* draw a graph of the score */
int DrawScoreGraph(Coordinates_t coords,int scores[],int tries)
{
    /* margin for drawing axis */
    const int margin = 10;
    /* used to set where vertically the x axis sits on screen */
    const int offset = YWINDOW/2;
    /* constant for scaling score such that highest or lowest possible score remains onscreen */
    const float graphscale = (float) (YWINDOW/2-margin) / (60*SHOTS);
    /* count for loops */
    int i;
    /* string to write score */
    char strScore[32];

    GFX_ClearWindow();

    /* choose white pen colour */
	GFX_SetColour(WHITE);

    /* Set up some axis */
	coords.x = margin;
	coords.y = YWINDOW - coords.x;
    /* move to origin and draw axis */
	GFX_MoveTo(coords.x,coords.y);
	GFX_DrawLineTo(coords.x,10,2);
	GFX_MoveTo(coords.x,offset);
	GFX_DrawLineTo(XWINDOW - margin,offset,2);

    /* choose red pen colour */
	GFX_SetColour(RED);

	/* move to the middle of the y axis */
    coords.y = offset;
    GFX_MoveTo(coords.x,coords.y);

	/* loop as many times as user has attempts */
	/* +1 converts 0-starting arrays to 1-starting comparitors */
    for (i = 0; i < tries + 1; i++)
    {

        coords.x += (XWINDOW-2*margin)/SHOTS;
        /* scale the score to fit on screen better */
        coords.y -= scores[i] * graphscale;

        /* draw score line (coords.y - positive number will go up) */
        GFX_DrawLineTo(coords.x, coords.y, 2);
        GFX_DrawCircle(coords.x,coords.y,3,1);

	}
	/* choose white pen colour */
	GFX_SetColour(WHITE);

    /* rescale cursor position to give original score */
    coords.y /= graphscale;
	/* get total for drawing */
	snprintf(strScore,32,"Your stock's value: %d points",YWINDOW - coords.y);
	/* draw title */
	GFX_DrawText(XWINDOW/2-120,10,strScore);
	/* get total for drawing */
	snprintf(strScore,32,"Change: %d points",scores[tries]);
	/* draw title */
	GFX_DrawText(XWINDOW/2-70,25,strScore);

	GFX_UpdateDisplay();

	return 0;
}

/* startup screen */
int DrawStartupScreen(int xTarget)
{
    char strTries[19];
    /* draw rectangle for sky */
	GFX_DrawFilledRectangle(0,YFLOOR,XWINDOW,0,LIGHTCYAN);
	/* draw yFloor */
	GFX_DrawFilledRectangle(0,YFLOOR,XWINDOW,YWINDOW,GREEN);

    /* format string for printing */
	snprintf(strTries,19,"You have %d tries",SHOTS);

    /* draw title text */
    GFX_SetColour(BLACK);
    GFX_DrawText(225,30,"The Stock market Target Game!");
    GFX_DrawText(170,50, "Hit the target to raise your stock's value!");
    GFX_DrawText(150,70, "Hit the apple if you dare but avoid the banker!");
    GFX_DrawText(280,90, strTries);

    GFX_UpdateDisplay();

    return 0;
}

/* gets all the stats from the distances */
int DrawStatistics(int distances[],int scores[])
{
    /* stats variables */
    int scoretotal = 0,distancetotal = 0,mean,max = 0;
    /* set high for min calculation loop */
    int min = XWINDOW;
    int i;
    /* define string to print statistics */
    /* longest string = "Mean Distance: " + 4 digits + null */
    char strStatistics[20];

    /* loop through distances */
    for (i = 0; i < SHOTS; i++)
    {
        /* calculate totals */
        scoretotal += scores[i];
        distancetotal += distances[i];
        /* calculate max */
        if (distances[i] > max)
        {
            max = distances[i];
        }
        /* calculate min */
        if (distances[i] < min)
        {
            min = distances[i];
        }
    }

    /* calculate mean */
    mean = distancetotal / SHOTS;

    /* draw background */
    GFX_DrawFilledRectangle(0,0,XWINDOW,YWINDOW,LIGHTCYAN);

    GFX_SetColour(RED);
    /*put stats into strings */
    snprintf(strStatistics,21,"Total Score: %d",scoretotal);
    /* write stats on screen */
    GFX_DrawText(XWINDOW/2 - 100,20,strStatistics);
    GFX_SetColour(BLACK);
    snprintf(strStatistics,21,"Mean Distance: %d",mean);
    GFX_DrawText(XWINDOW/2 - 100,40,strStatistics);
    snprintf(strStatistics,21,"Max Distance: %d",max);
    GFX_DrawText(XWINDOW/2 - 100,60,strStatistics);
    snprintf(strStatistics,21,"Min Distance: %d",min);
    GFX_DrawText(XWINDOW/2 - 100,80,strStatistics);

    return scoretotal;
}

/* draw a stick man with a bow from the bottom left upwards 111 pixels tall */
int DrawStickArcher(Coordinates_t coords)
{
    /* choose pen colour */
	GFX_SetColour(BLACK);

    /* move to specified start coords */
    GFX_MoveTo(coords.x,YFLOOR);

    /* draw left foot to hips, leaving cursor at hips */
    GFX_DrawLineTo(coords.x+20, coords.y-40, 2);
    /* draw hips to right foot, leaving cursor at foot */
    GFX_DrawLineTo(coords.x+40, coords.y, 2);
    /* move to hips */
    GFX_MoveTo(coords.x+20, coords.y-40);
    /* draw hips to neck, leaving cursor at neck */
    GFX_DrawLineTo(coords.x+20,coords.y-90,2);
    /* draw head, leaving cursor in the centre */
    GFX_DrawCircle(coords.x+20,coords.y-110,20,2);
    GFX_MoveTo(coords.x+20,coords.y-80);
    /* move below neck, draw arms */
    /* left arm */
    GFX_MoveTo(coords.x+20,coords.y-80);
    GFX_DrawLineTo(coords.x, coords.y-50, 2);
    /* right arm */
    GFX_MoveTo(coords.x+20,coords.y-80);
    GFX_DrawLineTo(coords.x+60, coords.y-70, 2);
    /* draw bow */
    /* string */
    GFX_MoveTo(coords.x+40,coords.y-50);
    GFX_DrawLineTo(coords.x+40, coords.y-90, 2);
    /* bow */
    GFX_DrawArc(coords.x+40,coords.y-70,20,270,180,2);

    /* return cursor to (coords.x,coords.y) */
    GFX_MoveTo(coords.x,coords.y);

    GFX_UpdateDisplay();
    return 0;
}

/* draw a stick man from the bottom left upwards 111 pixels tall,40 pixels wide */
int DrawStickMan(Coordinates_t coords)
{
    /* choose pen colour */
	GFX_SetColour(BLACK);

    /* move to specified start coords */
    GFX_MoveTo(coords.x,YFLOOR);

    /* draw left foot to hips, leaving cursor at hips */
    GFX_DrawLineTo(coords.x+20, coords.y-40, 2);
    /* draw hips to right foot, leaving cursor at foot */
    GFX_DrawLineTo(coords.x+40, coords.y, 2);
    /* move to hips */
    GFX_MoveTo(coords.x+20, coords.y-40);
    /* draw hips to neck, leaving cursor at neck */
    GFX_DrawLineTo(coords.x+20,coords.y-90,2);
    /* draw head, leaving cursor in the centre */
    GFX_DrawCircle(coords.x+20,coords.y-110,20,2);
    /* draw banker score */
    GFX_DrawText(coords.x+6,coords.y-120+3,"-60");

    /* choose pen colour */
	GFX_SetColour(RED);
    /* draw target box */
    GFX_DrawRectangle(coords.x+10,YFLOOR-150,coords.x+30,coords.y-130,2);
    /* draw apple score */
    GFX_DrawText(coords.x+10+2,YFLOOR-150+2,"50");
    /* choose pen colour */
	GFX_SetColour(BLACK);

    GFX_MoveTo(coords.x+20,coords.y-80);
    /* move below neck, draw arms */
    /* left arm */
    GFX_MoveTo(coords.x+20,coords.y-80);
    GFX_DrawLineTo(coords.x, coords.y-50, 2);
    /* right arm */
    GFX_MoveTo(coords.x+20,coords.y-80);
    GFX_DrawLineTo(coords.x+40, coords.y-50, 2);

    /* return cursor to (coords.x,coords.y) */
    GFX_MoveTo(coords.x,coords.y);

    GFX_UpdateDisplay();
    return 0;
}

/* draw a target on the ground in a specified range */
int DrawTarget(Coordinates_t coords,int xTarget)
{
    int whiteRadius = BLUERADIUS/3;
    int redRadius = BLUERADIUS*2/3;

    /* draw blue ring */
    GFX_DrawFilledRectangle(xTarget - BLUERADIUS,YFLOOR,xTarget + BLUERADIUS,YFLOOR + 30,BLUE);
    GFX_SetColour(BLUE);
    GFX_DrawText(xTarget+redRadius,YFLOOR+40+5,"20");
    /* draw red ring */
    GFX_DrawFilledRectangle(xTarget - redRadius,YFLOOR,xTarget + redRadius,YFLOOR + 20,RED);
    GFX_SetColour(RED);
    GFX_DrawText(xTarget+whiteRadius,YFLOOR+40,"30");
    /* draw white center */
    GFX_DrawFilledRectangle(xTarget - whiteRadius,YFLOOR,xTarget + whiteRadius,YFLOOR + 10,WHITE);
    GFX_SetColour(WHITE);
    GFX_DrawText(xTarget,YFLOOR+40-5,"40");

    /* draw an obstacle stickman */
    coords.x = xTarget + BLUERADIUS + 30;
    coords.y = YFLOOR;
    DrawStickMan(coords);

    return 0;
}

/* turn distances into scores */
 int GetScore(int distances[],int tries)
 {
    /* get a score value to put in array */
    /* set default score which changes if user hits a target */
    int score = -20;

    /* use final distance to determine what was hit */
    /* apple distance from xTarget */
    if (distances[tries] == BLUERADIUS + 40)
    {
        score = 50;
    }
    /* banker distance from xTarget */
    else if (distances[tries] == BLUERADIUS + 30)
    {
        score = -60;
    }
    else
    {
        /* make distance absolute for ease of processing for rings */
        distances[tries] = abs(distances[tries]);
        /* target rings */
        /* inner ring hit */
        if (distances[tries] <= BLUERADIUS/3)
        {
            score = 40;
        }
        /* middle ring hit */
        else if (distances[tries] <= BLUERADIUS*2/3)
        {
            score = 30;
        }
        /* outer ring hit */
        else if (distances[tries] <= BLUERADIUS)
        {
            score = 20;
        }
    }

    return score;
 }

/* get a three character user name */
 int GetUserName(PlayerData_t * player_p)
 {
    char chars[1];
    char name[4];
    /* variables to hold name chars */
    char char1,char2,char3;

    int i,charOffset = 0;
    int boolean;
    int key;

    GFX_DrawText(XWINDOW/2-50,YWINDOW/2-70,"Hi SCORE!");
    GFX_DrawText(XWINDOW/2-80,YWINDOW/2-50,"ENTER NAME BELOW");
    GFX_UpdateDisplay();

    for (i=0;i<3;i++)
    {
        boolean = 0;
        do
        {
            GFX_WaitForEvent();
            if (GFX_IsEventKeyDown())
            {
                GFX_GetKeyPress(&key);
                /* change key to character */
                /* save each char to a variabe */
                if (i == 0)
                {
                    char1 = key+96;
                    /* format char to string */
                    snprintf(chars,2,"%c",char1);
                }
                else if (i == 1)
                {
                    char2 = key+96;
                    /* format char to string */
                    snprintf(chars,2,"%c",char2);
                }
                else if (i == 2)
                {
                    char3 = key+96;
                    /* format char to string */
                    snprintf(chars,2,"%c",char3);
                }
                /* print char just typed */
                GFX_DrawText(XWINDOW/2-50+charOffset,YWINDOW/2,chars);
                boolean = 1;
                charOffset+=35;
                GFX_UpdateDisplay();
            }
        } while (!boolean);
    }

    /* format chars to string */
    snprintf(name,4,"%c%c%c",char1,char2,char3);
    /* coPCoords.y name to the player data struct */
    strcpy((*player_p).name,name);

    return 0;
 }

/* saves user's score to a file */
 int SaveUserScore(int scores[],PlayerData_t * player_p)
 {
    int i,j;

    FILE *fp;
    PlayerData_t filePlayers[HISCORES];
    int tempscore;
    char tempname[4];

    /* open file for reading */
    fp = fopen("hiscores.txt","r");
    /* give error message and quit if opening file failed */
    if (fp == NULL)
    {
        printf("I'm sorry, Dave. I'm afraid I can't read the file.");
        exit(0);
    }
    /* loop hi scores to read names and scores to arrays */
    /* don't read in lowest score, replace it with new hi score */
    for (i = 0; i < HISCORES - 1; i++)
    {
        fscanf(fp, "%3s,%d", filePlayers[i].name,&filePlayers[i].score_total);
        /* printf("\nname: %3s, score: %d\n",strNames[i],filescore[i]); */
    }
    fclose(fp);

    /* place new hi score and name into array */
    filePlayers[HISCORES-1].score_total = (*player_p).score_total;
    strcpy(filePlayers[HISCORES-1].name,(*player_p).name);

    /* re sort array into descending order */
    for (i = 0; i < HISCORES - 1; i++)
    {
        for (j = 0; j < HISCORES - 1 - i; j++)
        {
            if (filePlayers[j].score_total < filePlayers[j+1].score_total)
            {
                /* switch score positions */
                tempscore = filePlayers[j+1].score_total;
                filePlayers[j+1].score_total = filePlayers[j].score_total;
                filePlayers[j].score_total = tempscore;
                /* switch name positions */
                strcpy(tempname,filePlayers[j+1].name);
                strcpy(filePlayers[j+1].name,filePlayers[j].name);
                strcpy(filePlayers[j].name,tempname);
            }
        }
    }

    /* open file for reading */
    fp = fopen("hiscores.txt","w");
    /* give error message and quit if opening file failed */
    if (fp == NULL)
    {
        printf("I'm sorry, Dave. I'm afraid I can't write to the file.");
        exit(0);
    }
    /* write the adjusted scores array back to the file */
    for (i = 0; i < HISCORES; i++)
    {
        fprintf(fp, "%3s,%d\n", filePlayers[i].name,filePlayers[i].score_total);
    }

    fclose(fp);
    return 0;
 }

/* when using events, wait for click in a box */
 int WaitForClickBox()
 {
    int button,boolean;
    Coordinates_t Coords;

    /* draw a 'continue?' box */
    GFX_DrawFilledRectangle(XWINDOW-150,YFLOOR+50,XWINDOW-50,YWINDOW-20,WHITE);
    GFX_SetColour(BLACK);
    GFX_DrawText(XWINDOW-120,YFLOOR+60,"NEXT");

    GFX_UpdateDisplay();

    /* wait for mouse click and get coords */
    boolean = 0;
    do{
        GFX_WaitForEvent();
        /* check event is a mouse click */
        if (GFX_IsEventMouseButton())
        {
            GFX_GetMouseButton(&button);
            /* check event is left click */
            if (button == MOUSE_BUTTON_LEFT)
            {
                /* left click is true */
                GFX_GetMouseCoordinates(&Coords.x,&Coords.y);
                /* only continue if click was in the box */
                if (XWINDOW-150 < Coords.x && Coords.x < XWINDOW-50 && YFLOOR+50 < Coords.y && Coords.y < YWINDOW-20)
                {
                    boolean = 1;
                }
            }
        }
    } while (!boolean);

    return 0;
 }

/* the main program */
int main(void)
{
    /* ---------------------- DEFINE VARIABLES ---------------------- */

    /* define structure for player name and total score */
    PlayerData_t player;
    /* struct for x and y coordinates */
    Coordinates_t coords;
    /* target position */
    int xTarget;
    /* holds all distances */
    int arrDistances[SHOTS];
    /* holds all scores */
    int arrScores[SHOTS];
    /* boolean for loops and checks */
    int boolean;
    /* holds current throw number */
    int tries;
    /* pointers and graphical feedback stuff */
    int keypress,button;
    /* wind acceleration */
    int windAcc;

    /* -------------------- INITIALISE VARIABLES -------------------- */

    /* make numbers even more random */
    srand(time(NULL));
    /* set initial coords to draw stick archer */
    coords.x = GFX_RandNumber(20,120);
	coords.y = YFLOOR;
	/* set target center inside the given range */
	/* lower range is arbitrary, upper range must fit target and stickman onscreen */
	xTarget = GFX_RandNumber(XWINDOW - 275, XWINDOW - (BLUERADIUS+72));

    /* ------------------------ MAIN PROGRAM ------------------------ */

    /* open a graphics window of size (xWindow, yWindow) */
	GFX_InitWindow(XWINDOW, YWINDOW);
	/* create event cue and initialise handlers */
	GFX_CreateEventQueue();
	GFX_InitFont();
	GFX_InitMouse();
	GFX_RegisterMouseEvents();
	GFX_InitKeyboard();
	GFX_RegisterKeyboardEvents();
	GFX_RegisterDisplayEvents();

	/* draw the start screen, wait for user input */
    DrawStartupScreen(xTarget);
    /* trigger the main screen */
    WaitForClickBox();

    /* menu loop - loops menu until all projectile are thrown */
    tries = 0;
    do
    {
        /* set wind speed for this throw */
        windAcc = GFX_RandNumber(0,8);
        /* draw the game background */
        DrawGameBackground(tries,windAcc);
        DrawTarget(coords,xTarget);
        /* draw a stickman with its left foot at (x,y) and returns cursor to (x,y) */
        DrawStickArcher(coords);

        /* loop until user chooses to throw projectile */
        boolean = 0;
        do
        {
            /* get user input */
            GFX_WaitForEvent();

            if (GFX_IsEventKeyDown())
            {
                GFX_GetKeyPress(&keypress);
                /* and operator makes sure that the stickman can't move offscreen */
                if (keypress == ALLEGRO_KEY_A && coords.x > 10)
                {
                    /* move stickman along the yFloor */
                    coords.x -= 10;
                }
                /* and operator makes sure that the stickman can't move too close */
                else if (keypress == ALLEGRO_KEY_D && coords.x < XWINDOW - 400)
                {
                    /* move stickman along the yFloor */
                    coords.x += 10;
                }
                /* redraw background and stickman */
                DrawGameBackground(tries,windAcc);
                DrawTarget(coords,xTarget);
                DrawStickArcher(coords);
            }
            else if (GFX_IsEventMouseButton())
            {
                GFX_GetMouseButton(&button);
                if (button == MOUSE_BUTTON_LEFT)
                {
                    /* draw path, add distance to array */
                    arrDistances[tries] = DrawProjectilePath(coords,xTarget,windAcc);

                    /* change distance to score, add to array */
                    arrScores[tries] = GetScore(arrDistances,tries);
                    WaitForClickBox();
                    /* draw score graph scene */
                    DrawScoreGraph(coords,arrScores,tries);
                    WaitForClickBox();

                    /* redraw game background and character*/
                    DrawGameBackground(tries,windAcc);
                    DrawTarget(coords,xTarget);
                    DrawStickArcher(coords);

                    /* break the loop */
                    boolean = 1;
                }
            }
            GFX_UpdateDisplay();
        /* loop until projectile is thrown */
        } while(!boolean);
        /* user has taken a shot so increment tries */
        tries++;
    /* break when the user has taken the max number of shots */
    } while (tries < SHOTS);

    /* print out the stats */
    player.score_total = DrawStatistics(arrDistances,arrScores);
    /* check if score is new hi score */
    boolean = CheckHiScore(arrScores,player.score_total);

    /* if a new hi score, save it */
    if (boolean)
    {
        GetUserName(&player);
        /* gets the users name  */
        SaveUserScore(arrScores,&player);
    }
    else
    {
        GFX_DrawText(XWINDOW/2-150,YWINDOW/2-20,"Not a hi score this time I'm afraid");
        GFX_DrawText(XWINDOW/2-80,YWINDOW/2,"Try again soon!");
    }

    /* move the contents of the screen buffer to the display */
    GFX_UpdateDisplay();
	/* Wait for a user's signal to exit*/
	WaitForClickBox();
    /* draw hi score table */
	DrawHiScores(arrScores);
	WaitForClickBox();
    /* remove the display */
    GFX_CloseWindow();
    return 0;
}
