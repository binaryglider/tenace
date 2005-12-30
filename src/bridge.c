#include <assert.h>
#include <stdlib.h>
#include "bridge.h"

hand *hand_new(void)
{
	hand *h = malloc(sizeof(hand));
	assert(h);
	h->name = g_string_new("New Hand");
	bzero(h->cards, sizeof(card) * 14);
	return h;
}

void hand_free(hand *h)
{
	assert(h);
	g_string_free(h->name, TRUE);
	free(h);
}

board *board_new(void)
{
	int i;
	board *b = malloc(sizeof(board));
	assert(b);
	b->name = g_string_new("New Board");
	for (i = 0; i < 4; i++) {
		b->hands[i] = hand_new();
	}
	bzero(b->cards, sizeof(seat) * 52);
	return b;
}

void board_free(board *b)
{
	int i;
	assert(b);
	g_string_free(b->name, TRUE);
	for (i = 0; i < 4; i++) {
		free(b->hands[i]);
	}
}

gchar *rank_string (rank r)
{
	static gchar *label[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
	if (r == -1)
		return "x";
	return label[r];
}

GString *card_string (card c)
{
	gchar *suit = NULL;
	gchar *rank = NULL;
	static GString *s = NULL;
	if (!s)
		s = g_string_new(NULL);

	switch (SUIT(c)) {
		case spade: suit = "♠"; break;
		case heart: suit = "♥"; break;
		case diamond: suit = "♦"; break;
		case club: suit = "♣"; break;
	}
	g_string_printf(s, "%s%s", suit, rank_string(RANK(c)));
	return s;
}

GString *hand_string (hand *h)
{
	static GString *s = NULL;
	if (!s)
		s = g_string_new(NULL);
	assert(h);
	card *c = h->cards;
	g_string_printf(s, "♠");
	while (*c && SUIT(*c) == spade)
		g_string_append_printf(s, "%s", rank_string(RANK(*c++)));
	g_string_append_printf(s, "\n♥");
	while (*c && SUIT(*c) == heart)
		g_string_append_printf(s, "%s", rank_string(RANK(*c++)));
	g_string_append_printf(s, "\n♦");
	while (*c && SUIT(*c) == diamond)
		g_string_append_printf(s, "%s", rank_string(RANK(*c++)));
	g_string_append_printf(s, "\n♣");
	while (*c && SUIT(*c) == club)
		g_string_append_printf(s, "%s", rank_string(RANK(*c++)));
	return s;
}

void remove_card(hand *h, card c)
{
	card *p = h->cards;
	while (*p && *p != c)
		p++;
	while (*p)
		*p++ = *(p + 1);
}

void add_card(hand *h, card c)
{
	card *p = h->cards;
	while (*p)
		p++;
	*p = c;
	*(p+1) = 0;
	do {
		if (*p < *(p+1)) {
			card tmp = *(p+1);
			*(p+1) = *p;
			*p = tmp;
		}
		if (p == h->cards)
			break;
		p--;
	} while (1);
}

int give_card(board *b, seat s, card c)
{
	int i = CARD_I(SUIT(c), RANK(c));
	assert (i >= 0 && i < 52);
	seat cs = b->cards[i];
	assert (cs >= 0 && cs <= 4);
	if (cs == s) {
		remove_card(b->hands[s-1], c);
		b->cards[i] = 0;
		return 0;
	}
	if (b->hands[s-1]->cards[12])
		return cs != 0;
	if (cs)
		remove_card(b->hands[cs-1], c);
	add_card(b->hands[s-1], c);
	b->cards[i] = s;
	return 1;
}
