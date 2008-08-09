/*
 *  tenace - bridge hand viewer and editor
 *  Copyright (C) 2005-2008 Christoph Berg <cb@df7cb.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <assert.h>
#include <cairo/cairo-pdf.h>
#include <math.h>

#include "functions.h"
#include "window_board.h"

#define STRIDE 0.25 * 72
#define L 1.0 * 72
#define C 2.0 * 72
#define R 3.0 * 72
#define T 1.0 * 72
#define M 2.0 * 72
#define B 3.0 * 72

static void
draw_page (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint               page_nr,
           gpointer           user_data)

{
	cairo_t *cr;
	cr = gtk_print_context_get_cairo_context (context);

	board *b = CUR_BOARD;

	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 12);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);

	cairo_move_to (cr, 0.1 * 72, 0.2 * 72);
	cairo_show_text (cr, b->name->str);
	cairo_stroke (cr);

	cairo_move_to (cr, 0.1 * 72, 0.5 * 72);

	cairo_show_text (cr, contract_string(b->level, b->trumps, b->declarer, b->doubled));
	cairo_stroke (cr);

	cairo_move_to (cr, C, T+1*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, north, spade));
	cairo_move_to (cr, C, T+2*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, north, heart));
	cairo_move_to (cr, C, T+3*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, north, diamond));
	cairo_move_to (cr, C, T+4*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, north, club));

	cairo_move_to (cr, L, M+1*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, west, spade));
	cairo_move_to (cr, L, M+2*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, west, heart));
	cairo_move_to (cr, L, M+3*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, west, diamond));
	cairo_move_to (cr, L, M+4*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, west, club));

	cairo_move_to (cr, R, M+1*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, east, spade));
	cairo_move_to (cr, R, M+2*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, east, heart));
	cairo_move_to (cr, R, M+3*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, east, diamond));
	cairo_move_to (cr, R, M+4*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, east, club));

	cairo_move_to (cr, C, B+1*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, south, spade));
	cairo_move_to (cr, C, B+2*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, south, heart));
	cairo_move_to (cr, C, B+3*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, south, diamond));
	cairo_move_to (cr, C, B+4*STRIDE);
	cairo_show_text (cr, hand_suit_string (b, south, club));
	cairo_stroke (cr);

	cairo_move_to (cr, C, M);
	cairo_line_to (cr, C, R);
	cairo_line_to (cr, B, R);
	cairo_line_to (cr, B, C);
	cairo_close_path (cr);
	cairo_stroke (cr);
}

static void
arrow (cairo_t *cr, int n, seat dir, double size)
{
#define ARHEAD 0.0
#define ARWIDTH 0.2
	cairo_save (cr);
	cairo_translate (cr, 0.5*size, 0.5*size);

	/* background box */
	if ((n + (n - 1) / 6) % 2 == 0) {
		cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
		cairo_move_to (cr, -0.5*size, -0.5*size);
		cairo_line_to (cr, 0.5*size, -0.5*size);
		cairo_line_to (cr, 0.5*size, 0.5*size);
		cairo_line_to (cr, -0.5*size, 0.5*size);
		cairo_close_path (cr);
		cairo_fill (cr);
	}

	/* arrow */
	cairo_save (cr);
	cairo_rotate (cr, ((int)dir - 2) * M_PI_2);

	if (n % 2)
		cairo_set_source_rgb (cr, 0.7, 0.4, 0.4);
	else
		cairo_set_source_rgb (cr, 0.4, 0.4, 0.7);

	cairo_move_to (cr, 0.0, -0.5*size);
	cairo_line_to (cr, 0.5*size, -ARHEAD*size);
	cairo_line_to (cr, ARWIDTH*size, -ARHEAD*size);
	cairo_line_to (cr, ARWIDTH*size, 0.5*size);
	cairo_line_to (cr, -ARWIDTH*size, 0.5*size);
	cairo_line_to (cr, -ARWIDTH*size, -ARHEAD*size);
	cairo_line_to (cr, -0.5*size, -ARHEAD*size);
	cairo_close_path (cr);
	cairo_fill (cr);

	cairo_restore (cr);

	/* board number */
	char buf[8];
	cairo_text_extents_t ext;
	snprintf (buf, sizeof (buf), "%d", n);
	cairo_select_font_face (cr, "Sans",
		CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, size / 2.0);
	cairo_text_extents (cr, buf, &ext);

	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	cairo_move_to (cr, -ext.width / 2 - ext.x_bearing, ext.height/2);
	cairo_show_text (cr, buf);

	cairo_restore (cr);
}

static void
cairo_clip_rectangle (cairo_t *cr, double x0, double y0, double x1, double y1)
{
	cairo_move_to (cr, x0, y0);
	cairo_line_to (cr, x1, y0);
	cairo_line_to (cr, x1, y1);
	cairo_line_to (cr, x0, y1);
	cairo_close_path (cr);
	cairo_clip (cr);
}

#define ARROWSIZE 18.0

static void
magic_card_half (cairo_t *cr, window_board_t *win, card c)
{
	int n;
	for (n = 0; n < win->n_boards; n++) {
		if (n >= 24)
			break;

		cairo_save (cr);
		cairo_translate (cr, (n % 6) * ARROWSIZE, (n / 6) * ARROWSIZE);

		arrow (cr, n + 1, win->boards[n]->dealt_cards[c], ARROWSIZE);
		cairo_restore (cr);

		/* draw title */
		cairo_text_extents_t ext;
		cairo_save (cr);

		cairo_select_font_face (cr, "Sans",
			CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, 8);
		cairo_clip_rectangle (cr, 0.0, 0.0, 6 * ARROWSIZE, 9 * ARROWSIZE);
		cairo_move_to (cr, 2.0, 4.45 * ARROWSIZE);
		cairo_show_text (cr, win->title);

		cairo_text_extents (cr, win->title, &ext);
		/* clip subtitle to card width - title width */
		cairo_clip_rectangle (cr, 0.0, 0.0,
			6 * ARROWSIZE - ext.x_advance - 4.0, 9 * ARROWSIZE);
		cairo_move_to (cr, 2.0, 4.85 * ARROWSIZE);
		cairo_show_text (cr, win->subtitle);
		cairo_stroke (cr);

		cairo_restore (cr);
	}
}

static void
magic_card (cairo_t *cr, window_board_t *win, card c)
{
	/* upper part */
	magic_card_half (cr, win, c);
	/* lower part */
	cairo_save (cr);
	cairo_translate (cr, 6 * ARROWSIZE, 9 * ARROWSIZE);
	cairo_rotate (cr, M_PI);
	magic_card_half (cr, win, c);
	cairo_restore (cr);
}

#define N_COL 3
#define N_ROW 7
#define N_PAGE 3

static void
magic_draw_page (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint               page_nr,
           gpointer           user_data)

{
	cairo_t *cr;
	cr = gtk_print_context_get_cairo_context (context);
	double col_width = gtk_print_context_get_width (context) / N_COL;
	double row_height = gtk_print_context_get_height (context) / N_ROW;

	assert (page_nr < N_PAGE);
	static int cc, print_head;
	if (page_nr == 0) { /* restart printing */
		cc = 51;
		print_head = 1;
	}

	int r, c;
	for (c = 0; c < N_COL; c++) {
		for (r = 0; r < N_ROW && cc >= 0; r++) {
			if (print_head) { /* heading */
				cairo_move_to (cr, c * col_width + 80.0, 80.0);
				cairo_select_font_face (cr, "Symbol",
					CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size (cr, 40.0);
				cairo_show_text (cr, trump_str[SUIT(cc)]);
				print_head = 0;
				continue;
			}

			cairo_save (cr);
			cairo_translate (cr,
				col_width * c + (col_width - 9 * ARROWSIZE) / 2,
				row_height * (r + 1) - (row_height - 6 * ARROWSIZE) / 2);
			cairo_rotate (cr, -M_PI_2);
			magic_card (cr, win, cc);

			/* debugging: print card */
			cairo_move_to (cr, 6, -2);
			cairo_set_font_size (cr, 10);
			cairo_select_font_face (cr, "Symbol",
				CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_show_text (cr, trump_str[SUIT(cc)]);
			cairo_select_font_face (cr, "Sans",
				CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_show_text (cr, _(rank_string (RANK(cc))));

			cairo_restore (cr);
			
			cc--; /* next card */
			if (RANK(cc) == cardA)
				print_head = 1;
		}
	}
}

static void
begin_print (GtkPrintOperation *operation,
	     GtkPrintContext   *context,
	     gpointer           user_data)
{
	gtk_print_operation_set_n_pages (operation, 1);
}

static void
magic_begin_print (GtkPrintOperation *operation,
	     GtkPrintContext   *context,
	     gpointer           user_data)
{
	gtk_print_operation_set_n_pages (operation, 3);
	gtk_print_operation_set_use_full_page (operation, TRUE);
	gtk_print_operation_set_show_progress (operation, TRUE);
	//gtk_print_operation_set_export_filename (operation, "foo");
}

static GtkPrintSettings *settings = NULL;

void
on_menu_print_activate (void)
{
	GtkPrintOperation *print;
	GtkPrintOperationResult res;
	printf ("printing...\n");

	print = gtk_print_operation_new ();

	if (settings != NULL) 
		gtk_print_operation_set_print_settings (print, settings);

	g_signal_connect (print, "begin_print", G_CALLBACK (begin_print), NULL);
	g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), NULL);

	res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
			GTK_WINDOW (win->window), NULL);

	if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
	{
		if (settings != NULL)
			g_object_unref (settings);
		settings = g_object_ref (gtk_print_operation_get_print_settings (print));
	}

	g_object_unref (print);
}

void
on_menu_file_magic_activate (void)
{
	GtkPrintOperation *print;
	GtkPrintOperationResult res;

	print = gtk_print_operation_new ();

	if (settings != NULL) 
		gtk_print_operation_set_print_settings (print, settings);

	g_signal_connect (print, "begin_print", G_CALLBACK (magic_begin_print), NULL);
	g_signal_connect (print, "draw_page", G_CALLBACK (magic_draw_page), NULL);

	res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
			GTK_WINDOW (win->window), NULL);

	if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
	{
		if (settings != NULL)
			g_object_unref (settings);
		settings = g_object_ref (gtk_print_operation_get_print_settings (print));
	}

	g_object_unref (print);
}

