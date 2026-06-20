import json

more_monsters = [
    {
        "id": "mon_giant_bat",
        "name": "Giant Bat",
        "description": "Kelelawar raksasa yang hidup di goa-goa gelap. Sangat lincah.",
        "level": 2,
        "max_hp": 60,
        "max_mp": 0,
        "damage": 6,
        "agility": 18,
        "str": 8,
        "cons": 8,
        "agi": 18,
        "intl": 3,
        "wis": 3,
        "loot": [ {"item_id": "item_bat_wing", "chance": 40} ]
    },
    {
        "id": "mon_skeleton_warrior",
        "name": "Skeleton Warrior",
        "description": "Prajurit kerangka yang dibangkitkan dari kuburan tua.",
        "level": 4,
        "max_hp": 120,
        "max_mp": 0,
        "damage": 12,
        "agility": 10,
        "str": 15,
        "cons": 15,
        "agi": 10,
        "intl": 2,
        "wis": 2,
        "loot": [ {"item_id": "item_bone", "chance": 70} ]
    },
    {
        "id": "mon_fire_elemental",
        "name": "Fire Elemental",
        "description": "Roh api yang membakar apapun yang didekatinya.",
        "level": 5,
        "max_hp": 150,
        "max_mp": 100,
        "damage": 18,
        "agility": 14,
        "str": 5,
        "cons": 10,
        "agi": 14,
        "intl": 20,
        "wis": 15,
        "loot": [ {"item_id": "item_fire_core", "chance": 30} ]
    },
    {
        "id": "mon_stone_golem",
        "name": "Stone Golem",
        "description": "Raksasa batu dengan pertahanan yang luar biasa.",
        "level": 6,
        "max_hp": 300,
        "max_mp": 0,
        "damage": 20,
        "agility": 5,
        "str": 25,
        "cons": 30,
        "agi": 5,
        "intl": 1,
        "wis": 5,
        "loot": [ {"item_id": "item_stone_heart", "chance": 50} ]
    },
    {
        "id": "mon_bandit_scum",
        "name": "Bandit",
        "description": "Penjahat jalanan yang sering merampok musafir.",
        "level": 3,
        "max_hp": 100,
        "max_mp": 10,
        "damage": 10,
        "agility": 12,
        "str": 12,
        "cons": 10,
        "agi": 12,
        "intl": 8,
        "wis": 6,
        "loot": [ {"item_id": "item_gold_pouch", "chance": 45} ]
    },
    {
        "id": "mon_forest_troll",
        "name": "Forest Troll",
        "description": "Troll besar dengan kemampuan regenerasi.",
        "level": 7,
        "max_hp": 250,
        "max_mp": 20,
        "damage": 22,
        "agility": 9,
        "str": 22,
        "cons": 25,
        "agi": 9,
        "intl": 4,
        "wis": 8,
        "loot": [ {"item_id": "item_troll_blood", "chance": 60} ]
    },
    {
        "id": "mon_vampire_lord",
        "name": "Lord Malakar",
        "description": "Penguasa klan penghisap darah. Cepat, kuat, dan mematikan.",
        "level": 15,
        "max_hp": 600,
        "max_mp": 300,
        "damage": 40,
        "agility": 35,
        "str": 30,
        "cons": 25,
        "agi": 35,
        "intl": 40,
        "wis": 30,
        "loot": [ {"item_id": "item_vampire_fang", "chance": 100} ]
    },
    {
        "id": "mon_ancient_dragon",
        "name": "Ignis the Ancient",
        "description": "Naga legendaris penyembur api. Mimpi buruk bagi semua petualang.",
        "level": 20,
        "max_hp": 1500,
        "max_mp": 500,
        "damage": 80,
        "agility": 20,
        "str": 60,
        "cons": 50,
        "agi": 20,
        "intl": 40,
        "wis": 40,
        "loot": [ {"item_id": "item_dragon_scale", "chance": 100} ]
    }
]

for mon in more_monsters:
    with open(f"data/monsters/{mon['id'].replace('mon_', '')}.json", "w") as f:
        json.dump(mon, f, indent=2)

print("Added more monsters!")
