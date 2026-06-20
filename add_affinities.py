import json
import glob

# For elements: "fire", "water", "earth", "wind", "light", "none"
def get_affinity_weakness(mon_id, name):
    name = name.lower()
    mon_id = mon_id.lower()
    
    if "fire" in mon_id or "fire" in name or "dragon" in mon_id:
        return "fire", "water"
    elif "water" in mon_id or "water" in name:
        return "water", "earth"
    elif "wind" in mon_id or "wind" in name or "bat" in mon_id:
        return "wind", "fire"
    elif "earth" in mon_id or "earth" in name or "stone" in mon_id or "golem" in mon_id:
        return "earth", "wind"
    elif "vampire" in mon_id or "shadow" in mon_id or "skeleton" in mon_id:
        return "none", "light"
    elif "slime" in mon_id:
        # Standard slime
        return "none", "fire"
    elif "silas" in name:
        return "light", "none"
    return "none", "none"

# Update Monsters
for fpath in glob.glob("data/monsters/*.json"):
    with open(fpath, "r") as f:
        data = json.load(f)
        
    aff, weak = get_affinity_weakness(data.get("id", ""), data.get("name", ""))
    data["affinity"] = aff
    data["weakness"] = weak
    
    with open(fpath, "w") as f:
        json.dump(data, f, indent=2)

# Update NPCs
for fpath in glob.glob("data/npcs/*.json"):
    with open(fpath, "r") as f:
        data = json.load(f)
        
    aff, weak = get_affinity_weakness(data.get("id", ""), data.get("name", ""))
    data["affinity"] = aff
    data["weakness"] = weak
    
    with open(fpath, "w") as f:
        json.dump(data, f, indent=2)

print("Affinities and Weaknesses added!")
