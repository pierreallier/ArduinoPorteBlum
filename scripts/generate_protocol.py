from pathlib import Path
import re
import sys

_first_log = True
def log(message, file=sys.stdout):
    global _first_log
    prefix = "[generate_protocol] " if _first_log else " - "
    print(f"{prefix}{message}", file=file, flush=True)
    _first_log = False

# ============================================================
# Configuration
# ============================================================
try:
    Import("env")
    ROOT = Path(env["PROJECT_DIR"])
except NameError:
    ROOT = Path(__file__).resolve().parent.parent    

CSV_FILE = ROOT / "protocol.csv"

CPP_OUTPUT = ROOT / "src" / "Config" / "Codes.h"
PY_OUTPUT = ROOT / "python" / "codes.py"

# ============================================================
# Vérification des dates entre deux fichiers
# Renvoi vrai si le fichier1 est plus récent que le fichier2
# ============================================================
def doit_regenerer():
    """Vrai si une sortie est absente ou plus ancienne que le CSV."""
    csv_time = CSV_FILE.stat().st_mtime
    for out in (CPP_OUTPUT, PY_OUTPUT):
        if not out.exists() or out.stat().st_mtime < csv_time:
            return True
    return False

# ============================================================
# Lecture du fichier
# Format :
# NOM;CODE;"TEXTE" # commentaire
# ============================================================
LINE_PATTERN = re.compile(
    r'^\s*([^;#\s][^;]*)\s*;\s*(\d+)(?:\s*;\s*"((?:[^"]|"")*)")?\s*(?:#.*)?$'
)


def read_codes():
    codes = []
    with CSV_FILE.open("r", encoding="utf-8") as f:
        for line_number, line in enumerate(f, start=1):
            line = line.strip()
            # Ligne vide ou commentaire
            if not line or line.startswith("#"):
                continue
            match = LINE_PATTERN.match(line)
            if not match:
                raise ValueError(
                    f"Ligne {line_number} : format invalide\n"
                    f"  {line}\n"
                    f"Format attendu : NOM;CODE;\"TEXTE\""
                )

            name = match.group(1).strip()
            code = int(match.group(2))
            text = match.group(3)
            if text is not None:
                text = text.replace('""', '"')
            else:
                text = "None"

            # Vérification du nom
            if not name:
                raise ValueError(f"Ligne {line_number} : nom vide")

            # Vérification du code
            if not 0 <= code <= 254:
                raise ValueError(f"Ligne {line_number} : "f"code {code} hors plage (0..254)")

            codes.append({
                "name": name,
                "code": code,
                "text": text,
                "line": line_number,
            })

    return codes


# ============================================================
# Vérifications
# ============================================================
def check_codes(messages):
    names = {}
    codes = {}
    for msg in messages:
        name = msg["name"]
        code = msg["code"]
        line = msg["line"]
        # Nom unique
        if name in names:
            raise ValueError( f"Nom dupliqué : {name!r} " f"(lignes {names[name]} et {line})")
        names[name] = line
        # Code unique
        if code in codes:
            raise ValueError(f"Code dupliqué : {code} "f"(lignes {codes[code]} et {line})")
        codes[code] = line


# ============================================================
# Génération du fichier Codes.h
# ============================================================
def generate_cpp(messages):
    lines = [
        "#ifndef CODES_H",
        "#define CODES_H",
        "",
        "#include <stdint.h>",
        "#include \"Constantes.h\"",
        "",
        "/*",
        " * Fichier généré automatiquement.",
        " * Ne pas modifier manuellement.",
        " *",
        " * Source : protocol.csv",
        " */",
        "",
        "enum class MSG : uint8_t {",
    ]

    # Le CSV peut être dans n'importe quel ordre. On trie uniquement le fichier généré.
    sorted_messages = sorted(messages,key=lambda msg: msg["code"])
    current_section = None
    for msg in sorted_messages:
        code = msg["code"]
        lines.append(f'    {msg["name"]} = {code},    // "{msg["text"]}"')

    lines.extend([
        "};",
        "",
        "#if VERSION_DEV",
        "",
        "inline const __FlashStringHelper* msgName(MSG msg)",
        "{",
        "    switch (msg) {"
    ])

    for msg in sorted_messages:
        lines.append(
            f'        case MSG::{msg["name"]}: '
            f'return F("{msg["name"]}");'
        )

    lines.extend([
        "",
        "        default:",
        '            return F("UNKNOWN");',
        "    }",
        "}",
        "",
        "#endif",
        "",
        "#endif",
        ""
    ])
    CPP_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    CPP_OUTPUT.write_text("\n".join(lines),encoding="utf-8")


# ============================================================
# Génération du fichier Python
# ============================================================
def generate_python(messages):
    lines = [
        '"""',
        "Fichier généré automatiquement.",
        "Ne pas modifier manuellement.",
        "",
        "Source : protocol.csv",
        '"""',
        "",
        "from enum import IntEnum",
        "",
        "",
        "class MSG(IntEnum):",
    ]

    sorted_messages = sorted( messages, key=lambda msg: msg["code"])

    for msg in sorted_messages:
        lines.append( f'    {msg["name"]} = {msg["code"]}')

    lines.extend(["","","MESSAGE_TEXT = {"])

    for msg in sorted_messages:
        lines.append(f'    MSG.{msg["name"]}: {msg["text"]!r},')

    lines.extend(["}",""])
    PY_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    PY_OUTPUT.write_text("\n".join(lines),encoding="utf-8")


# ============================================================
# Programme principal
# ============================================================
def generer_fichiers():
    log(f"Génération des codes du protocole de communication depuis : {CSV_FILE}")
    try:
        messages = read_codes()
        if not messages:
            raise ValueError("Aucun code trouvé dans le fichier CSV")

        check_codes(messages)
        generate_cpp(messages)
        generate_python(messages)

    except ValueError as error:
        log(f"\nERREUR : {error}", file=sys.stderr)
        sys.exit(1)

    log(f"{len(messages)} messages générés.")
    log(f"  C++    : {CPP_OUTPUT}")
    log(f"  Python : {PY_OUTPUT}")


def main():
    if doit_regenerer():
        generer_fichiers()
    else:
        log("Fichiers déjà à jour : aucune modification.")


if "env" in globals() or __name__ == "__main__":
    main()