PLUGIN_NAME = gimp-dynamic-inkscape-c
CC          = gcc

# --- Bundle GIMP sur macOS : chemin vers les fichiers .pc ---
GIMP_BUNDLE_PC = /Applications/GIMP.app/Contents/Resources/lib/pkgconfig

# PKG_CONFIG avec le chemin explicite (résout le bug d'ordre d'évaluation)
PKG_CONFIG = PKG_CONFIG_PATH="$(GIMP_BUNDLE_PC):$$PKG_CONFIG_PATH" pkg-config

LIBS_LIST = gimp-3.0 gimpui-3.0 librsvg-2.0 cairo json-glib-1.0 gegl-0.4

CFLAGS = $(shell $(PKG_CONFIG) --cflags $(LIBS_LIST))
LIBS   = $(shell $(PKG_CONFIG) --libs   $(LIBS_LIST)) -lz -lm

# Sur macOS, on ajoute le RPATH pour que le binaire trouve les libs GIMP
ifeq ($(shell uname), Darwin)
    LDFLAGS += -Wl,-rpath,@loader_path/../../../../../../..
    LDFLAGS += -Wl,-rpath,/Applications/GIMP.app/Contents/Resources
endif

# Cible par défaut : affiche les flags pour vérification
$(info [Makefile] CFLAGS = $(CFLAGS))

SOURCES = main.c metadata.c
OBJECTS = $(SOURCES:.c=.o)

# GIMP 3.2+ : le dossier plug-ins porte le même nom que le binaire
GIMP_PLUGIN_DIR = $(HOME)/.config/GIMP/3.2/plug-ins/$(PLUGIN_NAME)

all: $(PLUGIN_NAME)

$(PLUGIN_NAME): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	mkdir -p "$(GIMP_PLUGIN_DIR)"
	cp $(PLUGIN_NAME) "$(GIMP_PLUGIN_DIR)/"
	@echo "✅ Plugin installé dans $(GIMP_PLUGIN_DIR)"

dist: clean
	@echo "Packaging source for distribution..."
	tar -czvf gimp-smart-capsule-src.tar.gz main.c metadata.c metadata.h Makefile README.md CHANGELOG.md
	@echo "✅ Archive gimp-smart-capsule-src.tar.gz créée."

clean:
	rm -f $(PLUGIN_NAME) $(OBJECTS) gimp-smart-capsule-src.tar.gz

# Cible de diagnostic : affiche toutes les versions des libs
check-deps:
	@echo "=== Versions des dépendances ==="
	@for lib in $(LIBS_LIST); do \
	  version=$$($(PKG_CONFIG) --modversion $$lib 2>/dev/null); \
	  if [ -n "$$version" ]; then \
	    echo "✅ $$lib → $$version"; \
	  else \
	    echo "❌ $$lib → INTROUVABLE"; \
	  fi; \
	done

.PHONY: all clean install check-deps dist
