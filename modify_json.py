import json
import glob
import os

def update_json(file_path, type_, name_):
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    data['weapon_type'] = type_
    data['weapon_name'] = name_
    with open(file_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=4, ensure_ascii=False)

# Monsters
monsters = {
    'ancient_dragon.json': ('unarmed', 'cakar naga raksasa'),
    'bandit_scum.json': ('dagger', 'belati tumpul'),
    'earth_slime.json': ('unarmed', 'tubuh keras'),
    'fire_elemental.json': ('staff', 'energi api'),
    'fire_slime.json': ('unarmed', 'tubuh panas'),
    'forest_troll.json': ('sword', 'gada kayu besar'), # treat club as sword for scaling, or unarmed
    'giant_bat.json': ('unarmed', 'taring tajam'),
    'goblin.json': ('dagger', 'belati berkarat'),
    'orc_boss.json': ('sword', 'kapak perang raksasa'),
    'shadow_knight.json': ('sword', 'pedang kegelapan'),
    'skeleton_warrior.json': ('sword', 'pedang berkarat'),
    'slime.json': ('unarmed', 'lendir asam'),
    'stone_golem.json': ('unarmed', 'tinju batu'),
    'vampire_lord.json': ('dagger', 'cakar vampir'),
    'water_slime.json': ('unarmed', 'lendir air'),
    'wind_slime.json': ('unarmed', 'angin puyuh'),
    'wolf.json': ('unarmed', 'gigi taring')
}

for f, (wt, wn) in monsters.items():
    update_json(os.path.join('data', 'monsters', f), wt, wn)

# NPCs
npcs = {
    'acolyte_1.json': ('staff', 'tongkat kayu'),
    'acolyte_2.json': ('staff', 'tongkat kayu'),
    'arthur.json': ('sword', 'pedang pelatihan'),
    'citizen_1.json': ('unarmed', 'tangan kosong'),
    'citizen_2.json': ('unarmed', 'tangan kosong'),
    'citizen_3.json': ('unarmed', 'tangan kosong'),
    'citizen_4.json': ('unarmed', 'tangan kosong'),
    'elara.json': ('staff', 'tongkat penyihir'),
    'garrick.json': ('sword', 'pedang prajurit'),
    'guard_1.json': ('sword', 'tombak penjaga'),
    'guard_2.json': ('sword', 'tombak penjaga'),
    'guard_3.json': ('sword', 'tombak penjaga'),
    'kael.json': ('bow', 'busur pemburu'),
    'merchant_1.json': ('unarmed', 'tangan kosong'),
    'merchant_2.json': ('unarmed', 'tangan kosong'),
    'silas.json': ('staff', 'tongkat bayangan'),
    'thug_1.json': ('dagger', 'pisau lipat'),
    'thug_2.json': ('dagger', 'pisau lipat'),
    'valerius.json': ('sword', 'pedang komandan'),
    'vane.json': ('sword', 'pedang ksatria')
}

for f, (wt, wn) in npcs.items():
    update_json(os.path.join('data', 'npcs', f), wt, wn)

print("Updated JSONs!")
