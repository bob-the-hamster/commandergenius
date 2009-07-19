/* GAMEDO.C
  Contains all of the gamedo_xxx functions...which are called from the
  main game loop. These functions perform some task that is done each
  time around the game loop, not directly related to the player.
*/

#include "keen.h"

#include "include/game.h"
#include "include/gamedo.h"
#include "include/gamepdo.h"
#include "include/misc.h"
#include "sdl/CVideoDriver.h"
#include "sdl/CTimer.h"
#include "sdl/CInput.h"
#include "sdl/sound/CSound.h"
#include "CGraphics.h"
#include "vorticon/CPlayer.h"
#include "keenext.h"

#include "include/enemyai.h"

extern unsigned long gotPlayX;

extern unsigned long CurrentTickCount;

extern unsigned int unknownKey;

extern CPlayer *Player;

int animtiletimer, curanimtileframe;

// gathers data from input controllers: keyboard, joystick, network,
// whatever to populate each player's keytable
unsigned char oldleftkey = 5;
unsigned char oldrightkey = 5;
unsigned char oldupkey = 5;
unsigned char olddownkey = 5;
unsigned char oldctrlkey = 5;
unsigned char oldaltkey = 5;
void gamedo_getInput(stCloneKeenPlus *pCKP)
{
int i;
int byt;
unsigned int msb, lsb;

     if (demomode==DEMO_PLAYBACK)
     {
        // time to get a new key block?
        if (!demo_RLERunLen)
        {
          /* get next RLE run length */
          lsb = demo_data[demo_data_index++];
          msb = demo_data[demo_data_index++];
          demo_RLERunLen = (msb<<8) | lsb;
          byt = demo_data[demo_data_index++];         // get keys down

          player[0].playcontrol[PA_X] = 0;
          player[0].playcontrol[PA_POGO] = 0;
          player[0].playcontrol[PA_JUMP] = 0;
          player[0].playcontrol[PA_FIRE] = 0;
          player[0].playcontrol[PA_STATUS] = 0;

          if (byt & 1) player[0].playcontrol[PA_X] -= 100;
          if (byt & 2) player[0].playcontrol[PA_X] += 100;
          if (byt & 4) player[0].playcontrol[PA_POGO] = 1;
          if (byt & 8) player[0].playcontrol[PA_JUMP] = 1;
          if (byt & 16)player[0].playcontrol[PA_FIRE] = 1;
          if (byt & 32)player[0].playcontrol[PA_STATUS] = 1;
          if (byt & 64)
          {  // demo STOP command
            if (fade.mode!=FADE_GO) endlevel(1, pCKP);
          }
        }
        else
        {
          // we're still in the last RLE run, don't change any keys
          demo_RLERunLen--;
        }

        // user trying to cancel the demo?
        for(i=0;i<KEYTABLE_SIZE;i++)
        {
          if (g_pInput->getPressedKey(i))
          {
            if (fade.mode!=FADE_GO) endlevel(0, pCKP);
          }
        }
        if (g_pInput->getPressedCommand(IC_STATUS))
        {
          if (fade.mode!=FADE_GO) endlevel(0, pCKP);
        }

        return;
     }

     for(Uint8 p=0 ; p<numplayers ; p++)
     {
    	 memcpy(player[p].lastplaycontrol,player[p].playcontrol,PA_MAX_ACTIONS);

    	 // Entry for every player
    	 for(Uint8 j=0 ; j<PA_MAX_ACTIONS ; j++)
    		 player[p].playcontrol[j] = 0;

    	 if(g_pInput->getHoldedCommand(p, IC_LEFT))
    		 player[p].playcontrol[PA_X] -= 100;
    	 if(g_pInput->getHoldedCommand(p, IC_RIGHT))
    		 player[p].playcontrol[PA_X] += 100;

    	 if(g_pInput->getHoldedCommand(p, IC_UP))
    		 player[p].playcontrol[PA_Y] -= 100;
    	 if(g_pInput->getHoldedCommand(p, IC_DOWN))
    		 player[p].playcontrol[PA_Y] += 100;

    	 if(g_pInput->getHoldedCommand(p, IC_JUMP))
    		 player[p].playcontrol[PA_JUMP] = 1;
    	 if(g_pInput->getHoldedCommand(p, IC_POGO))
    		 player[p].playcontrol[PA_POGO] = 1;
    	 if(g_pInput->getHoldedCommand(p, IC_FIRE))
    		 player[p].playcontrol[PA_FIRE] = 1;
    	 if(g_pInput->getHoldedCommand(p, IC_STATUS))
    		 player[p].playcontrol[PA_STATUS] = 1;

    	 if (demomode==DEMO_RECORD)
    	 {
    	   if(i) player[p].playcontrol[PA_X] += 100;
    	   fputc(i, demofile);
    	   if(i) player[p].playcontrol[PA_X] -= 100;
    	   fputc(i, demofile);
    	   if(i) player[p].playcontrol[PA_POGO] = 1;
    	   fputc(i, demofile);
    	   if(i) player[p].playcontrol[PA_JUMP] = 1;
    	   fputc(i, demofile);
    	   if(i) player[p].playcontrol[PA_FIRE] = 1;
    	   fputc(i, demofile);
    	   if(i) player[p].playcontrol[PA_STATUS] = 1;
    	   fputc(i, demofile);
    	 }
     }
}

// handles scrolling, for player cp
// returns nonzero if the scroll was changed
int gamedo_ScrollTriggers(int theplayer)
{
signed int px, py;
int scrollchanged;

   if (player[theplayer].pdie) return 0;

   //px = (Player[theplayer].getCoordX()>>CSF)-scroll_x;
   //py = (Player[theplayer].getCoordY()>>CSF)-scroll_y;

   px = (player[theplayer].x>>CSF)-scroll_x;
   py = (player[theplayer].y>>CSF)-scroll_y;

   scrollchanged = 0;

   /* left-right scrolling */
   if(px > SCROLLTRIGGERRIGHT && scroll_x < max_scroll_x)
   {
      map_scroll_right();
      scrollchanged = 1;
   }
   else if(px < SCROLLTRIGGERLEFT && scroll_x > 32)
   {
      map_scroll_left();
      scrollchanged = 1;
   }

   /* up-down scrolling */
   if (py > SCROLLTRIGGERDOWN && scroll_y < max_scroll_y)
   {
      map_scroll_down();
      scrollchanged = 1;
   }
   else if (py < SCROLLTRIGGERUP && scroll_y > 32)
   {
      map_scroll_up();
      scrollchanged = 1;
   }

   return scrollchanged;
}

// animates animated tiles
void gamedo_AnimatedTiles(void)
{
int i;
   /* animate animated tiles */
   if (animtiletimer>ANIM_TILE_TIME)
   {
      /* advance to next frame */
      curanimtileframe = (curanimtileframe+1)&7;
      /* re-draw all animated tiles */
      for(i=1;i<MAX_ANIMTILES-1;i++)
      {
         if (animtiles[i].slotinuse)
         {
        	 g_pGraphics->drawTile(animtiles[i].x, animtiles[i].y, animtiles[i].baseframe+((animtiles[i].offset+curanimtileframe)%TileProperty[animtiles[i].baseframe][ANIMATION]));
         }
      }
      animtiletimer = 0;
   }
   else animtiletimer++;
}

// do object and enemy AI
void gamedo_enemyai(stCloneKeenPlus *pCKP)
{
int i;
// handle objects and do enemy AI
   for(i=1;i<MAX_OBJECTS-1;i++)
   {
      if (!objects[i].exists || objects[i].type==OBJ_PLAYER) continue;

      // check if object is really in the map!!!
      if (objects[i].x < 0 || objects[i].y < 0)
    	  continue;

      if (objects[i].x > (map.xsize << CSF << 4) || objects[i].y > (map.ysize << CSF << 4))
    	  continue;

      objects[i].scrx = (objects[i].x>>CSF)-scroll_x;
      objects[i].scry = (objects[i].y>>CSF)-scroll_y;
      if (objects[i].scrx < -(sprites[objects[i].sprite].xsize) || objects[i].scrx > 320 \
          || objects[i].scry < -(sprites[objects[i].sprite].ysize) || objects[i].scry > 200)
          {
             objects[i].onscreen = 0;
             objects[i].wasoffscreen = 1;
             if (objects[i].type==OBJ_ICEBIT) objects[i].exists = 0;
          }
          else
          {
             #ifdef TARGET_WIN32
//               if (numplayers>1)
//                 if (!objects[i].hasbeenonscreen)
//                   net_sendobjectonscreen(i);
             #endif

             objects[i].onscreen = 1;
             objects[i].hasbeenonscreen = 1;
          }

      if (objects[i].hasbeenonscreen || objects[i].type==OBJ_RAY || \
          objects[i].type==OBJ_ICECHUNK || objects[i].type==OBJ_PLATFORM ||\
          objects[i].type==OBJ_PLATVERT)
      {
         common_enemy_ai(i);
         switch(objects[i].type)
         {
          //KEEN1
          case OBJ_YORP: yorp_ai(i, pCKP->Control.levelcontrol); break;
          case OBJ_GARG: garg_ai(i, pCKP); break;
          case OBJ_VORT: vort_ai(i, pCKP, pCKP->Control.levelcontrol); break;
          case OBJ_BUTLER: butler_ai(i, pCKP->Control.levelcontrol.hardmode); break;
          case OBJ_TANK: tank_ai(i, pCKP->Control.levelcontrol.hardmode); break;
          case OBJ_RAY: ray_ai(i, pCKP, pCKP->Control.levelcontrol); break;
          case OBJ_DOOR: door_ai(i); break;
          case OBJ_ICECHUNK: icechunk_ai(i); break;
          case OBJ_ICEBIT: icebit_ai(i); break;
          case OBJ_TELEPORTER: teleporter_ai(i, pCKP->Control.levelcontrol); break;
          case OBJ_ROPE: rope_ai(i); break;
          //KEEN2
          case OBJ_WALKER: walker_ai(i, pCKP->Control.levelcontrol); break;
          case OBJ_TANKEP2: tankep2_ai(i, pCKP); break;
          case OBJ_PLATFORM: platform_ai(i, pCKP->Control.levelcontrol); break;
          case OBJ_BEAR: bear_ai(i, pCKP->Control.levelcontrol, pCKP); break;
          case OBJ_SECTOREFFECTOR: se_ai(i, pCKP); break;
          case OBJ_BABY: baby_ai(i, pCKP->Control.levelcontrol); break;
          case OBJ_EXPLOSION: explosion_ai(i); break;
          case OBJ_EARTHCHUNK: earthchunk_ai(i); break;
          //KEEN3
          case OBJ_FOOB: foob_ai(i, pCKP); break;
          case OBJ_NINJA: ninja_ai(i, pCKP); break;
          case OBJ_MEEP: meep_ai(i, pCKP->Control.levelcontrol); break;
          case OBJ_SNDWAVE: sndwave_ai(i, pCKP); break;
          case OBJ_MOTHER: mother_ai(i, pCKP->Control.levelcontrol); break;
          case OBJ_FIREBALL: fireball_ai(i, pCKP); break;
          case OBJ_BALL: ballandjack_ai(i, pCKP); break;
          case OBJ_JACK: ballandjack_ai(i, pCKP); break;
          case OBJ_PLATVERT: platvert_ai(i); break;
          case OBJ_NESSIE: nessie_ai(i); break;

          case OBJ_DEMOMSG: break;
          default:
            crashflag = 1;
            crashflag2 = i;
            crashflag3 = objects[i].type;
            why_term_ptr = "Invalid object flag2 of type flag3";
            break;
         }

        objects[i].scrx = (objects[i].x>>CSF)-scroll_x;
        objects[i].scry = (objects[i].y>>CSF)-scroll_y;
      }
   }
}


int savew, saveh;

void gamedo_render_drawobjects(stCloneKeenPlus *pCKP)
{
unsigned int i;
int x,y,o,tl,xsize,ysize;
int xa,ya;

   // copy player data to their associated objects show they can get drawn
   // in the object-drawing loop with the rest of the objects
   for( i=0 ;i < numplayers ; i++)
   {
     o = player[i].useObject;

     if (!player[i].hideplayer)
     {
       objects[o].sprite = player[i].playframe + playerbaseframes[i];
     }

     else
     {
       objects[o].sprite = BlankSprite;
     }
     objects[o].x = player[i].x;
     objects[o].y = player[i].y;
     objects[o].scrx = (player[i].x>>CSF)-scroll_x;
     objects[o].scry = (player[i].y>>CSF)-scroll_y;

   }

   // if we're playing a demo keep the "DEMO" message on the screen
   // as an object
   if (demomode==DEMO_PLAYBACK)
   {
     #define DEMO_X_POS         137
     #define DEMO_Y_POS         6
     objects[DemoObjectHandle].exists = 1;
     objects[DemoObjectHandle].onscreen = 1;
     objects[DemoObjectHandle].type = OBJ_DEMOMSG;
     objects[DemoObjectHandle].sprite = DemoSprite;
     objects[DemoObjectHandle].x = (DEMO_X_POS+scroll_x)<<CSF;
     objects[DemoObjectHandle].y = (DEMO_Y_POS+scroll_y)<<CSF;
     objects[DemoObjectHandle].honorPriority = 0;
   }
   else objects[DemoObjectHandle].exists = 0;

   // draw all objects. drawn in reverse order because the player sprites
   // are in the first few indexes and we want them to come out on top.
   for(i=MAX_OBJECTS-1;;i--)
   {
      if (objects[i].exists && objects[i].onscreen)
      {
        objects[i].scrx = ((objects[i].x>>CSF)-scroll_x);
        objects[i].scry = ((objects[i].y>>CSF)-scroll_y);
        g_pGraphics->drawSprite(objects[i].scrx, objects[i].scry, objects[i].sprite, i);

        if (objects[i].honorPriority)
        {
            // handle priority tiles and tiles with masks
            // get the upper-left coordinates to start checking for tiles
            x = (((objects[i].x>>CSF)-1)>>4)<<4;
            y = (((objects[i].y>>CSF)-1)>>4)<<4;

            // get the xsize/ysize of this sprite--round up to the nearest 16
            xsize = ((sprites[objects[i].sprite].xsize)>>4<<4);
            if (xsize != sprites[objects[i].sprite].xsize) xsize+=16;

            ysize = ((sprites[objects[i].sprite].ysize)>>4<<4);
            if (ysize != sprites[objects[i].sprite].ysize) ysize+=16;

            // now redraw any priority/masked tiles that we covered up
            // with the sprite
            for(ya=0;ya<=ysize;ya+=16)
            {
              for(xa=0;xa<=xsize;xa+=16)
              {
                tl = getmaptileat(x+xa,y+ya);
                if(TileProperty[tl][BEHAVIOR] == 65534)
                {
                	g_pGraphics->drawTilewithmask(x+xa-scroll_x,y+ya-scroll_y,tl,tl+1);
                }
                else if (TileProperty[tl][BEHAVIOR] == 65535)
                {
                   if ( TileProperty[tl][ANIMATION] > 1 )
                   {
                	  tl = (tl-tiles[tl].animOffset)+((tiles[tl].animOffset+curanimtileframe)%TileProperty[tl][ANIMATION]);
                   }
                   g_pGraphics->drawPrioritytile(x+xa-scroll_x,y+ya-scroll_y,tl);
                }
              }
            }
        }

      }
      if(i==0) break;
   }

}

void gamedo_render_drawdebug(void)
{
int tl,y;
/*int h;*/
char debugmsg[80];

   if (debugmode)
   {
      if (debugmode==1)
      {
        savew = 190;
        saveh = 80;
        g_pGraphics->saveArea(4,4,savew,saveh);
        y = 5-8;
        sprintf(debugmsg, "p1x/y: %ld/%d", player[0].x, player[0].y);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);
        sprintf(debugmsg, "p2x/y: %ld/%d", player[1].x, player[1].y);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);
        sprintf(debugmsg, "scroll_x/y = %d/%d", (unsigned int)scroll_x, (unsigned int)scroll_y);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);
        sprintf(debugmsg, "scrollbuf_x/y: %d/%d", scrollx_buf, scrolly_buf);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);
        sprintf(debugmsg, "iw,pw: %d/%d", player[0].inhibitwalking, player[0].pwalking);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);
        sprintf(debugmsg, "pinertia_x: %d", player[0].pinertia_x);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);
        sprintf(debugmsg, "psupt: (%d,%d)", player[0].psupportingtile, player[0].psupportingobject);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);

        sprintf(debugmsg, "lvl,tile = %d,%d", getlevelat((player[0].x>>CSF)+4, (player[0].y>>CSF)+9), tl);
        g_pGraphics->sb_font_draw( (unsigned char*) debugmsg, 5, y+=8);

/*
        sprintf(debugmsg, "NOH=%d", NessieObjectHandle);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "x,y=(%d,%d)", objects[NessieObjectHandle].x,objects[NessieObjectHandle].y);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF=(%d,%d)", objects[NessieObjectHandle].x>>CSF,objects[NessieObjectHandle].y>>CSF);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF>>4=(%d,%d)", objects[NessieObjectHandle].x>>CSF>>4,objects[NessieObjectHandle].y>>CSF>>4);
        sb_font_draw(debugmsg, 5, y+=8);

        sprintf(debugmsg, "nessiestate = %d", objects[NessieObjectHandle].ai.nessie.state);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "pausetimer = %d", objects[NessieObjectHandle].ai.nessie.pausetimer);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "pausex/y = (%d,%d)", objects[NessieObjectHandle].ai.nessie.pausex,objects[NessieObjectHandle].ai.nessie.pausey);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "destx/y = %d/%d", objects[NessieObjectHandle].ai.nessie.destx,objects[NessieObjectHandle].ai.nessie.desty);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF = %d/%d", objects[NessieObjectHandle].ai.nessie.destx>>CSF,objects[NessieObjectHandle].ai.nessie.desty>>CSF);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF>>4 = %d/%d", objects[NessieObjectHandle].ai.nessie.destx>>CSF>>4,objects[NessieObjectHandle].ai.nessie.desty>>CSF>>4);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "mort_swim_amt = %d", objects[NessieObjectHandle].ai.nessie.mortimer_swim_amt);
        sb_font_draw(debugmsg, 5, y+=8);

        h = objects[NessieObjectHandle].ai.nessie.tiletrailhead;

        sprintf(debugmsg, "tthead=%d", h);
        sb_font_draw(debugmsg, 5, y+=8);

        sprintf(debugmsg, "ttX=%d,%d,%d,%d,%d", objects[NessieObjectHandle].ai.nessie.tiletrailX[0],objects[NessieObjectHandle].ai.nessie.tiletrailX[1],objects[NessieObjectHandle].ai.nessie.tiletrailX[2],objects[NessieObjectHandle].ai.nessie.tiletrailX[3],objects[NessieObjectHandle].ai.nessie.tiletrailX[4]);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "ttY=%d,%d,%d,%d,%d", objects[NessieObjectHandle].ai.nessie.tiletrailY[0],objects[NessieObjectHandle].ai.nessie.tiletrailY[1],objects[NessieObjectHandle].ai.nessie.tiletrailY[2],objects[NessieObjectHandle].ai.nessie.tiletrailY[3],objects[NessieObjectHandle].ai.nessie.tiletrailY[4]);
        sb_font_draw(debugmsg, 5, y+=8);
*/
      }
      else if (debugmode==2)
      {
        savew = map.xsize+4;
        saveh = map.ysize+4;
        g_pGraphics->saveArea(4,4,savew,saveh);
        radar();
      }
   }
}

void gamedo_render_erasedebug(void)
{
   if (debugmode) g_pGraphics->restoreArea(4,4,savew,saveh);
}

void gamedo_render_eraseobjects(void)
{
int i;

   // erase all objects.
   // note that this is done in the reverse order they are drawn.
   // this is necessary or you will see corrupted pixels when
   // two objects are occupying the same space.
   for(i=0;i<MAX_OBJECTS;i++)
   {
      if (objects[i].exists && objects[i].onscreen)
      {
    	  g_pGraphics->eraseSprite(objects[i].scrx, objects[i].scry, objects[i].sprite, i);
      }
   }
}

extern int NumConsoleMessages;

// draws sprites, players, and debug messages (if debug mode is on),
// performs frameskipping and blits the display as needed,
// at end of functions erases all drawn objects from the scrollbuf.
void gamedo_RenderScreen(stCloneKeenPlus *pCKP)
{
   int x,y,bmnum;

   g_pGraphics->renderHQBitmap();

   gamedo_render_drawobjects(pCKP);

   if(pCKP != NULL)
   {
	   if (pCKP->Control.levelcontrol.gameovermode)
	   {
		   // figure out where to center the gameover bitmap and draw it
		   bmnum = g_pGraphics->getBitmapNumberFromName("GAMEOVER");
		   x = (320/2)-(bitmaps[bmnum].xsize/2);
		   y = (200/2)-(bitmaps[bmnum].ysize/2);
		   g_pGraphics->drawBitmap(x, y, bmnum);
	   }
   }

   g_pVideoDriver->sb_blit();	// blit scrollbuffer to display

   gamedo_render_erasedebug();
   gamedo_render_eraseobjects();

   curfps++;
}

int ctspace=0, lastctspace=0;
void gamedo_HandleFKeys(stCloneKeenPlus *pCKP)
{
int i;

    if (g_pInput->getHoldedKey(KC) &&
    		g_pInput->getHoldedKey(KT) &&
    		g_pInput->getHoldedKey(KSPACE))
       {
          ctspace = 1;
       }
       else ctspace = 0;

       if (ctspace && !lastctspace)
       {
              for(i=0;i<MAX_PLAYERS;i++)
              {
                 if (player[i].isPlaying)
                 {
                    give_keycard(DOOR_YELLOW, i);
                    give_keycard(DOOR_RED, i);
                    give_keycard(DOOR_GREEN, i);
                    give_keycard(DOOR_BLUE, i);

                    player[i].inventory.charges = 999;
                    player[i].inventory.HasPogo = 1;
                    player[i].inventory.lives = 10;

                    // Show a message like in the original game
          		    char **text;
          		    int i;

          		    text = (char**) malloc(4*sizeof(char*));

          		    for(i=0;i<4;i++)
          		    {
          		    	text[i]= (char*) malloc(MAX_STRING_LENGTH*sizeof(char));
          		    }

          		    strcpy(text[0], "You are now cheating!");
          		    strcpy(text[1], "You just got a pogo stick,");
          		    strcpy(text[2], "all the key cards, and");
          		    strcpy(text[3], "lots of ray gun charges.");

          		    showTextMB(4,text,pCKP);

          		    for(i=0;i<4;i++)
          		    {
          		    	free(text[i]);
          		    }
          		    free(text);
                 }
              }
         g_pVideoDriver->AddConsoleMsg("All items cheat");
       }

       lastctspace = ctspace;

       // GOD cheat -- toggle god mode
       if (g_pInput->getHoldedKey(KG) && g_pInput->getHoldedKey(KO) && g_pInput->getHoldedKey(KD))
       {
           for(i=0;i<MAX_PLAYERS;i++)
           {
              player[i].godmode ^= 1;
           }
           g_pVideoDriver->DeleteConsoleMsgs();
           if (player[0].godmode)
        	   g_pVideoDriver->AddConsoleMsg("God mode ON");
           else
        	   g_pVideoDriver->AddConsoleMsg("God mode OFF");

           g_pSound->playSound(SOUND_GUN_CLICK, PLAY_FORCE);

           // Show a message like in the original game
 		    char **text;

 		    text = (char**) malloc(sizeof(char*));

	    	text[0]= (char*) malloc(MAX_STRING_LENGTH*sizeof(char));

 		    if (player[0].godmode)
 			   strcpy(text[0], "Godmode enabled");
 		    else
 			   strcpy(text[0], "Godmode disabled");

 		    showTextMB(1,text,pCKP);

	    	free(text[0]);
 		    free(text);
       }


    if (pCKP->Option[OPT_CHEATS].value)
    {
            if (g_pInput->getHoldedKey(KTAB)) // noclip/revive
            {
              // resurrect any dead players. the rest of the KTAB magic is
              // scattered throughout the various functions.
              for(i=0;i<MAX_PLAYERS;i++)
              {
                 if (player[i].pdie)
                 {
                   player[i].pdie = PDIE_NODIE;
                   player[i].y -= (8<<CSF);
                 }
                 player[i].pfrozentime = 0;
              }
            }
            // F8 - frame by frame
            if(g_pInput->getPressedKey(KF8))
            {
              framebyframe = 1;
              #ifdef BUILD_SDL
              g_pVideoDriver->AddConsoleMsg("Frame-by-frame mode  F8:advance F7:stop");
              #endif
            }
            // F9 - exit level immediately
            if(g_pInput->getPressedKey(KF9))
            {
               endlevel(1, pCKP);
            }
            // F6 - onscreen debug--toggle through debug/radar/off
            if(g_pInput->getPressedKey(KF6))
            {
               debugmode++;
               if (debugmode>2) debugmode=0;
            }
            // F7 - accelerate mode/frame by frame frame advance
            if(g_pInput->getPressedKey(KF7))
            {
               if (!framebyframe) acceleratemode=1-acceleratemode;
            }
    }

    // F10 - change primary player
    if(g_pInput->getPressedKey(KF10))
    {
        primaryplayer++;
        if (primaryplayer>=numplayers) primaryplayer=0;
    }
    // F3 - save game
    if (g_pInput->getPressedKey(KF3))
       game_save_interface(pCKP);

}

void gamedo_fades(void)
{
    if (fade.mode != FADE_GO) return;

    if (fade.fadetimer > fade.rate)
    {
      if (fade.dir==FADE_IN)
      {
        if (fade.curamt < PAL_FADE_SHADES)
        {
          fade.curamt++;                // coming in from black
        }
        else
        {
          fade.curamt--;                // coming in from white-out
        }
        if (fade.curamt==PAL_FADE_SHADES)
        {
           fade.mode = FADE_COMPLETE;
        }
        g_pGraphics->fadePalette(fade.curamt);
      }
      else if (fade.dir==FADE_OUT)
      {
        fade.curamt--;
        if (fade.curamt==0) fade.mode = FADE_COMPLETE;
        g_pGraphics->fadePalette(fade.curamt);
      }
      fade.fadetimer = 0;
    }
    else
    {
      fade.fadetimer++;
    }
}

void gamedo_frameskipping(stCloneKeenPlus *pCKP)
{
	 if (framebyframe)
     {
       gamedo_RenderScreen(pCKP);
       return;
     }

     if (frameskiptimer >= g_pVideoDriver->getFrameskip())
     {
       gamedo_RenderScreen(pCKP);
       frameskiptimer = 0;
     } else frameskiptimer++;

}

// same as above but only does a sb_blit, not the full RenderScreen.
// used for intros etc.
void gamedo_frameskipping_blitonly(void)
{
    if (framebyframe)
    {
    	g_pVideoDriver->sb_blit();
    	return;
    }

    if (frameskiptimer >= g_pVideoDriver->getFrameskip())
    {
    	g_pVideoDriver->sb_blit();
    	frameskiptimer = 0;
    } else frameskiptimer++;
}
