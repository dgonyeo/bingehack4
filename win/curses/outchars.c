/* Copyright (c) Daniel Thaler, 2011 */
/* NetHack may be freely redistributed.  See license for details. */

/* NOTE: This file is utf-8 encoded; saving with a non utf-8 aware editor WILL
 * damage some symbols */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "nhcurses.h"

#define array_size(x) (sizeof(x)/sizeof(x[0]))


static int corpse_id;
struct curses_drawing_info *default_drawing, *cur_drawing;
static struct curses_drawing_info *ibm_drawing, *dec_drawing, *rogue_drawing;


static struct curses_symdef ibm_graphics_ovr[] = {
    /* bg */
    {"vwall",	-1,	{0x2502, 0},	0xb3},	/* │ vertical rule */
    {"hwall",	-1,	{0x2500, 0},	0xc4},	/* ─ horizontal rule */
    {"tlcorn",	-1,	{0x250C, 0},	0xda},	/* ┌ top left corner */
    {"trcorn",	-1,	{0x2510, 0},	0xbf},	/* ┐ top right corner */
    {"blcorn",	-1,	{0x2514, 0},	0xc0},	/* └ bottom left */
    {"brcorn",	-1,	{0x2518, 0},	0xd9},	/* ┘ bottom right */
    {"crwall",	-1,	{0x253C, 0},	0xc5},	/* ┼ cross */
    {"tuwall",	-1,	{0x2534, 0},	0xc1},	/* T up */
    {"tdwall",	-1,	{0x252C, 0},	0xc2},	/* T down */
    {"tlwall",	-1,	{0x2524, 0},	0xb4},	/* T left */
    {"trwall",	-1,	{0x251C, 0},	0xc3},	/* T right */
    {"ndoor",	-1,	{0x00B7, 0},	0xfa},	/* · centered dot */
    {"vodoor",	-1,	{0x25A0, 0},	0xfe},	/* ■ small centered square */
    {"hodoor",	-1,	{0x25A0, 0},	0xfe},	/* ■ small centered square */
    {"bars",	-1,	{0x2261, 0},	0xf0},	/* ≡ equivalence symbol */
    {"tree",	-1,	{0x00B1, 0},	0xf1},	/* ± plus or minus symbol */
    {"room",	-1,	{0x00B7, 0},	0xfa},	/* · centered dot */
    {"corr",	-1,	{0x2591, 0},	0xb0},	/* ░ light shading */
    {"litcorr",	-1,	{0x2592, 0},	0xb1},	/* ▒ medium shading */
    {"fountain",-1,	{0x2320, 0},	0xf4},	/* ⌠ integral top half */
    {"pool",	-1,	{0x2248, 0},	0xf7},	/* ≈ approx. equals */
    {"ice",	-1,	{0x00B7, 0},	0xfa},	/* · centered dot */
    {"lava",	-1,	{0x2248, 0},	0xf7},	/* ≈ approx. equals */
    {"vodbridge",-1,	{0x00B7, 0},	0xfa},	/* · centered dot */
    {"hodbridge",-1,	{0x00B7, 0},	0xfa},	/* · centered dot */
    {"water",	-1,	{0x2248, 0},	0xf7},	/* ≈ approx. equals */

    /* zap */
    {"zap_v",	-1,	{0x2502, 0},	0xb3},	/* │ vertical rule */
    {"zap_h",	-1,	{0x2500, 0},	0xc4},	/* ─ horizontal rule */

    /* swallow */
    {"swallow_mid_l",-1,{0x2502, 0},	0xb3},	/* │ vertical rule */
    {"swallow_mid_r",-1,{0x2502, 0},	0xb3},	/* │ vertical rule */
    
    /* explosion */
    {"exp_mid_l",-1,	{0x2502, 0},	0xb3},	/* │ vertical rule */
    {"exp_mid_r",-1,	{0x2502, 0},	0xb3},	/* │ vertical rule */
};


static struct curses_symdef dec_graphics_ovr[] = {
    /* bg */
    {"vwall",	-1,	{0x2502, 0},	0xf8},	/* │ vertical rule */
    {"hwall",	-1,	{0x2500, 0},	0xf1},	/* ─ horizontal rule */
    {"tlcorn",	-1,	{0x250C, 0},	0xec},	/* ┌ top left corner */
    {"trcorn",	-1,	{0x2510, 0},	0xeb},	/* ┐ top right corner */
    {"blcorn",	-1,	{0x2514, 0},	0xed},	/* └ bottom left */
    {"brcorn",	-1,	{0x2518, 0},	0xea},	/* ┘ bottom right */
    {"crwall",	-1,	{0x253C, 0},	0xee},	/* ┼ cross */
    {"tuwall",	-1,	{0x2534, 0},	0xf6},	/* T up */
    {"tdwall",	-1,	{0x252C, 0},	0xf7},	/* T down */
    {"tlwall",	-1,	{0x2524, 0},	0xf5},	/* T left */
    {"trwall",	-1,	{0x251C, 0},	0xf4},	/* T right */
    {"ndoor",	-1,	{0x00B7, 0},	0xfe},	/* · centered dot */
    {"vodoor",	-1,	{0x2588, 0},	0xe1}, 	/* █ solid block */
    {"hodoor",	-1,	{0x2588, 0},	0xe1}, 	/* █ solid block */
    {"bars",	-1,	{0x03C0, 0},	0xfb},	/* π small pi */
    {"tree",	-1,	{0x00B1, 0},	0xe7},	/* ± plus-or-minus */
    {"room",	-1,	{0x00B7, 0},	0xfe},	/* · centered dot */
    {"upladder",-1,	{0x2265, 0},	0xf9},	/* ≥ greater-than-or-equals */
    {"dnladder",-1,	{0x2264, 0},	0xfa},	/* ≤ less-than-or-equals */
    {"pool",	-1,	{0x25C6, 0},	0xe0},	/* ◆ diamond */
    {"ice",	-1,	{0x00B7, 0},	0xfe},	/* · centered dot */
    {"lava",	-1,	{0x25C6, 0},	0xe0},	/* ◆ diamond */
    {"vodbridge",-1,	{0x00B7, 0},	0xfe},	/* · centered dot */
    {"hodbridge",-1,	{0x00B7, 0},	0xfe},	/* · centered dot */
    {"water",	-1,	{0x25C6, 0},	0xe0},	/* ◆ diamond */
    
    /* zap */
    {"zap_v",	-1,	{0x2502, 0},	0xf8}, /* │ vertical rule */
    {"zap_h",	-1,	{0x2500, 0},	0xf1}, /* ─ horizontal rule */
    
    /* swallow */
    {"swallow_top_c",-1,{0x23BA, 0},	0xef}, /* ⎺ high horizontal line */
    {"swallow_mid_l",-1,{0x2502, 0},	0xf8}, /* │ vertical rule */
    {"swallow_mid_r",-1,{0x2502, 0},	0xf8}, /* │ vertical rule */
    {"swallow_bot_c",-1,{0x23BD, 0},	0xf3}, /* ⎽ low horizontal line */
    
    /* explosion */
    {"exp_top_c", -1,	{0x23BA, 0},	0xef}, /* ⎺ high horizontal line */
    {"exp_mid_l", -1,	{0x2502, 0},	0xf8}, /* │ vertical rule */
    {"exp_mid_r", -1,	{0x2502, 0},	0xf8}, /* │ vertical rule */
    {"exp_bot_c", -1,	{0x23BD, 0},	0xf3}, /* ⎽ low horizontal line */
};


static struct curses_symdef rogue_graphics_ovr[] = {
    {"vodoor",	-1,	{0x002B, 0},	'+'},
    {"hodoor",	-1,	{0x002B, 0},	'+'},
    {"ndoor",	-1,	{0x002B, 0},	'+'},
    {"upstair",	-1,	{0x0025, 0},	'%'},
    {"dnstair",	-1,	{0x0025, 0},	'%'},
    
    {"gold piece",-1,	{0x002A, 0},	'*'},
    
    {"corpse",	-1,	{0x003A, 0},	':'}, /* the 2 most common food items... */
    {"food ration",-1,	{0x003A, 0},	':'}
};


static boolean apply_override_list(struct curses_symdef *list, int len,
				   const struct curses_symdef *ovr)
{
    int i;
    for (i = 0; i < len; i++)
	if (!strcmp(list[i].symname, ovr->symname)) {
	    list[i].ch = ovr->ch;
	    memcpy(list[i].unichar, ovr->unichar, sizeof(wchar_t) * CCHARW_MAX);
	    if (ovr->color != -1)
		list[i].color = ovr->color;
	    return TRUE;
	}
    return FALSE;
}


static void apply_override(struct curses_drawing_info *di,
			   const struct curses_symdef *ovr, int olen)
{
    int i;
    boolean ok;
    
    for (i = 0; i < olen; i++) {
	ok = FALSE;
	/* the override will effect exactly one of the symbol lists */
	ok |= apply_override_list(di->bgelements, di->num_bgelements, &ovr[i]);
	ok |= apply_override_list(di->traps, di->num_traps, &ovr[i]);
	ok |= apply_override_list(di->objects, di->num_objects, &ovr[i]);
	ok |= apply_override_list(di->effects, di->num_effects, &ovr[i]);
	ok |= apply_override_list(di->explsyms, NUMEXPCHARS, &ovr[i]);
	ok |= apply_override_list(di->swallowsyms, NUMSWALLOWCHARS, &ovr[i]);
	ok |= apply_override_list(di->zapsyms, NUMZAPCHARS, &ovr[i]);
	
	if (!ok)
	    fprintf(stdout, "sym override %s could not be applied\n", ovr[i].symname);
    }
}


static struct curses_symdef *load_nh_symarray(const struct nh_symdef *src, int len)
{
    int i;
    struct curses_symdef *copy = malloc(len * sizeof(struct curses_symdef));
    memset(copy, 0, len * sizeof(struct curses_symdef));
    
    for (i = 0; i < len; i++) {
	copy[i].symname = strdup(src[i].symname);
	copy[i].ch = src[i].ch;
	copy[i].color = src[i].color;
	
	/* this works because ASCII 0x?? (for ?? < 128) == Unicode U+00?? */
	copy[i].unichar[0] = (wchar_t)src[i].ch; 
    }
    
    return copy;
}


static struct curses_drawing_info *load_nh_drawing_info(const struct nh_drawing_info *orig)
{
    struct curses_drawing_info *copy = malloc(sizeof(struct curses_drawing_info));
    
    copy->num_bgelements = orig->num_bgelements;
    copy->num_traps = orig->num_traps;
    copy->num_objects = orig->num_objects;
    copy->num_monsters = orig->num_monsters;
    copy->num_warnings = orig->num_warnings;
    copy->num_expltypes = orig->num_expltypes;
    copy->num_zaptypes = orig->num_zaptypes;
    copy->num_effects = orig->num_effects;
    
    copy->bgelements = load_nh_symarray(orig->bgelements, orig->num_bgelements);
    copy->traps = load_nh_symarray(orig->traps, orig->num_traps);
    copy->objects = load_nh_symarray(orig->objects, orig->num_objects);
    copy->monsters = load_nh_symarray(orig->monsters, orig->num_monsters);
    copy->warnings = load_nh_symarray(orig->warnings, orig->num_warnings);
    copy->invis = load_nh_symarray(orig->invis, 1);
    copy->effects = load_nh_symarray(orig->effects, orig->num_effects);
    copy->expltypes = load_nh_symarray(orig->expltypes, orig->num_expltypes);
    copy->explsyms = load_nh_symarray(orig->explsyms, NUMEXPCHARS);
    copy->zaptypes = load_nh_symarray(orig->zaptypes, orig->num_zaptypes);
    copy->zapsyms = load_nh_symarray(orig->zapsyms, NUMZAPCHARS);
    copy->swallowsyms = load_nh_symarray(orig->swallowsyms, NUMSWALLOWCHARS);
    
    return copy;
}


void init_displaychars(void)
{
    int i;
    struct nh_drawing_info *dinfo = nh_get_drawing_info();
    
    default_drawing = load_nh_drawing_info(dinfo);
    ibm_drawing = load_nh_drawing_info(dinfo);
    dec_drawing = load_nh_drawing_info(dinfo);
    rogue_drawing = load_nh_drawing_info(dinfo);
    
    apply_override(ibm_drawing, ibm_graphics_ovr, array_size(ibm_graphics_ovr));
    apply_override(dec_drawing, dec_graphics_ovr, array_size(dec_graphics_ovr));
    apply_override(rogue_drawing, rogue_graphics_ovr, array_size(rogue_graphics_ovr));
    
    cur_drawing = default_drawing;
    
    /* find objects that need special treatment */
    for (i = 0; i < cur_drawing->num_objects; i++) {
	if (!strcmp("corpse", cur_drawing->objects[i].symname))
	    corpse_id = i;
    }
    
    /* options are parsed before display is initialized, so redo switch */
    switch_graphics(settings.graphics);
}


static void free_symarray(struct curses_symdef *array, int len)
{
    int i;
    for (i = 0; i < len; i++)
	free((char*)array[i].symname);
    
    free(array);
}


static void free_drawing_info(struct curses_drawing_info *di)
{
    free_symarray(di->bgelements, di->num_bgelements);
    free_symarray(di->traps, di->num_traps);
    free_symarray(di->objects, di->num_objects);
    free_symarray(di->monsters, di->num_monsters);
    free_symarray(di->warnings, di->num_warnings);
    free_symarray(di->invis, 1);
    free_symarray(di->effects, di->num_effects);
    free_symarray(di->expltypes, di->num_expltypes);
    free_symarray(di->explsyms, NUMEXPCHARS);
    free_symarray(di->zaptypes, di->num_zaptypes);
    free_symarray(di->zapsyms, NUMZAPCHARS);
    free_symarray(di->swallowsyms, NUMSWALLOWCHARS);
    
    free(di);
}


void free_displaychars(void)
{
    free_drawing_info(default_drawing);
    free_drawing_info(ibm_drawing);
    free_drawing_info(dec_drawing);
    free_drawing_info(rogue_drawing);
    
    default_drawing = ibm_drawing = dec_drawing = rogue_drawing = NULL;
}


int mapglyph(struct nh_dbuf_entry *dbe, struct curses_symdef *syms)
{
    int id, count = 0;

    if (dbe->effect) {
	id = NH_EFFECT_ID(dbe->effect);
	
	switch (NH_EFFECT_TYPE(dbe->effect)) {
	    case E_EXPLOSION:
		syms[0] = cur_drawing->explsyms[id % NUMEXPCHARS];
		syms[0].color = cur_drawing->expltypes[id / NUMEXPCHARS].color;
		break;
		
	    case E_SWALLOW:
		syms[0] = cur_drawing->swallowsyms[id & 0x7];
		syms[0].color = cur_drawing->monsters[id >> 3].color;
		break;
		
	    case E_ZAP:
		syms[0] = cur_drawing->zapsyms[id & 0x3];
		syms[0].color = cur_drawing->zaptypes[id >> 2].color;
		break;
		
	    case E_MISC:
		syms[0] = cur_drawing->effects[id];
		syms[0].color = cur_drawing->effects[id].color;
		break;
	}
	
	return 1; /* we don't want to show other glyphs under effects */
    }
    
    if (dbe->invis)
	syms[count++] = cur_drawing->invis[0];
    
    else if (dbe->mon) {
	if (dbe->mon > cur_drawing->num_monsters && (dbe->monflags & MON_WARNING)) {
	    id = dbe->mon - 1 - cur_drawing->num_monsters;
	    syms[count++] = cur_drawing->warnings[id];
	} else {
	    id = dbe->mon - 1;
	    syms[count++] = cur_drawing->monsters[id];
	}
    }
    
    if (dbe->obj) {
	id = dbe->obj - 1;
	if (id == corpse_id) {
	    syms[count] = cur_drawing->objects[id];
	    syms[count].color = cur_drawing->monsters[dbe->obj_mn - 1].color;
	    count++;
	} else
	    syms[count++] = cur_drawing->objects[id];
    }
    
    if (dbe->trap) {
	id = dbe->trap - 1;
	syms[count++] = cur_drawing->traps[id];
    } 
    
    /* omit the background symbol from the list if it is boring */
    if (count == 0 ||
	default_drawing->bgelements[dbe->bg].ch == '<' ||
	default_drawing->bgelements[dbe->bg].ch == '>')
	syms[count++] = cur_drawing->bgelements[dbe->bg];

    return count; /* count <= 4 */
}


void set_rogue_level(boolean enable)
{
    if (enable)
	cur_drawing = rogue_drawing;
    else
	switch_graphics(settings.graphics);
}


void curses_notify_level_changed(int dmode)
{
    set_rogue_level(dmode == LDM_ROGUE);
}


void switch_graphics(enum nh_text_mode mode)
{
    switch (mode) {
	default:
	case ASCII_GRAPHICS:
	    cur_drawing = default_drawing;
	    break;
	    
	case IBM_GRAPHICS:
/*
 * Use the nice IBM Extended ASCII line-drawing characters (codepage 437).
 * This fails on UTF-8 terminals and on terminals that use other codepages.
 */
	    cur_drawing = ibm_drawing;
	    break;
	    
	case DEC_GRAPHICS:
/*
 * Use the VT100 line drawing character set.
 * VT100 emulation is very common on Unix, so this should generally work.
 */
	    cur_drawing = dec_drawing;
	    break;
    }
}


int curses_color_attr(int nh_color)
{
    int color = nh_color + 1;
    int cattr = A_NORMAL;
    
    if (COLORS < 16 && color > 8) {
	color -= 8;
	cattr = A_BOLD;
    }
    cattr |= COLOR_PAIR(color);
    return cattr;
}


void print_sym(WINDOW *win, struct curses_symdef *sym, int extra_attrs)
{
    int attr;
    cchar_t uni_out;
    
    /* nethack color index -> curses color */
    attr = A_NORMAL | extra_attrs;
    if (ui_flags.color)
	attr |= curses_color_attr(sym->color);
    
    /* print it; preferably as unicode */
    if (sym->unichar[0] && ui_flags.unicode && settings.unicode) {
	int color = PAIR_NUMBER(attr);
	setcchar(&uni_out, sym->unichar, attr, color, NULL);
	wadd_wch(win, &uni_out);
    } else {
	wattron(win, attr);
	waddch(win, sym->ch);
	wattroff(win, attr);
    }
}

/*mapglyph.c*/