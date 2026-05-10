# Changelog - GIMP Smart Capsules

## [6.2.1] - 2024-05-10
### Changements
- Renommage de "Smart Object" en "Capsule" pour une terminologie cohérente.
- Mise à jour des attributions : "Assistance IA & User".
- Création de la documentation interne (`README.md`).

## [6.2.0] - 2024-05-10
### Ajout
- **Internationalisation (I18N)** : Support de gettext et macros `_()`.
- **Documentation PDB** : Aide complète, auteurs et copyright intégrés au navigateur de procédures de GIMP.
- **Standard GIMP 3.2** : Architecture conforme aux plugins officiels du "core".

## [6.1.1] - 2024-05-10
### Correction
- **Rafraîchissement des miniatures** : Implémentation d'un cycle de visibilité forcée pour corriger l'affichage des miniatures des calques liés.
- Suppression des appels invalides à `gimp_image_flush`.

## [6.0.0] - 2024-05-09
### Majeur
- Passage au mode **XCF Embarqué** : Les capsules stockent désormais des fichiers XCF compressés en interne au lieu de simples SVG ou PNG.
- Utilisation de **JSON-Glib** pour la persistence des métadonnées.
- Migration vers **GimpLinkLayer** (Classe native GIMP 3.2).

## [5.0.0] - 2024-05-08
- Version initiale en C pur utilisant GEGL pour les transformations.
