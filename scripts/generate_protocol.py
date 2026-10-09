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
    if "Import" in globals():
        globals()["Import"]("env")
    pio_env = globals().get("env")
    if pio_env is not None:
        ROOT = Path(pio_env["PROJECT_DIR"])
    else:
        raise RuntimeError("Environnement PlatformIO non disponible")
except Exception:
    ROOT = Path(__file__).resolve().parent.parent

CSV_FILE = ROOT / "protocol.csv"
ETATS_CSV_FILE = ROOT / "etats.csv"

CPP_OUTPUT = ROOT / "src" / "Config" / "Codes.h"
ETATS_OUTPUT = ROOT / "src" / "Config" / "Etats.h"
PY_OUTPUT = ROOT / "python" / "codes.py"


# ============================================================
# Vérification des dates entre entrées et sorties
# ============================================================
def doit_regenerer():
    """Vrai si une sortie est absente ou plus ancienne qu'une entrée."""
    inputs = (CSV_FILE, ETATS_CSV_FILE)
    latest_input_time = max(path.stat().st_mtime for path in inputs)

    for out in (CPP_OUTPUT, ETATS_OUTPUT, PY_OUTPUT):
        if not out.exists() or out.stat().st_mtime < latest_input_time:
            return True
    return False


# ============================================================
# Lecture du fichier protocol.csv
# Format : NOM;CODE;"TEXTE" # commentaire
# ============================================================
LINE_PATTERN = re.compile(
    r'^\s*([^;#\s][^;]*)\s*;\s*(\d+)(?:\s*;\s*"((?:[^"]|"")*)")?\s*(?:#.*)?$'
)


def read_codes():
    codes = []
    with CSV_FILE.open("r", encoding="utf-8") as f:
        for line_number, line in enumerate(f, start=1):
            line = line.strip()
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

            if not name:
                raise ValueError(f"Ligne {line_number} : nom vide")

            if not 0 <= code <= 254:
                raise ValueError(
                    f"Ligne {line_number} : code {code} hors plage (0..254)"
                )

            codes.append({
                "name": name,
                "code": code,
                "text": text,
                "line": line_number,
            })

    return codes


# ============================================================
# Lecture du fichier etats.csv
# Format : TYPE;ETAT;VALEUR;NOM
# TYPE = PROD|CALI
# Remarque : un '#' en début de ligne est autorisé
# ============================================================
def read_states_csv():
    prod = []
    cali = []

    with ETATS_CSV_FILE.open("r", encoding="utf-8") as f:
        for line_number, raw_line in enumerate(f, start=1):
            line = raw_line.strip()
            if not line:
                continue

            # Accepte les lignes commentées de configuration:
            # # PROD ; INIT ; 0 ; INIT
            if line.startswith("#"):
                line = line[1:].strip()

            if not line:
                continue

            parts = [part.strip() for part in line.split(";")]
            if len(parts) != 4:
                raise ValueError(
                    f"Ligne {line_number} dans {ETATS_CSV_FILE.name}: "
                    f"4 colonnes attendues TYPE;ETAT;VALEUR;NOM"
                )

            raw_type, etat_name, raw_value, display_name = parts
            type_upper = raw_type.upper()

            # Tolère une éventuelle ligne d'en-tête :
            # TYPE ; ETAT ; VALEUR ; NOM
            if type_upper == "TYPE":
                continue

            if type_upper not in ("PROD", "CALI"):
                raise ValueError(
                    f"Ligne {line_number}: TYPE invalide {raw_type!r} "
                    f"(attendu: PROD ou CALI)"
                )

            if not etat_name:
                raise ValueError(f"Ligne {line_number}: ETAT vide")

            if not display_name:
                raise ValueError(f"Ligne {line_number}: NOM vide")

            try:
                value = int(raw_value, 0)
            except ValueError as exc:
                raise ValueError(
                    f"Ligne {line_number}: VALEUR invalide {raw_value!r}"
                ) from exc

            row = {
                "type": type_upper,
                "etat": etat_name,
                "value": value,
                "name": display_name,
                "line": line_number,
            }

            if type_upper == "PROD":
                prod.append(row)
            else:
                cali.append(row)

    if not prod:
        raise ValueError(f"Aucun état PROD trouvé dans {ETATS_CSV_FILE}")
    if not cali:
        raise ValueError(f"Aucun état CALI trouvé dans {ETATS_CSV_FILE}")

    return prod, cali

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
            raise ValueError(
                f"Nom dupliqué : {name!r} (lignes {names[name]} et {line})"
            )
        names[name] = line

        # Code unique
        if code in codes:
            raise ValueError(
                f"Code dupliqué : {code} (lignes {codes[code]} et {line})"
            )
        codes[code] = line


def check_states(states, label):
    etat_names = {}
    values = {}
    for state in states:
        etat = state["etat"]
        value = state["value"]
        line = state["line"]

        if etat in etat_names:
            raise ValueError(
                f"Etat {label} dupliqué: {etat!r} "
                f"(lignes {etat_names[etat]} et {line})"
            )
        etat_names[etat] = line

        if value in values:
            raise ValueError(
                f"Valeur {label} dupliquée: {value} "
                f"(lignes {values[value]} et {line})"
            )
        values[value] = line


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

    sorted_messages = sorted(messages, key=lambda msg: msg["code"])
    for msg in sorted_messages:
        code = msg["code"]
        lines.append(f'    {msg["name"]} = {code},    // "{msg["text"]}"')

    lines.extend([
        "};",
        "",
        "inline const __FlashStringHelper* msgName(MSG msg)",
        "{",
        "    switch (msg) {",
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
    ])

    CPP_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    CPP_OUTPUT.write_text("\n".join(lines), encoding="utf-8")


# ============================================================
# Génération du fichier Etats.h
# ============================================================
def generate_etats_h(prod_states, calibration_states):
    lines = [
        "#ifndef ETATS_H",
        "#define ETATS_H",
        "",
        "#include <stdint.h>",
        "#include \"Constantes.h\"",
        "",
        "/*",
        " * Fichier généré automatiquement.",
        " * Ne pas modifier manuellement.",
        " *",
        " * Source : etats.csv",
        " */",
        "",
        "enum class ETAT : uint8_t {",
    ]

    for state in sorted(prod_states, key=lambda state: state["value"]):
        lines.append(f'    {state["etat"]} = {state["value"]},    // "{state["name"]}"')

    lines.extend([
        "};",
        "",
        "enum class ETAT_CALIBRATION : uint8_t {",
    ])

    for state in sorted(calibration_states, key=lambda state: state["value"]):
        lines.append(f'    {state["etat"]} = {state["value"]},    // "{state["name"]}"')

    lines.extend([
        "};",
        "",
        "inline const __FlashStringHelper* etatName(ETAT etat)",
        "{",
        "    switch (etat) {",
    ])

    for state in sorted(prod_states, key=lambda state: state["value"]):
        lines.append(f'        case ETAT::{state["etat"]}: return F("{state["name"]}");')

    lines.extend([
        "",
        "        default:",
        '            return F("UNKNOWN");',
        "    }",
        "}",
        "",
        "inline const __FlashStringHelper* etatName(ETAT_CALIBRATION etat)",
        "{",
        "    switch (etat) {",
    ])

    for state in sorted(calibration_states, key=lambda state: state["value"]):
        lines.append(
            f'        case ETAT_CALIBRATION::{state["etat"]}: return F("{state["name"]}");'
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
    ])

    ETATS_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    ETATS_OUTPUT.write_text("\n".join(lines), encoding="utf-8")


# ============================================================
# Génération du fichier Python
# ============================================================
def generate_python(messages, prod_states, calibration_states):
    lines = [
        '"""',
        "Fichier généré automatiquement.",
        "Ne pas modifier manuellement.",
        "",
        "Source : protocol.csv + etats.csv + Constantes.h",
        '"""',
        "",
        "MSG = {",
    ]

    sorted_messages = sorted(messages, key=lambda msg: msg["code"])

    for msg in sorted_messages:
        lines.append(f'    {msg["code"]}: {msg["text"]!r},    # {msg["name"]!r}')

    lines.extend([
        "}",
        "",
        "ETAT_PROD = {",
    ])

    for state in sorted(prod_states, key=lambda state: state["value"]):
        lines.append(f'    {state["value"]}: "{state["name"]}",')

    lines.extend([
        "}",
        "",
        "ETAT_CALIBRATION = {",
    ])

    for state in sorted(calibration_states, key=lambda state: state["value"]):
        lines.append(f'    {state["value"]}: "{state["name"]}",')

    lines.extend([
        "}",
        "",
    ])

    PY_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    PY_OUTPUT.write_text("\n".join(lines), encoding="utf-8")


# ============================================================
# Programme principal
# ============================================================
def generer_fichiers():
    log(f"Génération des codes du protocole de communication depuis : {CSV_FILE}")
    try:
        messages = read_codes()
        if not messages:
            raise ValueError("Aucun code trouvé dans le fichier CSV")

        prod_states, calibration_states = read_states_csv()

        check_codes(messages)
        check_states(prod_states, "PROD")
        check_states(calibration_states, "CALI")

        generate_cpp(messages)
        generate_etats_h(prod_states, calibration_states)
        generate_python(messages, prod_states, calibration_states)

    except ValueError as error:
        log(f"\nERREUR : {error}", file=sys.stderr)
        sys.exit(1)

    log(f"{len(messages)} messages générés.")
    log(f"  C++ messages : {CPP_OUTPUT}")
    log(f"  C++ états    : {ETATS_OUTPUT}")
    log(f"  Python       : {PY_OUTPUT}")


def main():
    if doit_regenerer():
        generer_fichiers()
    else:
        log("Fichiers déjà à jour : aucune modification.")


if "env" in globals() or __name__ == "__main__":
    main()