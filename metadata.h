#ifndef __METADATA_H__
#define __METADATA_H__

#include <libgimp/gimp.h>

/* Persistent Metadata for the Smart Capsule */
#define PARASITE_SVG_META   "capsule-meta-v5"
#define PARASITE_SVG_SOURCE "capsule-source-v5"

typedef struct {
    gdouble base_w;
    gdouble base_h;
    gchar  *format;      /* "svg", "xcf", "bin" */
    gchar  *origin_path;
    gchar  *hash;
    gchar  *guide_name;  /* For inkscape layers */
} VectorMeta;

void        metadata_store      (GimpLayer *layer, const gchar *data, gsize data_len, VectorMeta *meta);
gchar*      metadata_get_source (GimpLayer *layer, gsize *out_len);
VectorMeta* metadata_get_meta   (GimpLayer *layer);
void        vector_meta_free    (VectorMeta *meta);

#endif
