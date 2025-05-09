import sqlite3, zipfile, csv, os, shutil, html
from html.parser import HTMLParser

# --- HTML Cleaner -----------------------------------------------------------
class HTMLStripper(HTMLParser):
    def __init__(self):
        super().__init__()
        self.fed = []

    def handle_data(self, d):
        self.fed.append(d)

    def get_data(self):
        return ''.join(self.fed)

def strip_html_tags(text):
    s = HTMLStripper()
    s.feed(text)
    return s.get_data()

def clean_html_field(field: str) -> str:
    field = field.replace('<br>', '\n').replace('<br/>', '\n').replace('<br />', '\n')
    field = html.unescape(field)
    field = strip_html_tags(field)
    return field.strip()

# --- Main -------------------------------------------------------------------
def convert_apkg_to_csv(apkg_path: str) -> None:
    out_csv = "cards.csv"  # Hardcoded output file

    tmp = "temp_apkg"
    if os.path.exists(tmp):
        shutil.rmtree(tmp)
    os.makedirs(tmp, exist_ok=True)

    # Extract .apkg (just a ZIP archive)
    with zipfile.ZipFile(apkg_path, "r") as zf:
        zf.extractall(tmp)

    # Locate collection DB (newer .anki21 first, then .anki2)
    db_path = None
    for name in ("collection.anki21", "collection.anki2"):
        p = os.path.join(tmp, name)
        if os.path.exists(p):
            db_path = p
            break
    if not db_path:
        raise FileNotFoundError("No collection database found in the .apkg.")

    conn = sqlite3.connect(db_path)
    try:
        cur = conn.cursor()
        cur.execute("SELECT flds FROM notes")
        notes = cur.fetchall()

        with open(out_csv, "w", newline="", encoding="utf-8") as fh:
            writer = csv.writer(fh, quoting=csv.QUOTE_ALL)

            for (flds,) in notes:
                fields = flds.split("\x1f")
                if len(fields) < 2:
                    continue
                front = clean_html_field(fields[0])
                back  = clean_html_field(fields[1])
                if front and back:
                    writer.writerow([front, back])
    finally:
        conn.close()
        shutil.rmtree(tmp)

# --- CLI --------------------------------------------------------------------
if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python ankiToCsv.py deck.apkg")
    else:
        convert_apkg_to_csv(sys.argv[1])
