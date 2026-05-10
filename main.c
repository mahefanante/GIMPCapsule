#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <glib/gi18n.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "metadata.h"

#define PLUG_IN_BINARY "gimp-dynamic-inkscape"
#define GETTEXT_PACKAGE "gimp-dynamic-inkscape"
#define TEMP_DIR "/tmp/gimp_capsule"

typedef struct _DynamicInkscape DynamicInkscape;
typedef struct _DynamicInkscapeClass DynamicInkscapeClass;

struct _DynamicInkscape { GimpPlugIn parent_instance; };
struct _DynamicInkscapeClass { GimpPlugInClass parent_class; };

G_DEFINE_TYPE(DynamicInkscape, dynamic_inkscape, GIMP_TYPE_PLUG_IN)

#define DYNAMIC_INKSCAPE_TYPE_PLUG_IN (dynamic_inkscape_get_type())

static GList* dynamic_inkscape_query_procedures(GimpPlugIn *plug_in);
static GimpProcedure* dynamic_inkscape_create_procedure(GimpPlugIn *plug_in, const gchar *name);
static gboolean dynamic_inkscape_set_i18n(GimpPlugIn *plug_in, const gchar *procedure_name, gchar **gettext_domain, gchar **catalog_dir);

static GimpValueArray* dynamic_inkscape_run_import(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data);
static GimpValueArray* dynamic_inkscape_run_edit(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data);
static GimpValueArray* dynamic_inkscape_run_sync(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data);
static GimpValueArray* dynamic_inkscape_run_from_selection(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data);
static GimpValueArray* dynamic_inkscape_run_unencapsulate(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data);
static void capsule_log(const gchar *format, ...);

static void dynamic_inkscape_class_init(DynamicInkscapeClass *klass) {
    GimpPlugInClass *plug_in_class = GIMP_PLUG_IN_CLASS(klass);
    plug_in_class->query_procedures = dynamic_inkscape_query_procedures;
    plug_in_class->create_procedure = dynamic_inkscape_create_procedure;
    plug_in_class->set_i18n         = dynamic_inkscape_set_i18n;
}

static void dynamic_inkscape_init(DynamicInkscape *plug_in) {}

static gboolean dynamic_inkscape_set_i18n(GimpPlugIn *plug_in, const gchar *procedure_name, gchar **gettext_domain, gchar **catalog_dir) {
    *gettext_domain = g_strdup(GETTEXT_PACKAGE);
    return TRUE;
}

static GList* dynamic_inkscape_query_procedures(GimpPlugIn *plug_in) {
    GList *list = NULL;
    list = g_list_append(list, g_strdup("capsule-import"));
    list = g_list_append(list, g_strdup("capsule-edit"));
    list = g_list_append(list, g_strdup("capsule-sync"));
    list = g_list_append(list, g_strdup("capsule-from-selection"));
    list = g_list_append(list, g_strdup("capsule-unencapsulate"));
    return list;
}

static GimpProcedure* dynamic_inkscape_create_procedure(GimpPlugIn *plug_in, const gchar *name) {
    GimpProcedure *procedure = NULL;
    const gchar *authors = "Assistance IA & User";
    const gchar *copyright = "Assistance IA & User";
    const gchar *date = "2024";

    if (g_strcmp0(name, "capsule-import") == 0) {
        procedure = gimp_image_procedure_new(plug_in, name, GIMP_PDB_PROC_TYPE_PLUGIN, dynamic_inkscape_run_import, NULL, NULL);
        gimp_procedure_set_menu_label(procedure, _("Importer une Capsule (Native)..."));
        gimp_procedure_set_documentation(procedure, _("Importer un fichier externe (XCF, SVG, etc.) comme Capsule liée."), _("Permet d'importer un fichier et de l'encapsuler comme un calque de lien dynamique."), name);
    } else if (g_strcmp0(name, "capsule-edit") == 0) {
        procedure = gimp_image_procedure_new(plug_in, name, GIMP_PDB_PROC_TYPE_PLUGIN, dynamic_inkscape_run_edit, NULL, NULL);
        gimp_procedure_set_menu_label(procedure, _("Éditer le Contenu de la Capsule..."));
        gimp_procedure_set_documentation(procedure, _("Ouvrir la source de la capsule pour modification."), _("Extrait la source embarquée si nécessaire et l'ouvre dans GIMP ou l'application système par défaut."), name);
    } else if (g_strcmp0(name, "capsule-sync") == 0) {
        procedure = gimp_image_procedure_new(plug_in, name, GIMP_PDB_PROC_TYPE_PLUGIN, dynamic_inkscape_run_sync, NULL, NULL);
        gimp_procedure_set_menu_label(procedure, _("Synchroniser vers XCF (Embarquer)..."));
        gimp_procedure_set_documentation(procedure, _("Mettre à jour la source embarquée depuis le fichier lié."), _("Lit le fichier sur le disque et met à jour le parasite interne du calque pour assurer la persistence."), name);
    } else if (g_strcmp0(name, "capsule-from-selection") == 0) {
        procedure = gimp_image_procedure_new(plug_in, name, GIMP_PDB_PROC_TYPE_PLUGIN, dynamic_inkscape_run_from_selection, NULL, NULL);
        gimp_procedure_set_menu_label(procedure, _("Créer une Capsule depuis la sélection..."));
        gimp_procedure_set_documentation(procedure, _("Convertir les calques sélectionnés en une nouvelle Capsule."), _("Groupe les calques sélectionnés, les exporte dans un XCF interne et les remplace par un calque de lien."), name);
    } else if (g_strcmp0(name, "capsule-unencapsulate") == 0) {
        procedure = gimp_image_procedure_new(plug_in, name, GIMP_PDB_PROC_TYPE_PLUGIN, dynamic_inkscape_run_unencapsulate, NULL, NULL);
        gimp_procedure_set_menu_label(procedure, _("Dissoudre la Capsule (Désencapsuler)"));
        gimp_procedure_set_documentation(procedure, _("Extraire les calques de la capsule vers l'image parente."), _("Dissout la capsule et replace ses calques internes dans le projet actuel tout en préservant les offsets."), name);
    }

    if (procedure) {
        gimp_procedure_set_image_types(procedure, "*");
        gimp_procedure_set_sensitivity_mask(procedure, GIMP_PROCEDURE_SENSITIVE_DRAWABLE | GIMP_PROCEDURE_SENSITIVE_DRAWABLES);
        gimp_procedure_add_menu_path(procedure, "<Image>/Filters/Capsules");
        gimp_procedure_set_attribution(procedure, authors, copyright, date);
    }
    return procedure;
}

/* HELPER: Ensure temp directory exists */
static void ensure_temp_dir() {
    mkdir(TEMP_DIR, 0777);
}

/* IMPORT */
static GimpValueArray* dynamic_inkscape_run_import(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data) {
    gimp_ui_init(PLUG_IN_BINARY);
    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Importer Capsule Native"), NULL, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Annuler"), GTK_RESPONSE_CANCEL, _("_Ouvrir"), GTK_RESPONSE_OK, NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        GFile *file = g_file_new_for_path(filename);
        
        /* Create Native Link Layer */
        GimpLinkLayer *layer = gimp_link_layer_new(image, file);
        if (layer) {
            gimp_image_undo_group_start(image);
            gimp_image_insert_layer(image, GIMP_LAYER(layer), NULL, -1);
            
            /* SCALING & CENTERING LOGIC */
            gint img_w = gimp_image_get_width(image);
            gint img_h = gimp_image_get_height(image);
            gint lw = gimp_drawable_get_width(GIMP_DRAWABLE(layer));
            gint lh = gimp_drawable_get_height(GIMP_DRAWABLE(layer));
            
            if (lw > img_w || lh > img_h) {
                gdouble scale_w = (gdouble)img_w * 0.8 / lw;
                gdouble scale_h = (gdouble)img_h * 0.8 / lh;
                gdouble scale = MIN(scale_w, scale_h);
                gint new_w = round(lw * scale);
                gint new_h = round(lh * scale);
                
                /* Center before scaling? No, scale then center is easier */
                gimp_item_transform_scale(GIMP_ITEM(layer), 0, 0, new_w, new_h);
                lw = new_w; lh = new_h;
                capsule_log("Scaled capsule down to %dx%d", new_w, new_h);
            }
            
            /* Center the capsule */
            gimp_layer_set_offsets(GIMP_LAYER(layer), (img_w - lw) / 2, (img_h - lh) / 2);
            
            /* Force thumbnail refresh via update and visibility toggle */
            gimp_drawable_update(GIMP_DRAWABLE(layer), 0, 0, lw, lh);
            gimp_item_set_visible(GIMP_ITEM(layer), FALSE);
            gimp_item_set_visible(GIMP_ITEM(layer), TRUE);
            gimp_displays_flush();

            /* Store metadata and embed content */
            gchar *data = NULL; gsize length;
            if (g_file_get_contents(filename, &data, &length, NULL)) {
                gchar *ext = strrchr(filename, '.');
                VectorMeta meta = {0};
                meta.format = ext ? g_ascii_strdown(ext + 1, -1) : g_strdup("svg");
                meta.origin_path = g_strdup(filename);
                meta.base_w = lw; meta.base_h = lh;
                
                metadata_store(GIMP_LAYER(layer), data, length, &meta);
                
                if (!ext) g_free(meta.format);
                g_free(meta.origin_path); g_free(data);
            }
            gimp_image_undo_group_end(image);
            g_message(_("Capsule importée et adaptée au canevas."));
        } else {
            g_message(_("Erreur : Impossible de créer le calque de lien."));
        }
        
        g_object_unref(file); g_free(filename);
    }
    gtk_widget_destroy(dialog);
    return gimp_procedure_new_return_values(procedure, GIMP_PDB_SUCCESS, NULL);
}

/* EDIT */
static GimpValueArray* dynamic_inkscape_run_edit(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data) {
    if (!drawables || !drawables[0] || !GIMP_IS_LINK_LAYER(drawables[0])) {
        g_message(_("Veuillez sélectionner une Capsule (Calque de lien)."));
        return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    }
    
    GimpLayer *layer = GIMP_LAYER(drawables[0]);
    VectorMeta *meta = metadata_get_meta(layer);
    if (!meta) return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    
    gchar *target_path = g_strdup(meta->origin_path);
    gboolean is_temp = FALSE;

    /* Check if original file exists, if not extract to temp */
    if (access(target_path, F_OK) != 0) {
        ensure_temp_dir();
        gsize len;
        gchar *data = metadata_get_source(layer, &len);
        if (data) {
            g_free(target_path);
            target_path = g_strdup_printf("%s/capsule_%d.%s", TEMP_DIR, gimp_item_get_id(GIMP_ITEM(layer)), meta->format);
            g_file_set_contents(target_path, data, len, NULL);
            g_free(data);
            is_temp = TRUE;
            
            /* Re-link to temp file so GIMP can track it */
            GFile *temp_file = g_file_new_for_path(target_path);
            gimp_link_layer_set_file(GIMP_LINK_LAYER(layer), temp_file);
            g_object_unref(temp_file);
        }
    }

    /* Open */
    if (g_ascii_strcasecmp(meta->format, "xcf") == 0) {
        GFile *f = g_file_new_for_path(target_path);
        GimpImage *new_img = gimp_file_load(GIMP_RUN_NONINTERACTIVE, f);
        if (new_img) gimp_display_new(new_img);
        g_object_unref(f);
    } else {
        gchar *cmd = g_strdup_printf("xdg-open \"%s\"", target_path);
        g_spawn_command_line_async(cmd, NULL);
        g_free(cmd);
    }

    g_free(target_path); vector_meta_free(meta);
    return gimp_procedure_new_return_values(procedure, GIMP_PDB_SUCCESS, NULL);
}

/* SYNC BACK TO XCF */
static GimpValueArray* dynamic_inkscape_run_sync(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data) {
    if (!drawables || !drawables[0] || !GIMP_IS_LINK_LAYER(drawables[0])) return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    
    GimpLinkLayer *link_layer = GIMP_LINK_LAYER(drawables[0]);
    GFile *file = gimp_link_layer_get_file(link_layer);
    if (!file) return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    
    gchar *path = g_file_get_path(file);
    gchar *data; gsize len;
    if (g_file_get_contents(path, &data, &len, NULL)) {
        VectorMeta *meta = metadata_get_meta(GIMP_LAYER(link_layer));
        if (meta) {
            metadata_store(GIMP_LAYER(link_layer), data, len, meta);
            vector_meta_free(meta);
            g_message(_("Capsule synchronisée et embarquée dans le XCF."));
        }
        g_free(data);
    }
    g_free(path);
    return gimp_procedure_new_return_values(procedure, GIMP_PDB_SUCCESS, NULL);
}

/* LOGGING HELPER */
static void capsule_log(const gchar *format, ...) {
    g_mkdir_with_parents(TEMP_DIR, 0777);
    va_list args;
    va_start(args, format);
    gchar *msg = g_strdup_vprintf(format, args);
    va_end(args);

    /* Log to file */
    FILE *f = fopen(TEMP_DIR "/log.txt", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
    /* Log to GIMP console (stderr) */
    g_printerr("SmartCapsule: %s\n", msg);
    g_free(msg);
}

/* FROM SELECTION - V1-SMOTHER ADAPTED APPROACH */
static GimpValueArray* dynamic_inkscape_run_from_selection(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data) {
    capsule_log("--- Start Capsule From Selection (V1-Smother Approach) ---");
    
    if (!drawables || !drawables[0]) {
        g_message(_("Veuillez sélectionner au moins un calque."));
        return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    }

    gimp_image_undo_group_start(image);

    /* 1. Calculate bounding box */
    gint min_x = G_MAXINT, min_y = G_MAXINT;
    gint max_x = G_MININT, max_y = G_MININT;
    for (int i = 0; drawables[i]; i++) {
        gint x, y, w, h;
        gimp_drawable_get_offsets(drawables[i], &x, &y);
        w = gimp_drawable_get_width(drawables[i]);
        h = gimp_drawable_get_height(drawables[i]);
        min_x = MIN(min_x, x); min_y = MIN(min_y, y);
        max_x = MAX(max_x, x + w); max_y = MAX(max_y, y + h);
    }
    gint total_w = max_x - min_x;
    gint total_h = max_y - min_y;

    /* 2. Create NEW temporary image (like v1-smother) */
    GimpImage *temp_img = gimp_image_new_with_precision(total_w, total_h, 
                                                       gimp_image_get_base_type(image),
                                                       gimp_image_get_precision(image));
    gdouble xres, yres;
    gimp_image_get_resolution(image, &xres, &yres);
    gimp_image_set_resolution(temp_img, xres, yres);
    
    /* 3. Copy layers (preserving order) */
    /* We iterate in reverse to maintain the stack order in the new image */
    int count = 0; while (drawables[count]) count++;
    for (int i = count - 1; i >= 0; i--) {
        if (gimp_item_is_layer(GIMP_ITEM(drawables[i]))) {
            GimpLayer *new_layer = gimp_layer_new_from_drawable(drawables[i], temp_img);
            if (new_layer) {
                gimp_image_insert_layer(temp_img, new_layer, NULL, -1);
                gint ox, oy;
                gimp_drawable_get_offsets(drawables[i], &ox, &oy);
                gimp_layer_set_offsets(new_layer, ox - min_x, oy - min_y);
            }
        }
    }

    /* 4. Save temp XCF */
    ensure_temp_dir();
    gchar *temp_xcf = g_strdup_printf("%s/selection_%u.xcf", TEMP_DIR, g_random_int());
    GFile *temp_file = g_file_new_for_path(temp_xcf);
    
    if (gimp_file_save(GIMP_RUN_NONINTERACTIVE, temp_img, temp_file, NULL)) {
        /* 5. Create Link Layer in original image */
        GimpLinkLayer *link_layer = gimp_link_layer_new(image, temp_file);
        if (link_layer) {
            gimp_image_insert_layer(image, GIMP_LAYER(link_layer), NULL, -1);
            gimp_layer_set_offsets(GIMP_LAYER(link_layer), min_x, min_y);
            
            /* Force thumbnail refresh */
            gimp_drawable_update(GIMP_DRAWABLE(link_layer), 0, 0, total_w, total_h);
            gimp_item_set_visible(GIMP_ITEM(link_layer), FALSE);
            gimp_item_set_visible(GIMP_ITEM(link_layer), TRUE);
            gimp_displays_flush();

            /* Embed data */
            gchar *data = NULL; gsize length;
            if (g_file_get_contents(temp_xcf, &data, &length, NULL)) {
                VectorMeta meta = {0};
                meta.format = g_strdup("xcf");
                meta.origin_path = g_strdup(temp_xcf);
                meta.base_w = total_w; meta.base_h = total_h;
                metadata_store(GIMP_LAYER(link_layer), data, length, &meta);
                g_free(meta.format); g_free(meta.origin_path); g_free(data);
            }
            
            /* 6. Remove original layers */
            for (int i = 0; drawables[i]; i++) {
                gimp_image_remove_layer(image, GIMP_LAYER(drawables[i]));
            }
            g_message(_("Capsule créée depuis la sélection."));
        } else {
            g_message(_("Erreur : Impossible de sauvegarder le XCF temporaire."));
        }
    }

    g_object_unref(temp_file); g_free(temp_xcf);
    gimp_image_delete(temp_img);
    gimp_image_undo_group_end(image);
    gimp_displays_flush();

    capsule_log("--- End Capsule From Selection ---");
    return gimp_procedure_new_return_values(procedure, GIMP_PDB_SUCCESS, NULL);
}

/* UNENCAPSULATE - Dissolve capsule back to layers */
static GimpValueArray* dynamic_inkscape_run_unencapsulate(GimpProcedure *procedure, GimpRunMode run_mode, GimpImage *image, GimpDrawable **drawables, GimpProcedureConfig *config, gpointer run_data) {
    if (!drawables || !drawables[0]) return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    GimpLayer *layer = GIMP_LAYER(drawables[0]);

    VectorMeta *meta = metadata_get_meta(layer);
    if (!meta || g_strcmp0(meta->format, "xcf") != 0) {
        g_message("Ce calque n'est pas une capsule XCF valide.");
        if (meta) vector_meta_free(meta);
        return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    }

    gsize data_len;
    gchar *data = metadata_get_source(layer, &data_len);
    if (!data) {
        g_message("Impossible d'extraire la source de la capsule.");
        vector_meta_free(meta);
        return gimp_procedure_new_return_values(procedure, GIMP_PDB_EXECUTION_ERROR, NULL);
    }

    gimp_image_undo_group_start(image);

    /* 1. Save to temp file to load it back */
    ensure_temp_dir();
    gchar *tmp_path = g_build_filename(TEMP_DIR, "unencap_temp.xcf", NULL);
    g_file_set_contents(tmp_path, data, data_len, NULL);
    GFile *tmp_file = g_file_new_for_path(tmp_path);

    /* 2. Load the XCF */
    GimpImage *src_img = gimp_file_load(GIMP_RUN_NONINTERACTIVE, tmp_file);
    if (src_img) {
        gint ox, oy;
        gimp_drawable_get_offsets(GIMP_DRAWABLE(layer), &ox, &oy);
        
        GimpLayer **src_layers = gimp_image_get_layers(src_img);
        /* Insert layers in reverse order to keep stack */
        int count = 0; while (src_layers[count]) count++;
        
        for (int i = count - 1; i >= 0; i--) {
            GimpLayer *new_l = gimp_layer_new_from_drawable(GIMP_DRAWABLE(src_layers[i]), image);
            gimp_image_insert_layer(image, new_l, GIMP_LAYER(gimp_item_get_parent(GIMP_ITEM(layer))), gimp_image_get_item_position(image, GIMP_ITEM(layer)));
            
            gint lox, loy;
            gimp_drawable_get_offsets(GIMP_DRAWABLE(src_layers[i]), &lox, &loy);
            gimp_layer_set_offsets(new_l, ox + lox, oy + loy);
        }
        g_free(src_layers);
        gimp_image_delete(src_img);
        
        /* 3. Remove the capsule */
        gimp_image_remove_layer(image, layer);
        g_message(_("Capsule dissoute avec succès."));
    }

    g_object_unref(tmp_file);
    g_free(tmp_path);
    g_free(data);
    vector_meta_free(meta);
    
    gimp_image_undo_group_end(image);
    gimp_displays_flush();

    return gimp_procedure_new_return_values(procedure, GIMP_PDB_SUCCESS, NULL);
}

GIMP_MAIN(DYNAMIC_INKSCAPE_TYPE_PLUG_IN)
