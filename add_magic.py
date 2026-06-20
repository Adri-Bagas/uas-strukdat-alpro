import json
import glob
import random
import os

# Define some spells
basic_spells = [
    {"id": "mag_fireball", "name": "Fireball", "type": "attacking", "mana_cost": 10, "power": 15, "element": "fire"},
    {"id": "mag_heal", "name": "Heal", "type": "healing", "mana_cost": 8, "power": 20, "element": "light"},
    {"id": "mag_wind_slash", "name": "Wind Slash", "type": "attacking", "mana_cost": 12, "power": 18, "element": "wind"}
]

adv_spells = [
    {"id": "mag_inferno", "name": "Inferno", "type": "attacking", "mana_cost": 30, "power": 50, "element": "fire"},
    {"id": "mag_great_heal", "name": "Great Heal", "type": "healing", "mana_cost": 25, "power": 100, "element": "light"},
    {"id": "mag_earthquake", "name": "Earthquake", "type": "attacking", "mana_cost": 35, "power": 60, "element": "earth"}
]

# Update NPCs
for fpath in glob.glob("data/npcs/*.json"):
    with open(fpath, "r") as f:
        data = json.load(f)
        
    role = data.get("role", "").lower()
    npc_type = data.get("type", "unnamed").lower()
    name = data.get("name", "").lower()
    
    # Reset magics/specials to avoid duplicates
    data["magics"] = []
    if "special_move" in data:
        del data["special_move"]
        
    # Give magic based on role
    if "mage" in role or "acolyte" in role or "pendeta" in role or "silas" in name:
        data["magics"] = random.sample(basic_spells, 2)
        if npc_type == "named":
            data["magics"] += random.sample(adv_spells, 1)
    elif "guard" in role or "thug" in role or "arthur" in name:
        # Fighters might know 0 or 1 basic spell
        if random.random() < 0.3:
            data["magics"] = random.sample(basic_spells, 1)
            
    # Special moves ONLY for named characters
    if npc_type == "named":
        if "arthur" in name:
            data["special_move"] = {
                "id": "sp_silent_strike", "name": "Silent Strike", "max_uses_per_day": 3, "power": 80, "element": "none"
            }
        elif "silas" in name:
            data["special_move"] = {
                "id": "sp_arcane_burst", "name": "Arcane Burst", "max_uses_per_day": 2, "power": 120, "element": "light"
            }
        else:
            data["special_move"] = {
                "id": "sp_heroic_blow", "name": "Heroic Blow", "max_uses_per_day": 1, "power": 50, "element": "none"
            }

    with open(fpath, "w") as f:
        json.dump(data, f, indent=2)


# Update Monsters
bosses = ["mon_orc_boss", "mon_shadow_knight", "mon_ancient_dragon", "mon_vampire_lord"]
for fpath in glob.glob("data/monsters/*.json"):
    with open(fpath, "r") as f:
        data = json.load(f)
        
    mon_id = data.get("id", "")
    data["magics"] = []
    if "special_move" in data:
        del data["special_move"]
        
    # Basic magic for some monsters
    if "elemental" in mon_id:
        data["magics"].append(basic_spells[0]) # fireball
        data["magics"].append(adv_spells[0]) # inferno
    elif "dragon" in mon_id:
        data["magics"].append(adv_spells[0]) # inferno
    elif "troll" in mon_id or "goblin" in mon_id:
        if random.random() < 0.5:
            data["magics"].append(basic_spells[2]) # wind slash
            
    # Boss monsters get special moves
    if mon_id in bosses:
        if "dragon" in mon_id:
            data["special_move"] = {
                "id": "sp_dragon_breath", "name": "Dragon's Breath", "max_uses_per_day": 5, "power": 150, "element": "fire"
            }
        elif "vampire" in mon_id:
            data["special_move"] = {
                "id": "sp_blood_drain", "name": "Blood Drain", "max_uses_per_day": 3, "power": 100, "element": "none"
            }
        elif "orc" in mon_id:
            data["special_move"] = {
                "id": "sp_brutal_smash", "name": "Brutal Smash", "max_uses_per_day": 2, "power": 80, "element": "none"
            }
        elif "shadow" in mon_id:
            data["special_move"] = {
                "id": "sp_shadow_cleave", "name": "Shadow Cleave", "max_uses_per_day": 3, "power": 90, "element": "none"
            }
            
    with open(fpath, "w") as f:
        json.dump(data, f, indent=2)

print("Added magics and specials!")
