// Keypress high level translation (mostly from quake... sorry)
#include <windows.h>
#include <stdio.h>
#include "WolfDef.h"

extern char KeyPressed; // FIXME: remove
extern int	LastKey;

keydest_t	key_dest=key_game;
int		shift_down=false;
int		key_lastpress;
int		key_count;			// incremented every key event

char	*keybindings[256];
bool	consolekeys[256];	// if true, can't be rebound while in console
bool	menubound[256];	// if true, can't be rebound while in menu
int		keyshift[256];		// key to map to if shift held down in console
int		key_repeats[256];	// if > 1, it is autorepeating
bool	keydown[256];

typedef struct
{
	char	*name;
	int		keynum;
} keyname_t;

keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},
	
	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"PAUSE", K_PAUSE},

	{"MWHEELUP", K_MWHEELUP},
	{"MWHEELDOWN", K_MWHEELDOWN},

	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL, 0}
};

// ------------------------- * Devider * -------------------------
/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum(char *str)
{
	keyname_t	*kn;
	
	if(!str || !str[0]) return -1;
	if(!str[1])	return str[0]; // single char!

	for(kn=keynames; kn->name; kn++)
	{
		if(!Q_strcasecmp(str, kn->name)) return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString(int keynum)
{
	keyname_t *kn;	
	static char tinystr[2];
	
	if(keynum==-1) return "<KEY NOT FOUND>";
	if(keynum>32 && keynum<127)
	{	// printable ascii
		tinystr[0]=keynum;
		tinystr[1]=0;
		return tinystr;
	}
	
	for(kn=keynames; kn->name; kn++)
		if(keynum==kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}

/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding(int keynum, char *binding)
{
	char *newb;
	int	l;
			
	if(keynum==-1) return;

// free old bindings
	if(keybindings[keynum])
	{
		free(keybindings[keynum]);
		keybindings[keynum]=NULL;
	}
			
// allocate memory for new binding
	l=Q_strlen(binding);
	if(!l) return; // removing binding, don't bother with malloc!
	newb=malloc(l+1);
	Q_strcpy(newb, binding);
	newb[l]=0;
	keybindings[keynum]=newb;
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f(void)
{
	int b;

	if(Cmd_Argc()!=2)
	{
		Con_Printf("unbind <key> : remove commands from a key\n");
		return;
	}
	
	b=Key_StringToKeynum(Cmd_Argv(1));
	if(b==-1)
	{
		Con_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding(b, "");
}

void Key_Unbindall_f(void)
{
	int n;
	
	for(n=0; n<256; n++)
		if(keybindings[n]) Key_SetBinding(n, "");
}

/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f(void)
{
	int i, c, b;
	char cmd[1024];
	
	c=Cmd_Argc();

	if(c!=2 && c!=3)
	{
		Con_Printf("bind <key> [command] : attach a command to a key\n");
		return;
	}
	
	b=Key_StringToKeynum(Cmd_Argv(1));
	if(b==-1)
	{
		Con_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if(c==2) // bind <key>: display current binding!
	{
		if(keybindings[b])
			Con_Printf("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b]);
		else
			Con_Printf("\"%s\" is not bound\n", Cmd_Argv(1));
		return;
	}
	
// copy the rest of the command line
	cmd[0]=0; // start out with a null string
	for(i=2; i<c; i++)
	{
		if(i>2)	strcat(cmd, " ");
		strcat(cmd, Cmd_Argv(i));
	}

	Key_SetBinding(b, cmd);
}

/*
============
Key_WriteBindings: Writes lines containing "bind key value"
============
*/
void Key_WriteBindings(FILE *f)
{
	int		i;

	for(i=0; i<256; i++)
		if(keybindings[i])
			if(*keybindings[i])
				fprintf(f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}

/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int	i;

//
// init ascii characters in console mode
//
	for(i=32 ; i<128 ; i++) consolekeys[i] = true;
	consolekeys[K_ENTER]=true;
	consolekeys[K_TAB]=true;
	consolekeys[K_LEFTARROW]=true;
	consolekeys[K_RIGHTARROW]=true;
	consolekeys[K_UPARROW]=true;
	consolekeys[K_DOWNARROW]=true;
	consolekeys[K_BACKSPACE]=true;
	consolekeys[K_PGUP]=true;
	consolekeys[K_PGDN]=true;
	consolekeys[K_DEL]=true;
	consolekeys[K_HOME]=true;
	consolekeys[K_END]=true;
	consolekeys[K_SHIFT]=true;
	consolekeys[K_MWHEELUP]=true;
	consolekeys[K_MWHEELDOWN]=true;
	consolekeys['`']=false;
	consolekeys['~']=false;

	for(i=0; i<256; i++) keyshift[i]=i;
	for(i='a'; i<='z'; i++)	keyshift[i]=i-'a'+'A';
	keyshift['1']='!';
	keyshift['2']='@';
	keyshift['3']='#';
	keyshift['4']='$';
	keyshift['5']='%';
	keyshift['6']='^';
	keyshift['7']='&';
	keyshift['8']='*';
	keyshift['9']='(';
	keyshift['0']=')';
	keyshift['-']='_';
	keyshift['=']='+';
	keyshift[',']='<';
	keyshift['.']='>';
	keyshift['/']='?';
	keyshift[';']=':';
	keyshift['\'']='"';
	keyshift['[']='{';
	keyshift[']']='}';
	keyshift['`']='~';
	keyshift['\\']='|';

	menubound[K_ESCAPE]=true;
	for(i=0; i<12; i++) menubound[K_F1+i] = true;

// register our functions
	Cmd_AddCommand("bind", Key_Bind_f);
	Cmd_AddCommand("unbind", Key_Unbind_f);
	Cmd_AddCommand("unbindall", Key_Unbindall_f);
}
// FIXME: add Shut down function!

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
void Key_Event(int key, bool down)
{
	char *kb;
	char cmd[1024];

	keydown[key]=down;

	if(!down)	key_repeats[key]=0;

	key_lastpress=key;
	key_count++;
	if(key_count<=0) return;		// just catching keys for Con_NotifyBox

// update auto-repeat status
	if(down)
	{
		key_repeats[key]++;
		if(key!=K_BACKSPACE && key!=K_PAUSE && key_repeats[key] > 1) 
			return;	// ignore most autorepeats

/* As a legacy of Quake source. FIXME /I do not need it?/
		if(key>=200 && !keybindings[key])
			Con_Printf("%s is unbound, hit F4 to set.\n", Key_KeynumToString(key) );
*/
	}

	if(key==K_SHIFT) shift_down=down;

//
// handle escape specialy, so the user can never unbind it
//
	if(key==K_ESCAPE)
	{
		if(!down)	return;
		switch(key_dest)
		{
		case key_message:
			//Key_Message(key);
			break;
		case key_menu:
			M_Keydown(key);
			break;
		case key_game:
		case key_console:
			M_ToggleMenu_f();
			break;
		default:
			Con_Printf("Bad key_dest");
		}
		return;
	}

//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
	if(!down)
	{
		kb=keybindings[key];
		if(kb && kb[0]=='+')
		{
			sprintf(cmd, "-%s %d\n", kb+1, key);
			Cbuf_AddText(cmd);
		}
		if(keyshift[key]!=key)
		{
			kb=keybindings[keyshift[key]];
			if(kb && kb[0]=='+')
			{
				sprintf(cmd, "-%s %d\n", kb+1, key);
				Cbuf_AddText(cmd);
			}
		}
		return;
	}

/* Good idea! use it later!
//
// during demo playback, most keys bring up the main menu
//
	if(cls.demoplayback && down && consolekeys[key] && key_dest==key_game)
	{
		M_ToggleMenu_f ();
		return;
	}
*/

//
// if not a consolekey, send to the interpreter no matter what mode is
//
	if( (key_dest==key_menu		 && menubound[key])
	||	(key_dest==key_console && !consolekeys[key])
	||	(key_dest==key_game		 && ( !con_forcedup || !consolekeys[key] ) ) )
	{
		kb=keybindings[key];
		if(kb)
		{
			if(kb[0]=='+')
			{	// button commands add keynum as a parm
				sprintf(cmd, "%s %i\n", kb, key);
				Cbuf_AddText(cmd);
			}
			else
			{
				Cbuf_AddText(kb);
				Cbuf_AddText("\n");
			}
		}
		return;
	}

	if(shift_down)
	{
		key=keyshift[key];
	}

	switch(key_dest)
	{
	case key_message:
//		Key_Message(key);
		break;
	case key_menu:
		M_Keydown(key);
		break;

	case key_game:
	case key_console:
		Con_KeyInput(key);
		break;
	default:
		Con_Printf("Bad key_dest");
	}
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates(void)
{
	int i;

	for(i=0; i<256; i++)
	{
		keydown[i]=false;
		key_repeats[i]=0;
	}
}