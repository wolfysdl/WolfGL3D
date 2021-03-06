// these are the key numbers that should be passed to Key_Event
#define	K_TAB				9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW			128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL		133
#define	K_SHIFT		134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN		149
#define	K_PGUP		150
#define	K_HOME		151
#define	K_END			152

#define K_PAUSE		255

// mouse buttons generate virtual keys
#define	K_MOUSE1	200
#define	K_MOUSE2	201
#define	K_MOUSE3	202

// Intellimouse(c) Mouse Wheel Support
#define K_MWHEELUP		239
#define K_MWHEELDOWN	240

typedef enum {key_game, key_console, key_message, key_menu} keydest_t;

extern keydest_t	key_dest;
extern char *keybindings[256];
extern	int	 key_repeats[256];
extern	int	 key_count;			// incremented every key event
extern	int	 key_lastpress;

extern void Key_Event(int key, bool down);
extern void Key_Init(void);
extern void Key_WriteBindings(FILE *f);
extern void Key_SetBinding(int keynum, char *binding);
extern void Key_ClearStates(void);
extern char *Key_KeynumToString(int keynum);
extern void Key_ClearStates(void);
