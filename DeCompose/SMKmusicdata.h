


#ifndef FILEEXTENSIONS
#define FILEEXTENSIONS
#define unassembled_extension ".ssf"
#define assembled_extension ".sng"

#endif



#ifndef DEFAULT_SONGNAMES_LIST
#define DEFAULT_SONGNAMES_LIST

#define gv_theme_name "gv_theme"
#define podium_win_name "podium_win"
#define toad_rank_name "toad_rank"
#define final_lap_name "final_lap"
#define no_record_name "no_record"
#define bowser_rank_name "bowser_rank"
#define koopa_rank_name "koopa_rank"
#define rank_out_name "rank_out"
#define tt_countdown_name "tt_countdown"
#define race_countdown_name "race_countdown"
#define peach_rank_name "peach_rank"
#define race_qualified_name "race_qualified"
#define luigi_rank_name "luigi_rank"
#define mario_rank_name "mario_rank"
#define game_over_name "game_over"
#define race_fanfare_name "race_fanfare"
#define starman_name "starman"
#define dk_jr_rank_name "dk_jr_rank"
#define yoshi_rank_name "yoshi_rank"
#define dp_theme_name "dp_theme"
#define title_screen_name "title_screen"
#define kb_theme_name "kb_theme"
#define bc_theme_name "bc_theme"
#define ci_theme_name "ci_theme"
#define menu_name "menu"
#define rr_theme_name "rr_theme"
#define mc_theme_name "mc_theme"
#define battle_theme_name "battle_theme"
#define vl_theme_name "vl_theme"
#define credits_name "credits"
#define podium_lose_name "podium_lose"


constexpr const char* SONG_NAMES[] = {
	gv_theme_name,
	podium_win_name,
	toad_rank_name,
	final_lap_name,
	no_record_name,
	bowser_rank_name,
	koopa_rank_name,
	rank_out_name,
	tt_countdown_name,
	race_countdown_name,
	peach_rank_name,
	race_qualified_name,
	luigi_rank_name,
	mario_rank_name,
	game_over_name,
	race_fanfare_name,
	starman_name,
	dk_jr_rank_name,
	yoshi_rank_name,
	dp_theme_name,
	title_screen_name,
	kb_theme_name,
	bc_theme_name,
	ci_theme_name,
	menu_name,
	rr_theme_name,
	mc_theme_name,
	battle_theme_name,
	vl_theme_name,
	credits_name,
	podium_lose_name
};


constexpr int SONG_ROM_OFFSETS[] = {
	0x000334,
	0x021a83,
	0x0280b8,
	0x0281d9,

	0x028398,
	0x0284c3,
	0x0285bb,
	0x0286ad,

	0x028787,
	0x0287be,
	0x0289c4,
	0x028b34,

	0x028c72,
	0x028d76,
	0x028e6e,
	0x028f39,

	0x0298bc,
	0x029a4b,
	0x029bd4,
	0x030b9d,

	0x049801,
	0x04c800,
	0x04cc3a,
	0x04cfaf,

	0x061e28,
	0x06fa73,
	0x076301,
	0x076876,
	
	0x076d16,
	0x07f18d,
	0x07fbe1
};


constexpr int DEFAULT_FILE_SIZES[] =
{
	0x2a2, //gv theme
	0x210,
	0x121, // toad rank
	0xec, 

	0x12b,
	0xf8,
	0xf2,
	0xda,

	0x33,
	0x206,
	0x170,
	0x13e,

	0x104,
	0xf8,
	0xc7, // maybe?
	0xc0, // race fanfare

	0x18f,
	0x189,
	0x14f, //yoshi
	0x49f,

	0x419,
	0x43a,
	0x375,
	0x527,

	0x11e,
	0x551,
	0x575,
	0x4a0,

	0x474,
	0xa54,
	0x1d9
};

constexpr int SONG_SPC_OFFSETS[] =
{
	0xd000,
	0xd000,
	0x3d24,
	0x3e45,

	0x03db,
	0x0506,
	0x05fe,
	0x06f0,

	0x07ca,
	0xd783,
	0xd989,
	0xdaf9,

	0xdc37,
	0xdd3b,
	0xde33,
	0xff00,

	0x10bf,
	0x124e,
	0x13d7,
	0xd000,

	0xd000,
	0xd000,
	0xd000,
	0xd000,

	0xd000,
	0xd000,
	0xd000,
	0xd000,

	0xd000,
	0xd000,
	0xd000
};


constexpr bool is_compressed[] = { true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true };

/*
bool songNeedsCompression(int song_number) { 
	if (song_number > 30 || song_number < 0) return false;
	else return is_compressed[song_number]; 
};
*/



#endif
