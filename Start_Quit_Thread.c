#include"Bingo_Header.h"
/*
 * This thread takes conf_p* as argument.
 * it waits for server side keyboard input
 * while accept thread is running.
 * if the nomber of players is >1
 * then enter will be accepted and game will start
 * if q is pressed the accept thread ends and the program goes to main menu.
 */
void* startquit(void* arg)
{
    conf_p *conf=arg;
    while(conf->status==0)
    {
	switch(getch())
	{
	    case '\n':
		if(conf->pl->n==1)
		    continue;
		conf->status=1;
		break;
	    case 'q':
		conf->status=-2;
	}
    }
    /********************** When quiting or starting game ***********************/
    pthread_mutex_lock(&conf->pl->lock);  // Locks the player list
    pthread_cancel(conf->acct);		  // to make sure that, no player detail is added in part.
    pthread_mutex_unlock(&conf->pl->lock);// Unlocks after cancelling accept thread.
    pthread_exit(NULL);
}
