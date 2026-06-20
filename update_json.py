import json
import glob
import os
import random

# Update Monsters
for fpath in glob.glob("data/monsters/*.json"):
    with open(fpath, "r") as f:
        data = json.load(f)
    
    if "str" not in data:
        # Default for monsters
        data["str"] = random.randint(3, 8)
        data["cons"] = random.randint(3, 8)
        data["agi"] = data.get("agility", random.randint(3, 10))
        data["intl"] = random.randint(1, 5)
        data["wis"] = random.randint(1, 5)
        data["max_mp"] = random.randint(0, 10)
        
    with open(fpath, "w") as f:
        json.dump(data, f, indent=2)

# Update NPCs
for fpath in glob.glob("data/npcs/*.json"):
    with open(fpath, "r") as f:
        data = json.load(f)
        
    if "str" not in data:
        role = data.get("role", "").lower()
        if "guard" in role or "thug" in role or "petarung" in role:
            data["str"] = random.randint(10, 15)
            data["cons"] = random.randint(10, 15)
            data["agi"] = random.randint(8, 12)
            data["intl"] = random.randint(4, 8)
            data["wis"] = random.randint(4, 8)
            data["max_hp"] = data["cons"] * 10
            data["max_mp"] = data["intl"] * 5
        elif "acolyte" in role or "mage" in role or "pendeta" in role:
            data["str"] = random.randint(3, 6)
            data["cons"] = random.randint(4, 8)
            data["agi"] = random.randint(6, 10)
            data["intl"] = random.randint(12, 18)
            data["wis"] = random.randint(12, 18)
            data["max_hp"] = data["cons"] * 10
            data["max_mp"] = data["intl"] * 5
        else:
            data["str"] = random.randint(5, 8)
            data["cons"] = random.randint(5, 8)
            data["agi"] = random.randint(5, 8)
            data["intl"] = random.randint(5, 8)
            data["wis"] = random.randint(5, 8)
            data["max_hp"] = data["cons"] * 10
            data["max_mp"] = data["intl"] * 5
            
    with open(fpath, "w") as f:
        json.dump(data, f, indent=2)

print("Finished updating JSON files!")
