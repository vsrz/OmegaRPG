extern "C" {
#include "glob.h"
}

#include "OmegaRPG.h"

using namespace Omega;

void signalquit(int ignore)
{
    quit();
}

int main(int argc, char *argv[])
{
    OmegaRPG* game;

	bool showTitle = true;
	bool showScores = true;

#ifndef NOGETOPT
    int i;
	bool scoresOnly = false;

    while(( i= getopt( argc, argv, "dshq")) != -1)
    {
        switch (i)
        {
#ifdef DEBUG
        case 'd':
            DG_debug_flag++;
            break;
#endif
        case 's':
            scoresOnly = true;
            showTitle = false;
            break;
        case 'h':
            OmegaRPG::printUsage();
            exit(0);
            break;
        case 'q':
        	showTitle = false;
        	showScores = false;
        	break;
        case '?':
            /* error parsing args... ignore? */
            printf("'%c' is an invalid option, ignoring\n", optopt );
            break;
        }
    }

    if (optind >= argc ) {
        /* no save file given */
        game = new OmegaRPG();
    } else {
        /* savefile given */
        game = new OmegaRPG(argv[optind]);
        game->Continuing(true);
    }

    game->ScoresOnly(scoresOnly);

#else
    // Alternate code for people who don't support getopt() - no enhancement
    if (argc == 2) {
        game = new OmegaRPG(argv[1]);
    } else {
        game = new OmegaRPG();
    }
#endif

// Always catch ^c and hang-up signals
#ifdef SIGINT
    signal(SIGINT,signalquit);
#endif

#ifdef SIGHUP
    signal(SIGHUP,signalsave);
#endif

#ifdef SIGQUIT
        signal(SIGQUIT,signalexit);
#endif
#ifdef SIGILL
        signal(SIGILL,signalexit);
#endif

#ifdef SIGIOT
        signal(SIGIOT,signalexit);
#endif
#ifdef SIGABRT
        signal(SIGABRT,signalexit);
#endif
#ifdef SIGEMT
        signal(SIGEMT,signalexit);
#endif
#ifdef SIGBUS
        signal(SIGBUS,signalexit);
#endif
#ifdef SIGSYS
        signal(SIGSYS,signalexit);
#endif

#ifdef DEBUG
        if( DG_debug_flag ) {
#ifdef SIGTRAP
            signal(SIGTRAP,signalexit);
#endif
#ifdef SIGFPE
            signal(SIGFPE,signalexit);
#endif
#ifdef SIGSEGV
            signal(SIGSEGV,signalexit);
#endif
        }
#endif

    Omegalib = OMEGALIB;

    // If filecheck is 0, some necessary data files are missing
    if (filecheck() == 0) exit(-1);

    // All kinds of initialization
    init_perms();
    initgraf();
    initrand(E_RANDOM, 0);
    initspells();

#ifdef DEBUG
    /* initialize debug log file */
    DG_debug_log = fopen( "omegadbg.log", "a" );
    assert( DG_debug_log ); /* WDT :) */
    setvbuf( DG_debug_log, NULL, _IOLBF, 0);
    fprintf(DG_debug_log, "##############  new game started ##############\n");
#endif

    for (int count = 0; count < STRING_BUFFER_SIZE; count++)
    {
        strcpy(Stringbuffer[count], "<nothing>");
    }

#ifdef SAVE_LEVELS
    msdos_init();
#endif

    if (showTitle) {
    	omega_title();
    }

    if (showScores || game->ScoresOnly()) {
    	showscores();
    }

    if (game->ScoresOnly()) {
        endgraf();
        exit(0);
    }

    /* game restore attempts to restore game if there is an argument */
    if (game->Continuing())
    {
        game_restore(argv[optind]);
        game->SaveFilename(argv[optind]);
        mprint("Your adventure continues....");
    }
    else
    {
        /* monsters initialized in game_restore if game is being restored */
        /* items initialized in game_restore if game is being restored */
        inititem(TRUE);
        Date = random_range(360);
        Phase = random_range(24);

        // Initialize bank
        bank_init();

        game->Continuing(initplayer()); /* RM: 04-19-2000 loading patch */

        game->SaveFilename(Player.name);
    }

    if (!game->Continuing())
    {
        init_world();  /* RM: 04-19-2000 loading patch */
        mprint("'?' for help or commandlist, 'Q' to quit.");
    }

    timeprint();
    calc_melee();

    if (Current_Environment != E_COUNTRYSIDE)
    {
        showroom(Level->site[Player.x][Player.y].roomnumber);
    }
    else
    {
        terrain_check(FALSE);
    }

    if (optionp(SHOW_COLOUR))
    {
        colour_on();
    }
    else
    {
        colour_off();
    }

    screencheck(Player.x,Player.y);

    // Game cycle
    if (!game->Continuing())
    {
        time_clock(TRUE);
    }

    while (TRUE) {
        if (Current_Environment == E_COUNTRYSIDE)
        {
            p_country_process();
        }
        else
        {
            time_clock(FALSE);
        }
    }
}
