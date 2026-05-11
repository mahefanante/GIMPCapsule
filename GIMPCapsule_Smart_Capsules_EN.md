# GIMPCapsule

**Capsule is similar to a Photoshop Smart Object**

# GIMP Smart Capsules

The **Smart Capsules** plugin allows you to transform your GIMP layers into dynamic objects called **Capsules**, where the source content is preserved, editable, and synchronized directly inside your XCF file.

## Features

- **Native Import:** Import an XCF or SVG file as a dynamically linked layer.
- **Direct Editing:** Open the source of a Capsule, edit it, and automatically update it inside GIMP.
- **Full Persistence:** The Capsule content is compressed and embedded inside the metadata, also known as parasites, of the parent XCF layer.
- **Selection Encapsulation:** Convert a group of layers into a single Capsule.
- **Dissolution / Decapsulation:** Recover the original layers from a Capsule back into the main project.

## Location in GIMP

You can find the commands under:

```text
Filters > Capsules
```

# Technical Documentation

## Architecture

The plugin is written in **C** and uses the **GIMP 3.2 API**. It is based on two main pillars:

- **GimpLinkLayer:** Uses GIMP’s new linked-layer class for dynamic display.
- **Persistent Parasites:** The source data, compressed XCF, is stored in the `gimp-capsule-svg-source` parasite. Transformation metadata is stored in `gimp-capsule-svg-meta` in JSON format.

## Components

- **main.c:** Handles the plugin lifecycle and PDB procedures.
- **metadata.c:** Handles storage logic, Zlib compression, and JSON management.
- **metadata.h:** Defines structures and constants.

# Compilation and Installation

## Required Dependencies

To compile this plugin, you must have the following development tools installed:

- **Build tools:** `gcc`, `make`, `pkg-config`
- **GIMP 3 SDK:** `libgimp-3.0-dev`, `libgtk-3-dev`
- **Third-party libraries:** `libgegl-dev`, `libjson-glib-dev`, `zlib1g-dev`

## Commands

```bash
make
make install
```

The binary is installed in your GIMP user folder:

```bash
~/.config/GIMP/3.2/plug-ins/
```

To distribute the plugin, use:

```bash
make dist
```

This generates a source archive.

# Credits

- **Development:** AI Assistance & User
- **Version:** 6.2.1
- **Standard:** GIMP 3.2 Production Grade
