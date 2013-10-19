#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/HPMi.h"
#include "../map/script.h"
#include "../map/pc.h"
#include "../map/map.h"
#include "../map/unit.h"
#include "../map/atcommand.h"
#include "../common/nullpo.h"

#define OPTION_AUTOATTACK 0x10000000



HPExport struct hplugin_info pinfo = {
	"autoattack",		// Plugin name
	SERVER_TYPE_MAP,	// Which server types this plugin works with?
	"1.0",				// Plugin version
	HPM_VERSION,		// HPM Version (don't change, macro is automatically updated)
};

static int buildin_autoattack_sub(struct block_list *bl,va_list ap)
{
	int *target_id=va_arg(ap,int *);
	*target_id = bl->id;
	return 1;
}

void autoattack_motion(struct map_session_data* sd)
{
	int i, target_id;
	for(i=0;i<=9;i++)
	{
		target_id=0;
		map->foreachinarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x-i, sd->bl.y-i, sd->bl.x+i, sd->bl.y+i, BL_MOB, &target_id);
		if(target_id)
		{
			unit->attack(&sd->bl,target_id,1);
			break;			
		}
		target_id=0;
	}
	if(!target_id)
	{
		unit->walktoxy(&sd->bl,sd->bl.x+(rand()%2==0?-1:1)*(rand()%10),sd->bl.y+(rand()%2==0?-1:1)*(rand()%10),0);
	}
	return;
}

int autoattack_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd=NULL;

	sd=map->id2sd(id);
	if(sd==NULL)
		return 0;
	if(sd->sc.option & OPTION_AUTOATTACK)
	{
		autoattack_motion(sd);
		timer->add(timer->gettick()+2000,autoattack_timer,sd->bl.id,0);
	}
	return 0;
}

ACMD(autoatk)
{
	//nullpo_retr(-1, sd);
	if (sd->sc.option & OPTION_AUTOATTACK)
	{
		sd->sc.option &= ~OPTION_AUTOATTACK;
		unit->stop_attack(&sd->bl);
		clif->message(fd,"Auto attack off");
		return true;
	} else
	{
		sd->sc.option |= OPTION_AUTOATTACK;
		timer->add(timer->gettick()+200,autoattack_timer,sd->bl.id,0);
		clif->message(fd,"Auto attack on");
	}
	clif->changeoption(&sd->bl);
	return true;
}

/* Server Startup */
HPExport void plugin_init (void)
{
	clif = GET_SYMBOL("clif");
    script = GET_SYMBOL("script");
	pc = GET_SYMBOL("pc");
	atcommand = GET_SYMBOL("atcommand");
	map = GET_SYMBOL("map");
	unit = GET_SYMBOL("unit");
	timer = GET_SYMBOL("timer");

    if( HPMi->addCommand != NULL ) {//link our '@sample' command
		HPMi->addCommand("autoatk",ACMD_A(autoatk));
	}
}