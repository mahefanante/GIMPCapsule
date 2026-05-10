# GIMPCapsule
Capsule is similar to photoshop smart object
=======
# GIMP Smart Capsules

Le plugin **Smart Capsules** permet de transformer vos calques GIMP en objets dynamiques ("Capsules") dont le contenu source est préservé, éditable et synchronisé directement dans votre fichier XCF.

## Fonctionnalités
- **Importation Native** : Importer un fichier XCF ou SVG comme un calque de lien dynamique.
- **Édition Directe** : Ouvrir la source d'une capsule pour la modifier, avec mise à jour automatique dans GIMP.
- **Persistence Totale** : Le contenu de la capsule est compressé et embarqué dans les métadonnées (parasites) du calque XCF parent.
- **Encapsulation de Sélection** : Transformer un groupe de calques en une capsule unique.
- **Dissolution (Désencapsulation)** : Récupérer les calques originaux d'une capsule dans le projet principal.

## Emplacement dans GIMP
Retrouvez les commandes sous le menu : **Filtres > Capsules**.

---

## Documentation Technique

### Architecture
Le plugin est écrit en C et utilise l'API **GIMP 3.2**. Il repose sur deux piliers :
1. **GimpLinkLayer** : Utilise la nouvelle classe de calque de lien de GIMP pour l'affichage dynamique.
2. **Parasites Persistants** : Les données sources (XCF compressé) sont stockées dans le parasite `gimp-capsule-svg-source`. Les métadonnées de transformation sont stockées dans `gimp-capsule-svg-meta` au format JSON.

### Composants
- `main.c` : Gestion du cycle de vie du plugin et des procédures PDB.
- `metadata.c` : Logique de stockage, compression (Zlib) et gestion JSON.
- `metadata.h` : Définition des structures et constantes.

### Compilation et Installation

#### Dépendances requises
Pour compiler ce plugin, vous devez avoir installé les outils de développement suivants :
- **Outils de build** : `gcc`, `make`, `pkg-config`.
- **SDK GIMP 3** : `libgimp-3.0-dev`, `libgtk-3-dev`.
- **Bibliothèques tierces** : `libgegl-dev`, `libjson-glib-dev`, `zlib1g-dev`.

#### Commandes
```bash
make
make install
```
Le binaire est installé dans votre dossier utilisateur GIMP : `~/.config/GIMP/3.2/plug-ins/`.
Pour distribuer le plugin, utilisez `make dist` pour générer une archive source.

### Crédits
- **Développement** : Assistance IA & User
- **Version** : 6.2.1
- **Standard** : GIMP 3.2 Production Grade
>>>>>>> a7a0b4a (Initialisation du projet et ajout du code source)
