import json
import os

updates = {
    "balai_kota": {"east": "alun_alun", "south": "jalanan_utama_kota"},
    "alun_alun": {"west": "balai_kota", "east": "pasar_kota", "south": "kandang_kuda"},
    "pasar_kota": {"west": "alun_alun"},
    "jalanan_utama_kota": {"north": "balai_kota", "east": "kandang_kuda", "south": "kedai_usang"},
    "kandang_kuda": {"north": "alun_alun", "west": "jalanan_utama_kota"},
    "kedai_usang": {"north": "jalanan_utama_kota", "east": "permukiman_kumuh", "south": "kamar_loteng"},
    "permukiman_kumuh": {"west": "kedai_usang", "east": "menara_tua", "south": "gua_tambang"},
    "menara_tua": {"west": "permukiman_kumuh"},
    "kamar_loteng": {"north": "kedai_usang"},
    "gua_tambang": {"north": "permukiman_kumuh"}
}

directory = "data/places"
for filename in os.listdir(directory):
    if filename.endswith(".json"):
        filepath = os.path.join(directory, filename)
        with open(filepath, "r") as f:
            data = json.load(f)
        
        place_id = data.get("id")
        if place_id in updates:
            data["walkable"] = updates[place_id]
            with open(filepath, "w") as f:
                json.dump(data, f, indent=2)
                f.write('\n')
                
print("Done patching JSONs!")
