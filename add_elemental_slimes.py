import json

slimes = [
    {
        "id": "mon_fire_slime",
        "name": "Fire Slime",
        "description": "Slime yang terbakar dan sangat panas untuk disentuh.",
        "level": 2,
        "max_hp": 40,
        "max_mp": 15,
        "damage": 6,
        "agility": 9,
        "str": 8,
        "cons": 8,
        "agi": 9,
        "intl": 6,
        "wis": 4,
        "loot": [ {"item_id": "item_fire_core", "chance": 10} ],
        "magics": [
            {"id": "mag_fireball", "name": "Fireball", "type": "attacking", "mana_cost": 10, "power": 15, "element": "fire"}
        ]
    },
    {
        "id": "mon_water_slime",
        "name": "Water Slime",
        "description": "Slime berair yang sulit dihancurkan.",
        "level": 2,
        "max_hp": 45,
        "max_mp": 10,
        "damage": 5,
        "agility": 7,
        "str": 7,
        "cons": 9,
        "agi": 7,
        "intl": 5,
        "wis": 5,
        "loot": [ {"item_id": "item_water_drop", "chance": 20} ],
        "magics": [
            {"id": "mag_water_gun", "name": "Water Gun", "type": "attacking", "mana_cost": 8, "power": 12, "element": "water"}
        ]
    },
    {
        "id": "mon_wind_slime",
        "name": "Wind Slime",
        "description": "Slime yang sangat lincah dan melayang di udara.",
        "level": 2,
        "max_hp": 30,
        "max_mp": 15,
        "damage": 4,
        "agility": 15,
        "str": 5,
        "cons": 6,
        "agi": 15,
        "intl": 5,
        "wis": 6,
        "loot": [ {"item_id": "item_wind_feather", "chance": 10} ],
        "magics": [
            {"id": "mag_wind_slash", "name": "Wind Slash", "type": "attacking", "mana_cost": 12, "power": 18, "element": "wind"}
        ]
    },
    {
        "id": "mon_earth_slime",
        "name": "Earth Slime",
        "description": "Slime berbatu yang memiliki kulit keras.",
        "level": 2,
        "max_hp": 60,
        "max_mp": 5,
        "damage": 7,
        "agility": 4,
        "str": 10,
        "cons": 12,
        "agi": 4,
        "intl": 2,
        "wis": 3,
        "loot": [ {"item_id": "item_stone_heart", "chance": 5} ],
        "magics": []
    }
]

for mon in slimes:
    with open(f"data/monsters/{mon['id'].replace('mon_', '')}.json", "w") as f:
        json.dump(mon, f, indent=2)

print("Elemental slimes added!")
