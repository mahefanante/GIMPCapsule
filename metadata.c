#include "metadata.h"
#include <json-glib/json-glib.h>
#include <string.h>
#include <zlib.h>

void metadata_store(GimpLayer *layer, const gchar *data, gsize data_len, VectorMeta *meta) {
    if (data && data_len > 0) {
        uLongf compressed_size = compressBound(data_len);
        Bytef *compressed_data = g_malloc(compressed_size);
        
        int res = compress(compressed_data, &compressed_size, (const Bytef*)data, data_len);
        if (res == Z_OK) {
            GimpParasite *p_source = gimp_parasite_new(PARASITE_SVG_SOURCE, GIMP_PARASITE_PERSISTENT, compressed_size, compressed_data);
            gimp_item_attach_parasite(GIMP_ITEM(layer), p_source);
            gimp_parasite_free(p_source);
        }
        g_free(compressed_data);
    }

    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "base_w"); json_builder_add_double_value(builder, meta->base_w);
    json_builder_set_member_name(builder, "base_h"); json_builder_add_double_value(builder, meta->base_h);
    json_builder_set_member_name(builder, "format"); json_builder_add_string_value(builder, meta->format ? meta->format : "bin");
    json_builder_set_member_name(builder, "origin_path"); json_builder_add_string_value(builder, meta->origin_path ? meta->origin_path : "");
    json_builder_set_member_name(builder, "hash"); json_builder_add_string_value(builder, meta->hash ? meta->hash : "");
    json_builder_set_member_name(builder, "guide_name"); json_builder_add_string_value(builder, meta->guide_name ? meta->guide_name : "");
    json_builder_set_member_name(builder, "source_len"); json_builder_add_int_value(builder, data_len);
    json_builder_end_object(builder);

    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, json_builder_get_root(builder));
    gchar *json_data = json_generator_to_data(gen, NULL);

    GimpParasite *p_meta = gimp_parasite_new(PARASITE_SVG_META, GIMP_PARASITE_PERSISTENT, strlen(json_data), json_data);
    gimp_item_attach_parasite(GIMP_ITEM(layer), p_meta);
    
    g_free(json_data);
    g_object_unref(gen);
    g_object_unref(builder);
    gimp_parasite_free(p_meta);
}

gchar* metadata_get_source(GimpLayer *layer, gsize *out_len) {
    const GimpParasite *p = gimp_item_get_parasite(GIMP_ITEM(layer), PARASITE_SVG_SOURCE);
    if (!p) return NULL;

    guint32 comp_size;
    const Bytef *comp_data = (const Bytef *)gimp_parasite_get_data(p, &comp_size);
    uLongf dest_len = comp_size * 20 + 65536; /* Reasonable buffer for decompression */
    gchar *dest = g_malloc(dest_len);

    int res = uncompress((Bytef*)dest, &dest_len, comp_data, comp_size);
    if (res == Z_OK) {
        *out_len = dest_len;
        return dest;
    }
    
    g_free(dest);
    return NULL;
}

VectorMeta* metadata_get_meta(GimpLayer *layer) {
    const GimpParasite *p = gimp_item_get_parasite(GIMP_ITEM(layer), PARASITE_SVG_META);
    if (!p) return NULL;

    JsonParser *parser = json_parser_new();
    guint32 meta_size;
    const gchar *meta_raw = (const gchar *)gimp_parasite_get_data(p, &meta_size);
    if (!json_parser_load_from_data(parser, meta_raw, meta_size, NULL)) {
        g_object_unref(parser);
        return NULL;
    }

    JsonObject *obj = json_node_get_object(json_parser_get_root(parser));
    VectorMeta *meta = g_new0(VectorMeta, 1);

    meta->base_w = json_object_get_double_member(obj, "base_w");
    meta->base_h = json_object_get_double_member(obj, "base_h");
    meta->format = g_strdup(json_object_get_string_member(obj, "format"));
    meta->origin_path = g_strdup(json_object_get_string_member(obj, "origin_path"));
    
    if (json_object_has_member(obj, "hash"))
        meta->hash = g_strdup(json_object_get_string_member(obj, "hash"));
    if (json_object_has_member(obj, "guide_name"))
        meta->guide_name = g_strdup(json_object_get_string_member(obj, "guide_name"));

    g_object_unref(parser);
    return meta;
}

void vector_meta_free(VectorMeta *meta) {
    if (!meta) return;
    g_free(meta->format);
    g_free(meta->origin_path);
    g_free(meta->hash);
    g_free(meta->guide_name);
    g_free(meta);
}
