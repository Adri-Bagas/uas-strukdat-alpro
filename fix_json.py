import json

with open('data/dialogs/biara_scenes.json', 'r') as f:
    data = json.load(f)

# Add accepted scene
data.append({
    "id": "dialog_maria_q1_accepted",
    "on_enter": [],
    "nodes": [
      {
        "type": 1,
        "npc_name": "npc_maria",
        "value": "Terima kasih, pengembara! Tolong segera kembali jika kamu sudah mendapatkan gandumnya."
      }
    ]
})

for scene in data:
    if scene['id'] == 'scene_pasar_gandum':
        scene['nodes'] = [
            {
                "type": 3,
                "npc_name": "Narator",
                "value": "Kamu tiba di Pasar Kota. Terlihat seorang pedagang gandum yang sedang dipalak oleh preman pasar."
            },
            {
                "type": 1,
                "npc_name": "Preman Pasar",
                "value": "Serahkan semua gandum ini atau kau akan menyesal!"
            },
            {
                "type": 2,
                "npc_name": "Hero",
                "value": "Hei, hentikan itu! Makanan itu untuk anak-anak panti!"
            }
        ]
        scene['on_exit'] = ["start_battle bandit_scum,bandit_scum \"give_item bread 1\""]
    elif scene['id'] == 'dialog_maria_q1_start':
        for choice in scene.get('choices', []):
            if choice['text'] == "Terima Misi":
                choice['next_scene'] = "dialog_maria_q1_accepted"
    elif scene['id'] == 'scene_serangan_bandit':
        scene['on_exit'] = ["start_battle bandit_scum,bandit_scum,bandit_scum"]

with open('data/dialogs/biara_scenes.json', 'w') as f:
    json.dump(data, f, indent=2)

