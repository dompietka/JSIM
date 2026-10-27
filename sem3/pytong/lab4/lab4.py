import json
import sys
from pathlib import Path

def convert_single_cell (cell):
    
    if cell['cell_type'] == 'markdown':
        commented = map(lambda x: '# ' + x.rstrip("\n"), cell['source'])
        return "\n".join(commented) + "\n"
    elif cell['cell_type'] == 'code':
        return "\n".join(map(lambda x: x.rstrip("\n"),cell['source'])) + "\n"
    else:
        return ""

input_file = Path(sys.argv[1])
output_file = input_file.with_suffix('.py')
with open(input_file, 'r', encoding='utf-8') as f:
    notebook = json.load(f)

cells = notebook.get('cells', [])
converted_cells = list(map(convert_single_cell, cells))

exercises = map(lambda cell:  1 if cell['cell_type'] == 'markdown' and '# Ćwiczenie' in cell.get('source', [''])[0] else 0, cells)

with open(output_file, "w", encoding="utf-8") as f:
    print("\n".join(converted_cells), file=f)
    print(f"#Liczba ćwiczeń: ", sum(exercises), file=f)
    