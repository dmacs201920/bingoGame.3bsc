#include"Bingo_Header.h"
extern int end_game_flag;

/*
		FUNCTION TO HELP CLIENT CONNECT TO THE SERVER
		USED IN THE CLIENT SIDE OF THE PROGRAM
*/   

void connect_to_server(char **err)
{
    /********************************************************  INITIALIZATIONS  ***************************************************************/
    *err=NULL;
    end_game_flag=0;
    int adl = sizeof(struct sockaddr_in),port_no;


    struct sockaddr_in ad;
    //int port;
    game_p par;
   
    curs_set(1);
    cbreak();
   
    bzero(&ad,sizeof(ad));
    ad.sin_family=AF_INET;
    ad.sin_addr.s_addr=inet_addr(MY_ADDR);

    /*****************************************************************************************************************************************/


    WINDOW *ser_det = newwin(20,50,6,50);			//CREATING WINDOW
    if(ser_det==NULL)
    {
	*err = "ser_det window error";
	delwin(ser_det);
	clear();
	refresh();
	return;

    }
wattron(ser_det,COLOR_PAIR(1));
    box(ser_det,0,0);
wattroff(ser_det,COLOR_PAIR(1));


    if((par.sersd = socket(AF_INET,SOCK_STREAM,0))==-1)		//INITIALISING THE SOCKET DESCRIPTOR FOR THE SERVER SIDE
    {
	*err = "Socket create error";
	delwin(ser_det);
	clear();
	refresh();
	return;
    }

    echo();							//PRINTS THE USER KEYBOARD INPUT ON SCREEN

    while(1)
    {
	/******************************************  GETTING USER INPUT FOR PORT NUMBER  *********************************************************/
	mvwprintw(ser_det,2,2,"USE ONLY BACKSPACE AND DIGITS!");
	mvwprintw(ser_det,4,2,"ENTER PORT NUMBER(PRESS -1 TO EXIT):");

	refresh();
	wrefresh(ser_det);
	mvwprintw(ser_det,6,5,"         ");
	mvwscanw(ser_det,6,5,"%d",&port_no);

	if(port_no == -1)
	{
	    noecho();
	    curs_set(0);
	    delwin(ser_det);
	    clear();
	    refresh();
	    return;
	}
    /*****************************************************************************************************************************************/
	ad.sin_port=htons(port_no);   				//SETTING THE PORT NUMBER


	if((connect(par.sersd,ADCAST &ad,adl))!=0)		//TRYING TO CONNECT TO THE SERVER
	{
	    wattron(ser_det,COLOR_PAIR(4));
	    mvwprintw(ser_det,15,2,"UNABLE TO CONNECT TO SERVER!!!");
	    mvwprintw(ser_det,16,2,"PLEASE TRY AGAIN");
	    wattroff(ser_det,COLOR_PAIR(4));
	    continue;
	}

	break;
    }		//while close

    noecho();								//DISABLES ECHO

    delwin(ser_det);							//DELETING THE WINDOW
    clear();
    refresh();

    curs_set(0);							//CURSER HIDING FROM SCREEN

    int startx = 7,starty = 60,row,col;

/***********************************************  RECIEVING THE BINGO GAME PLAY ARRAY FROM THE SERVER  ******************************************/
    for(int i=0;i<5;++i)
    {
	for(int j=0;j<5;++j)
	{
	    if(timed_recv(par.sersd,&par.get.array[i][j],sizeof(int),0,2)!=sizeof(int))
	    {
		close(par.sersd);
		*err = "Array recieve error";
		return;
	    }

	}

    }

    /*****************************************************************************************************************************************/

    par.get.x = par.get.y = par.get.p =  par.get.q = 0;
    pthread_mutex_init(&par.get.get_m,NULL);			//MUTEX VARIABLE INITIALIZATIONS
    pthread_mutex_init(&par.get.done_mutex,NULL);
    pthread_cond_init(&par.get.done,NULL);			//CONDITION VARIABLE INITIALIZATION

    //BINGO IS FOR DISPLAYING THE ARRAY NUMBERS
    //PLAYCHANCE FOR DISPLAYING WHO'S CHANCE IT IS
    //BINGOCNT IS FOR DISPLAYING THE THE NUMBER OF ROWS,COLS AND DIAGONALS COMPLETED



    par.playchance = newwin(3,120,3,starty);		//CREATES THE WINDOW AND RETURNS A POINTER TO THE PLAYCHANCE
    if(par.playchance==NULL)				
    {
	*err="Unable to create WINDOW";				//ERROR MSG
	return;
    }

    par.chancepan = new_panel(par.playchance);			//CREATES THE PANEL AND RETURN THE POINTER TO CHANCEPAN
    if(par.chancepan==NULL)				
    {
	delwin(par.playchance);
	*err="Unable to create PANEL";				//ERROR MSG
	return;
    }

    par.bingocnt = newwin(60,3,startx+2,starty-5);
    if(par.bingocnt==NULL)				
    {
	del_panel(par.chancepan);
	delwin(par.playchance);

	*err="Unable to create WINDOW";				//ERROR MSG
	return;
    }

    par.bingcnt = new_panel(par.bingocnt);
    if(par.bingcnt==NULL)				
    {
	del_panel(par.chancepan);
	delwin(par.playchance);

	delwin(par.bingocnt);
	*err="Unable to create PANEL";				//ERROR MSG
	return;
    }

    ////////////////////////////////////////////////////   CREATING BINGO WINDOWS AND PANELS    //////////////////////////////////////////////////

    int i,j,t1,t2;

    for(i=0;i<5;i++)
    {
	for(j=0;j<5;j++)
	{
	    par.get.bingo[i][j] = newwin(width,length,startx,starty);
	    if(par.get.bingo[i][j]==NULL)
	    {
		del_panel(par.chancepan);
		delwin(par.playchance);

		del_panel(par.bingcnt);
		delwin(par.bingocnt);

		for(t1=0;t1<=i;++t1)
		    for(t2=0;t2<5;++t2)
		    {
			if(t1==i&&t2==j)
			    break;
			delwin(par.get.bingo[t1][t2]);
		    }
		*err="Unable to create WINDOW";				//ERROR MSG
		return;
	    }

	    par.pan[i][j] = new_panel(par.get.bingo[i][j]);
	    if(par.pan[i][j]==NULL)
	    {
		del_panel(par.chancepan);
		delwin(par.playchance);
		del_panel(par.bingcnt);
		delwin(par.bingocnt);
		for(i=0;i<5;i++)
		    for(j=0;j<5;j++)
			delwin(par.get.bingo[i][j]);
		for(t1=0;t1<=i;++t1)
		    for(t2=0;t2<5;++t2)
		    {
			if(t1==i&&t2==j)
			    break;
			del_panel(par.pan[t1][t2]);
		    }

		*err="Unable to create PANEL";				//ERROR MSG
		return;
	    }

	    wattron(par.get.bingo[i][j],COLOR_PAIR(1)|A_REVERSE);		//ENABLES ATTRIBUTES IN THE ARGUMENTS


	    wborder(par.get.bingo[i][j],'|','|','-','-','+','+','+','+');	//FOR BORDERING THE WINDOW

	    wattroff(par.get.bingo[i][j],COLOR_PAIR(1)|A_REVERSE);		//DISABLES ATTRIBUTES IN THE ARGUMENTS

	    mvwprintw(par.get.bingo[i][j],2,3,"%d",par.get.array[i][j]);		//MOVES TO THE REQ POSITION AND PRINTS IN THE WINDOW

	    starty+=right;

	}
	startx+=width-1;
	starty = sy; 
    }




    wattron(par.get.bingo[0][0],A_STANDOUT);
    mvwprintw(par.get.bingo[0][0],2,3,"%d",par.get.array[0][0]);
    wattroff(par.get.bingo[0][0],A_STANDOUT);
    update_panels();	
    doupdate();				//REFRESHES ALL THE PANELS IN ORDER REQUIRED


    /*****************************************************************************************************************************************/

    if(pthread_create(&par.getid,NULL,get_key_t,&par.get)<0)			//STARTING THE GET KEY THREAD FUNCTION
    {
	close(par.sersd);

	for(t1=0;t1<5;++t1)
	    for(t2=0;t2<5;++t2)
	    {
		delwin(par.get.bingo[t1][t2]);
		del_panel(par.pan[t1][t2]);
	    }
	del_panel(par.chancepan);
	delwin(par.playchance);
	del_panel(par.bingcnt);
	delwin(par.bingocnt);


	*err="Unable to create Get Key thread";
	return;
    }

    if(pthread_create(&par.get.gameid,NULL,client_game_t,&par)<0)		//STARTING THE CLIENT GAME THREAD FUNCTION
    {
	close(par.sersd);
	pthread_cancel(par.getid);

	for(t1=0;t1<5;++t1)
	    for(t2=0;t2<5;++t2)
	    {
		del_panel(par.pan[t1][t2]);
		delwin(par.get.bingo[t1][t2]);
	    }
	del_panel(par.chancepan);
	delwin(par.playchance);
	del_panel(par.bingcnt);
	delwin(par.bingocnt);


	*err="Unable to create Game thread";
	return;
    }


    while(end_game_flag==0)							//WAITS FOR THE END GAME FLAG TO CHANGE
	sleep(0.2);

    if(end_game_flag>0)
	pthread_join(par.get.gameid,(void**)err);				//RECIEVING ERROR MESSAGE FROM CLIENT GAME THREAD FUNCTION

    /***************************************************  CLOSING CONNECTIONS AND DELETING WINDOWS  **********************************************/

    		close(par.sersd);
		for(i=0;i<5;i++)
		{
		    for(j=0;j<5;j++)
		    {
			del_panel(par.pan[i][j]);
			delwin(par.get.bingo[i][j]);
		    }
		}
		del_panel(par.chancepan);
		delwin(par.playchance);
		del_panel(par.bingcnt);
		delwin(par.bingocnt);

    /*****************************************************************************************************************************************/

		return;


	

    
}
