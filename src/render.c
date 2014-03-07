/*****************************************************\
* RENDER.C
* By: Jesse McClure (c) 2012-2014
* See slider.c or COPYING for license information
\*****************************************************/

#include "slider.h"

static Show *render(const char *, Bool);

Show *render_init(const char *fshow, const char *fnote) {
	Show *show;
	if (conf.interleave && !fnote) {
		show = render(fshow, True);
		show->notes = render(fshow, False);
	}
	else {
		conf.interleave = False;
		show = render(fshow, True);
		if (fnote) show->notes = render(fnote, False);
	}
	return show;
}

int render_free(Show *show) {
	int i;
	if (show->notes) {
		for (i = 0; i < show->notes->nslides; i++)
			cairo_surface_destroy(show->notes->slide[i]);
		free(show->notes->slide);
		free(show->notes->uri);
		free(show->notes);
	}
	for (i = 0; i < show->nslides; i++)
		cairo_surface_destroy(show->slide[i]);
	free(show->slide);
	free(show->uri);
	free(show);
	xlib_free();
	return 0;
}

Show *render(const char *fname, Bool X) {
	char *rpath = realpath(fname, NULL);
	if (!rpath) return NULL;
	Show *show = calloc(1, sizeof(Show));
	show->uri = malloc(strlen(rpath)+8);
	strcpy(show->uri,"file://"); strcat(show->uri,rpath);
	free(rpath);
	PopplerDocument *pdf;
	if (X) {
		xlib_init(show);
	}
	else {
		show->w = conf.view[0].w;
		show->h = conf.view[0].h;
	}
	if ( !(pdf=poppler_document_new_from_file(show->uri, NULL, NULL)) ) {
		fprintf(stderr, "\"%s\' is not a pdf file\n", show->uri);
		return NULL;
	}
	if ( !(show->nslides=poppler_document_get_n_pages(pdf)) ) {
		fprintf(stderr, "No pages found in \"%s\"\n", show->uri);
		return NULL;
	}
	if (conf.interleave) show->nslides /= 2;
	show->slide = calloc(show->nslides, sizeof(cairo_surface_t *));
	PopplerPage *page;
	double pdfw, pdfh;
	int i, p;
	cairo_t *ctx;
	for (i = 0; i < show->nslides; i++) {
		p = (conf.interleave ? 2 * i + (X ? 1 : 0) : i);
		page = poppler_document_get_page(pdf, p);
		poppler_page_get_size(page, &pdfw, &pdfh);
		show->slide[i] = cairo_image_surface_create(0, show->w, show->h);
		ctx = cairo_create(show->slide[i]);
		cairo_scale(ctx, show->w / pdfw, show->h / pdfh);
		cairo_set_source_rgba(ctx, 1, 1, 1, 1);
		cairo_paint(ctx);
		poppler_page_render(page, ctx);
		cairo_destroy(ctx);
	}
	return show;
}

