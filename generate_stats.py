import json
import glob
import random
import os

# Update NPCs
for fpath in glob.glob("data/npcs/*.json"):
    with open(fpath, "r") as f:
        data = json.load(f)
        
    name = data.get("name", "").lower()
    full_name = data.get("full_name", "").lower()
    role = data.get("role", "").lower()
    npc_type = data.get("type", "unnamed").lower()
    
    if npc_type == "named":
        # Named character: stats 20 - 50
        data["level"] = random.randint(5, 15)
        if name == "arthur":
            data["str"] = random.randint(45, 50) # Arthur has more str
            data["cons"] = random.randint(30, 40)
            data["agi"] = random.randint(25, 35)
            data["intl"] = random.randint(20, 30)
            data["wis"] = random.randint(20, 30)
        elif "silas" in name:
            data["str"] = random.randint(20, 30)
            data["cons"] = random.randint(20, 30)
            data["agi"] = random.randint(25, 40)
            data["intl"] = random.randint(40, 50)
            data["wis"] = random.randint(40, 50)
        else:
            data["str"] = random.randint(20, 40)
            data["cons"] = random.randint(20, 40)
            data["agi"] = random.randint(20, 40)
            data["intl"] = random.randint(20, 40)
            data["wis"] = random.randint(20, 40)
            
        data["max_hp"] = data["cons"] * 10
        data["max_mp"] = data["intl"] * 10
    else:
        # Normal person: 10 - 15
        data["level"] = random.randint(1, 3)
        data["str"] = random.randint(10, 15)
        data["cons"] = random.randint(10, 15)
        data["agi"] = random.randint(10, 15)
        data["intl"] = random.randint(10, 15)
        data["wis"] = random.randint(10, 15)
        data["max_hp"] = 100 # Normal NPC should have 100
        data["max_mp"] = 50
        
    with open(fpath, "w") as f:
        json.dump(data, f, indent=2)

# Monsters
monsters = [
    {
        "id": "mon_slime",
        "name": "Slime Lendir",
        "description": "Makhluk kenyal yang sering mengganggu petualang pemula.",
        "level": 1,
        "max_hp": 50,
        "max_mp": 10,
        "damage": 5,
        "agility": 8,
        "str": 10,
        "cons": 10,
        "agi": 8,
        "intl": 5,
        "wis": 5,
        "loot": [ {"item_id": "item_slime_gel", "chance": 80} ]
    },
    {
        "id": "mon_goblin",
        "name": "Goblin",
        "description": "Monster hijau kecil yang licik dan suka mencuri.",
        "level": 2,
        "max_hp": 80,
        "max_mp": 20,
        "damage": 8,
        "agility": 12,
        "str": 12,
        "cons": 10,
        "agi": 12,
        "intl": 8,
        "wis": 8,
        "loot": [ {"item_id": "item_goblin_ear", "chance": 50} ]
    },
    {
        "id": "mon_wolf",
        "name": "Dire Wolf",
        "description": "Serigala besar yang berburu dalam kawanan.",
        "level": 3,
        "max_hp": 100,
        "max_mp": 0,
        "damage": 12,
        "agility": 15,
        "str": 14,
        "cons": 12,
        "agi": 15,
        "intl": 6,
        "wis": 10,
        "loot": [ {"item_id": "item_wolf_pelt", "chance": 60} ]
    },
    {
        "id": "mon_orc_boss",
        "name": "Grommash the Orc Chief",
        "description": "Pemimpin klan Orc yang ganas dengan kekuatan besar.",
        "level": 10,
        "max_hp": 400,
        "max_mp": 50,
        "damage": 25,
        "agility": 18,
        "str": 30,
        "cons": 28,
        "agi": 18,
        "intl": 15,
        "wis": 15,
        "loot": [ {"item_id": "item_orc_axe", "chance": 100} ]
    },
    {
        "id": "mon_shadow_knight",
        "name": "Lord Valerius' Phantom",
        "description": "Ksatria bayangan yang dibangkitkan oleh ilmu hitam.",
        "level": 12,
        "max_hp": 500,
        "max_mp": 200,
        "damage": 30,
        "agility": 25,
        "str": 35,
        "cons": 30,
        "agi": 25,
        "intl": 20,
        "wis": 20,
        "loot": [ {"item_id": "item_shadow_blade", "chance": 100} ]
    }
]

for mon in monsters:
    with open(f"data/monsters/{mon['id'].replace('mon_', '')}.json", "w") as f:
        json.dump(mon, f, indent=2)

print("Finished generating stats!")
